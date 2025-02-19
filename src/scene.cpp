#include "scene.hpp"
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/window.hpp>
#include <darmok/utils.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/string.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.hpp"

namespace darmok
{
    std::vector<Entity> ISceneDelegate::getSerializableEntities(const EntitySparseSet& storage, const OptionalRef<ISceneDelegate>& dlg)
    {
        std::vector<Entity> entities;
        entities.reserve(storage.size());
        for (auto itr = storage.rbegin(), last = storage.rend(); itr != last; ++itr)
        {
            const Entity entity = *itr;
            if (entity != entt::null && storage.contains(entity) && (!dlg || dlg->shouldEntityBeSerialized(entity)))
            {
                entities.push_back(entity);
            }
        }
        return entities;
    }

    SceneImpl::SceneImpl(Scene& scene) noexcept
        : _scene(scene)
        , _renderChain(*this)
        , _paused(false)
        , _pendingDestroyAll(false)
        , _pendingDestroyFilter({}, EntityFilterOperation::Or)
    {
    }

    entt::id_type SceneImpl::getId() const noexcept
    {
        return reinterpret_cast<uintptr_t>(static_cast<const void*>(this));
    }

    SceneImpl::~SceneImpl() noexcept
    {
        // empty on purpose
    }

    void SceneImpl::setDelegate(const OptionalRef<ISceneDelegate>& dlg) noexcept
    {
        _delegate = dlg;
    }

    const OptionalRef<ISceneDelegate>& SceneImpl::getDelegate() const noexcept
    {
        return _delegate;
    }

    std::string SceneImpl::toString() const noexcept
    {
        return "Scene(" + getDescName() + ")";
    }

    std::string SceneImpl::getDescName() const noexcept
    {
        if (!_name.empty())
        {
            return _name;
        }
        return StringUtils::binToHex(getId());
    }

    void SceneImpl::addSceneComponent(std::unique_ptr<ISceneComponent>&& component) noexcept
    {
        if (auto type = component->getSceneComponentType())
        {
            removeSceneComponent(type->hash());
        }
        if (_app)
        {
            component->init(_scene, _app.value());
        }
        _components.emplace_back(std::move(component));
    }

    bool SceneImpl::removeSceneComponent(entt::id_type type) noexcept
    {
        auto itr = findSceneComponent(type);
        if (itr == _components.end())
        {
            return false;
        }
        _components.erase(itr);
        return true;
    }

    bool SceneImpl::hasSceneComponent(entt::id_type type) const noexcept
    {
        auto itr = findSceneComponent(type);
        return itr != _components.end();
    }

    OptionalRef<ISceneComponent> SceneImpl::getSceneComponent(entt::id_type type) noexcept
    {
        auto itr = findSceneComponent(type);
        if (itr == _components.end())
        {
            return nullptr;
        }
        return **itr;
    }

    OptionalRef<const ISceneComponent> SceneImpl::getSceneComponent(entt::id_type type) const noexcept
    {
        auto itr = findSceneComponent(type);
        if (itr == _components.end())
        {
            return nullptr;
        }
        return **itr;
    }

    struct SceneComponentTypeHashFinder final
    {
        entt::id_type type;

        bool operator()(const std::shared_ptr<ISceneComponent>& comp) const noexcept
        {
            if (auto typeInfo = comp->getSceneComponentType())
            {
                return typeInfo->hash() == type;
            }
            return false;
        }
    };

    SceneImpl::Components::iterator SceneImpl::findSceneComponent(entt::id_type type) noexcept
    {
        return std::find_if(_components.begin(), _components.end(),
            SceneComponentTypeHashFinder{ type });
    }

    SceneImpl::Components::const_iterator SceneImpl::findSceneComponent(entt::id_type type) const noexcept
    {
        return std::find_if(_components.begin(), _components.end(),
            SceneComponentTypeHashFinder{ type });
    }

    SceneComponentRefs SceneImpl::getSceneComponents() noexcept
    {
        SceneComponentRefs comps;
        comps.reserve(_components.size());
        for (auto& comp : _components)
        {
            comps.emplace_back(*comp);
        }
        return comps;
    }

    ConstSceneComponentRefs SceneImpl::getSceneComponents() const noexcept
    {
        ConstSceneComponentRefs comps;
        comps.reserve(_components.size());
        for (const auto& comp : _components)
        {
            comps.emplace_back(*comp);
        }
        return comps;
    }

    EntityRegistry& SceneImpl::getRegistry() noexcept
    {
        return _registry;
    }

    const EntityRegistry& SceneImpl::getRegistry() const noexcept
    {
        return _registry;
    }

    RenderChain& SceneImpl::getRenderChain() noexcept
    {
        return _renderChain;
    }

    const RenderChain& SceneImpl::getRenderChain() const noexcept
    {
        return _renderChain;
    }

    const std::optional<Viewport>& SceneImpl::getViewport() const noexcept
    {
        return _viewport;
    }

    void SceneImpl::setViewport(const std::optional<Viewport>& vport) noexcept
    {
        if (_viewport == vport)
        {
            return;
        }
        _viewport = vport;
        if (_app)
        {
            _app->requestRenderReset();
        }
    }

    Viewport SceneImpl::getCurrentViewport() const noexcept
    {
        if (_viewport)
        {
            return _viewport.value();
        }
        if (_app)
        {
            return { _app->getWindow().getPixelSize() };
        }
        return {};
    }

    void SceneImpl::setName(const std::string& name) noexcept
    {
        _name = name;
    }

    const std::string& SceneImpl::getName() const noexcept
    {
        return _name;
    }

    void SceneImpl::setPaused(bool paused) noexcept
    {
        _paused = paused;
    }

    bool SceneImpl::isPaused() const noexcept
    {
        return _paused;
    }

    std::vector<Entity> SceneImpl::getRootEntities() const noexcept
    {
        std::vector<Entity> roots;
        for (auto entity : _scene.getEntities<Transform>())
        {
            if (!_scene.getComponent<const Transform>(entity)->getParent())
            {
                roots.push_back(entity);
            }
        }
        return roots;
    }

    void SceneImpl::init(App& app)
    {
        if (_app == app)
        {
            return;
        }

        if (_app)
        {
            shutdown();
        }

        _app = app;
        _renderChain.init();

        for (auto& comp : Components(_components))
        {
            comp->init(_scene, app);
        }

        auto& cams = _registry.storage<Camera>();
        for (auto itr = cams.rbegin(), last = cams.rend(); itr != last; ++itr)
        {
            itr->getImpl().init(_scene, app);
        }

        _registry.on_construct<Camera>().connect<&SceneImpl::onCameraConstructed>(*this);
        _registry.on_destroy<Camera>().connect<&SceneImpl::onCameraDestroyed>(*this);
    }

    Viewport SceneImpl::getRenderChainViewport() const noexcept
    {
        return getCurrentViewport();
    }

    void SceneImpl::onRenderChainChanged() noexcept
    {
        if (_app)
        {
            _app->requestRenderReset();
        }
    }

    void SceneImpl::onCameraConstructed(EntityRegistry& registry, Entity entity)
    {
        if (_app)
        {
            auto& cam = registry.get<Camera>(entity);
            cam.getImpl().init(_scene, _app.value());
        }
    }

    void SceneImpl::onCameraDestroyed(EntityRegistry& registry, Entity entity)
    {
        if (_app)
        {
            auto& cam = registry.get<Camera>(entity);
            cam.getImpl().shutdown();
        }
    }

    void SceneImpl::render()
    {
        auto& cams = _registry.storage<Camera>();
        for (auto itr = cams.rbegin(), last = cams.rend(); itr != last; ++itr)
        {
            if (!shouldCameraRender(*itr))
            {
                continue;
            }
            itr->getImpl().render();
        }
        _renderChain.render();
    }

    void SceneImpl::shutdown()
    {
        _registry.clear();

        {
            auto components = Components(_components);
            for (auto itr = components.rbegin(); itr != components.rend(); ++itr)
            {
                (*itr)->shutdown();
            }
        }
        _renderChain.shutdown();

        _registry.on_construct<Camera>().disconnect<&SceneImpl::onCameraConstructed>(*this);
        _registry.on_destroy<Camera>().disconnect<&SceneImpl::onCameraDestroyed>(*this);

        _app.reset();
    }

    bool SceneImpl::shouldCameraRender(const Camera& cam) const
    {
        if (_delegate)
        {
            return _delegate->shouldCameraRender(cam);
        }
        return true;
    }

    bool SceneImpl::shouldEntityBeSerialized(Entity entity) const
    {
        if (_delegate)
        {
            return _delegate->shouldEntityBeSerialized(entity);
        }
        return true;
    }

    bgfx::ViewId SceneImpl::renderReset(bgfx::ViewId viewId)
    {
        _renderChain.beforeRenderReset();

        {
            auto components = Components(_components);
            for (auto itr = components.rbegin(); itr != components.rend(); ++itr)
            {
                viewId = (*itr)->renderReset(viewId);
            }
        }

        // iteration in reverse to maintain the order in wich the cameras where added
        auto& cams = _registry.storage<Camera>();
        for (auto itr = cams.rbegin(), last = cams.rend(); itr != last; ++itr)
        {
            if (!shouldCameraRender(*itr))
            {
                continue;
            }
            viewId = itr->getImpl().renderReset(viewId);
        }

        viewId = _renderChain.renderReset(viewId);
        return viewId;
    }

    void SceneImpl::destroyEntityImmediate(Entity entity, bool destroyChildren) noexcept
    {
        std::vector<Entity> entities;
        if (destroyChildren)
        {
            _scene.forEachChild(entity, [&entities](auto entity, auto& /* trans */)
            {
                entities.push_back(entity);
                return false;
            });
        }
        else
        {
            entities.push_back(entity);
        }
        for (auto& entity : entities)
        {
            _registry.destroy(entity);
        }
    }

    void SceneImpl::destroyEntitiesImmediate() noexcept
    {
        _registry.clear();
        _registry.storage<Entity>().clear();
    }

    void SceneImpl::destroyEntitiesImmediate(const EntityFilter& filter) noexcept
    {
        for (auto entity : _scene.getEntities(filter))
        {
            _registry.destroy(entity);
        }
    }

    void SceneImpl::destroyPendingEntities() noexcept
    {
        if (_pendingDestroyAll)
        {
            _pendingDestroyAll = false;
            destroyEntitiesImmediate();
            return;
        }
        if (!_pendingDestroyFilter.empty())
        {
            auto& filter = _pendingDestroyFilter;
            _pendingDestroyFilter.elements.clear();
            destroyEntitiesImmediate(filter);
        }
        if(!_pendingDestroy.empty())
        {
            auto entities(_pendingDestroy);
            _pendingDestroy.clear();
            for (auto entity : entities)
            {
                destroyEntityImmediate(entity);
            }
        }
    }

    void SceneImpl::update(float deltaTime)
    {
        destroyPendingEntities();

        if (!_paused)
        {
            for (auto entity : _scene.getUpdateEntities<Transform>())
            {
                _scene.getComponent<Transform>(entity)->update();
            }

            for (auto entity : _scene.getUpdateEntities<Camera>())
            {
                _scene.getComponent<Camera>(entity)->getImpl().update(deltaTime);
            }

            for (auto& comp : Components(_components))
            {
                comp->update(deltaTime);
            }

            _renderChain.update(deltaTime);
        }
    }

    void SceneImpl::destroyEntities() noexcept
    {
        _pendingDestroyAll = true;
    }

    void SceneImpl::destroyEntities(const EntityFilter& filter)
    {
        _pendingDestroyFilter |= filter;
    }

    void SceneImpl::destroyEntity(Entity entity, bool destroyChildren) noexcept
    {
        _pendingDestroy.insert(entity);
        if (destroyChildren)
        {
            _scene.forEachChild(entity, [this](auto entity, auto& /* trans */)
            {
                _pendingDestroy.insert(entity);
                return false;
            });
        }
    }

    bool SceneImpl::removeComponent(Entity entity, entt::id_type typeId) noexcept
    {
        auto& reg = getRegistry();
        return reg.storage(typeId)->remove(entity);
    }

    void SceneImpl::setUpdateFilter(const EntityFilter& filter) noexcept
    {
        _updateFilter = filter;
    }

    const EntityFilter& SceneImpl::getUpdateFilter() const noexcept
    {
        return _updateFilter;
    }

    Scene::Scene() noexcept
    : _impl(std::make_unique<SceneImpl>(*this))
    {
    }

    Scene::Scene(App& app) noexcept
        : Scene()
    {
        _impl->init(app);
    }

    Scene::~Scene() noexcept
    {
    }

    SceneImpl& Scene::getImpl() noexcept
    {
        return *_impl;
    }

    const SceneImpl& Scene::getImpl() const noexcept
    {
        return *_impl;
    }

    EntityRegistry& Scene::getRegistry() noexcept
    {
        return _impl->getRegistry();
    }

    const EntityRegistry& Scene::getRegistry() const noexcept
    {
        return _impl->getRegistry();
    }

    OptionalRef<Transform> Scene::getTransformParent(const Transform& trans) noexcept
    {
        return trans.getParent();
    }

    TransformChildren Scene::getTransformChildren(const Transform& trans) noexcept
    {
        return trans.getChildren();
    }

    Entity Scene::createEntity() noexcept
    {
        return getRegistry().create();
    }

    void Scene::destroyEntities() noexcept
    {
        _impl->destroyEntities();
    }

    void Scene::destroyEntities(const EntityFilter& filter) noexcept
    {
        _impl->destroyEntities(filter);
    }

    void Scene::destroyEntity(Entity entity, bool destroyChildren) noexcept
    {
        _impl->destroyEntity(entity, destroyChildren);
    }

    void Scene::destroyEntityImmediate(Entity entity, bool destroyChildren) noexcept
    {
        _impl->destroyEntityImmediate(entity, destroyChildren);
    }

    void Scene::destroyEntitiesImmediate() noexcept
    {
        _impl->destroyEntitiesImmediate();
    }

    void Scene::destroyEntitiesImmediate(const EntityFilter& filter) noexcept
    {
        _impl->destroyEntitiesImmediate(filter);
    }

    bool Scene::isValidEntity(Entity entity) const noexcept
    {
        return getRegistry().valid(entity);
    }

    Entity Scene::getEntity(entt::id_type type, const void* ptr) const noexcept
    {
        const auto* storage = getRegistry().storage(type);
        if (!storage || storage->empty())
        {
            return entt::null;
        }

        // TODO: is there a faster way that does not require to iterate over the entities?
        for (auto itr = storage->rbegin(), last = storage->rend(); itr < last; ++itr)
        {
            auto entity = *itr;
            if (!storage->contains(entity))
            {
                continue;
            }
            const auto* comp = storage->value(entity);
            if (comp == ptr)
            {
                return entity;
            }
        }
        return entt::null;
    }

    const void* Scene::getComponent(Entity entity, entt::id_type type) const noexcept
    {
        const auto* storage = getRegistry().storage(type);
        if (!storage)
        {
            return nullptr;
        }
        return storage->value(entity);
    }

    std::unordered_map<entt::type_info, const void*> Scene::getComponents(Entity entity) const noexcept
    {
        std::unordered_map<entt::type_info, const void*> ptrs;
        for (auto [typeId, storage] : getRegistry().storage())
        {
            if (storage.contains(entity))
            {
                const auto* ptr = storage.value(entity);
                ptrs.emplace(storage.type(), ptr);
            }
        }
        return ptrs;
    }

    std::unordered_map<entt::type_info, void*> Scene::getComponents(Entity entity) noexcept
    {
        std::unordered_map<entt::type_info, void*> ptrs;
        for (auto [typeId, storage] : getRegistry().storage())
        {
            if (storage.contains(entity))
            {
                auto* ptr = storage.value(entity);
                ptrs.emplace(storage.type(), ptr);
            }
        }
        return ptrs;
    }

    EntityView Scene::getEntities() const noexcept
    {
        return getEntities({});
    }

    EntityView Scene::getEntities(entt::id_type typeId) const noexcept
    {
        return getEntities({}, typeId);
    }

    EntityView Scene::getEntities(const EntityFilter& filter, entt::id_type typeId) const noexcept
    {
        if (typeId == 0)
        {
            return { getRegistry(), filter };
        }
        return { getRegistry(), EntityFilter(typeId) & filter };
    }

    bool Scene::removeComponent(Entity entity, entt::id_type typeId) noexcept
    {
        return _impl->removeComponent(entity, typeId);
    }

    bool Scene::hasComponent(Entity entity, entt::id_type typeId) const noexcept
    {
        const auto* storage = getRegistry().storage(typeId);
        return storage && storage->find(entity) != storage->end();
    }

    std::vector<Entity> Scene::getRootEntities() const noexcept
    {
        return _impl->getRootEntities();
    }

    entt::id_type Scene::getId() const noexcept
    {
        return _impl->getId();
    }

    Scene& Scene::setName(const std::string& name) noexcept
    {
        _impl->setName(name);
        return *this;
    }

    const std::string& Scene::getName() const noexcept
    {
        return _impl->getName();
    }

    std::string Scene::toString() const noexcept
    {
        return _impl->toString();
    }

    Scene& Scene::setDelegate(const OptionalRef<ISceneDelegate>& dlg) noexcept
    {
        _impl->setDelegate(dlg);
        return *this;
    }

    const OptionalRef<ISceneDelegate>& Scene::getDelegate() const noexcept
    {
        return _impl->getDelegate();
    }

    Scene& Scene::setPaused(bool paused) noexcept
    {
        _impl->setPaused(paused);
        return *this;
    }

    bool Scene::isPaused() const noexcept
    {
        return _impl->isPaused();
    }

    RenderChain& Scene::getRenderChain() noexcept
    {
        return _impl->getRenderChain();
    }

    const RenderChain& Scene::getRenderChain() const noexcept
    {
        return _impl->getRenderChain();
    }

    const std::optional<Viewport>& Scene::getViewport() const noexcept
    {
        return _impl->getViewport();
    }

    Scene& Scene::setViewport(const std::optional<Viewport>& viewport) noexcept
    {
        _impl->setViewport(viewport);
        return *this;
    }

    Viewport Scene::getCurrentViewport() const noexcept
    {
        return _impl->getCurrentViewport();
    }

    void Scene::addSceneComponent(std::unique_ptr<ISceneComponent>&& component) noexcept
    {
        _impl->addSceneComponent(std::move(component));
    }

    bool Scene::removeSceneComponent(entt::id_type type) noexcept
    {
        return _impl->removeSceneComponent(type);
    }

    bool Scene::hasSceneComponent(entt::id_type type) const noexcept
    {
        return _impl->hasSceneComponent(type);
    }

    OptionalRef<ISceneComponent> Scene::getSceneComponent(entt::id_type type) noexcept
    {
        return _impl->getSceneComponent(type);
    }

    OptionalRef<const ISceneComponent> Scene::getSceneComponent(entt::id_type type) const noexcept
    {
        return _impl->getSceneComponent(type);
    }

    SceneComponentRefs Scene::getSceneComponents() noexcept
    {
        return _impl->getSceneComponents();
    }

    ConstSceneComponentRefs Scene::getSceneComponents() const noexcept
    {
        const auto& impl = *_impl;
        return impl.getSceneComponents();
    }

    Scene& Scene::setUpdateFilter(const EntityFilter& filter) noexcept
    {
        _impl->setUpdateFilter(filter);
        return *this;
    }

    const EntityFilter& Scene::getUpdateFilter() const noexcept
    {
        return _impl->getUpdateFilter();
    }

    SceneAppComponent::SceneAppComponent(const std::shared_ptr<Scene>& scene) noexcept
    {
        _scenes.push_back(scene ? scene : std::make_shared<Scene>());
    }

    std::shared_ptr<Scene> SceneAppComponent::getScene(size_t i) const noexcept
    {
        if (i < 0 || i >= _scenes.size())
        {
            return nullptr;
        }
        return _scenes[i];
    }

    SceneAppComponent& SceneAppComponent::setScene(const std::shared_ptr<Scene>& scene, size_t i) noexcept
    {
        if (i < 0 || !scene)
        {
            return *this;
        }
        if (i >= _scenes.size())
        {
            _scenes.resize(i + 1);
        }
        auto& oldScene = _scenes[i];
        if (_app && oldScene)
        {
            oldScene->getImpl().shutdown();
        }
        _scenes[i] = scene;
        if (_app)
        {
            scene->getImpl().init(*_app);
        }
        return *this;
    }

    std::shared_ptr<Scene> SceneAppComponent::addScene() noexcept
    {
        auto scene = std::make_shared<Scene>();
        addScene(scene);
        return scene;
    }

    SceneAppComponent& SceneAppComponent::addScene(const std::shared_ptr<Scene>& scene) noexcept
    {
        if (scene)
        {
            _scenes.push_back(scene);
            if (_app)
            {
                scene->getImpl().init(*_app);
            }
        }

        return *this;
    }

    const SceneAppComponent::Scenes& SceneAppComponent::getScenes() const noexcept
    {
        return _scenes;
    }

    void SceneAppComponent::init(App& app)
    {
        if (_app)
        {
            shutdown();
        }
        _app = app;
        for(auto& scene : _scenes)
        {
            scene->getImpl().init(app);
        }
    }

    bgfx::ViewId SceneAppComponent::renderReset(bgfx::ViewId viewId)
    {
        for (auto& scene : _scenes)
        {
            viewId = scene->getImpl().renderReset(viewId);
        }
        return viewId;
    }

    void SceneAppComponent::render()
    {
        for (auto& scene : _scenes)
        {
            scene->getImpl().render();
        }
    }

    void SceneAppComponent::shutdown()
    {
        _app = nullptr;
        for (auto itr = _scenes.rbegin(); itr != _scenes.rend(); ++itr)
        {
            (*itr)->getImpl().shutdown();
        }
    }

    void SceneAppComponent::update(float deltaTime)
    {
        for (auto& scene : _scenes)
        {
            scene->getImpl().update(deltaTime);
        }
    }
}