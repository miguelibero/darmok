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
            removeSceneComponent(type);
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

    SceneImpl::Components::iterator SceneImpl::findSceneComponent(entt::id_type type) noexcept
    {
        return std::find_if(_components.begin(), _components.end(),
            [type](auto& comp) { return comp->getSceneComponentType() == type; });
    }

    SceneImpl::Components::const_iterator SceneImpl::findSceneComponent(entt::id_type type) const noexcept
    {
        return std::find_if(_components.begin(), _components.end(),
            [type](auto& comp) { return comp->getSceneComponentType() == type; });
    }

    EntityRegistry& SceneImpl::getRegistry()
    {
        return _registry;
    }

    const EntityRegistry& SceneImpl::getRegistry() const
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

    void SceneImpl::setViewport(const std::optional<Viewport>& vp) noexcept
    {
        if (_viewport == vp)
        {
            return;
        }
        _viewport = vp;
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
        else if (_app)
        {
            return Viewport(_app->getWindow().getPixelSize());
        }
        return Viewport();
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

        for (auto comp : Components(_components))
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
            viewId = itr->getImpl().renderReset(viewId);
        }

        viewId = _renderChain.renderReset(viewId);
        return viewId;
    }

    void SceneImpl::destroyEntityImmediate(Entity entity) noexcept
    {
        _registry.destroy(entity);
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
            auto filter = _pendingDestroyFilter;
            _pendingDestroyFilter.elements.clear();
            destroyEntitiesImmediate(filter);
        }
        if(!_pendingDestroy.empty())
        {
            std::vector<Entity> entities(_pendingDestroy);
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

            for (auto comp : Components(_components))
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

    void SceneImpl::destroyEntity(Entity entity) noexcept
    {
        auto itr = std::find(_pendingDestroy.begin(), _pendingDestroy.end(), entity);
        if (itr != _pendingDestroy.end())
        {
            return;
        }
        _pendingDestroy.push_back(entity);
    }

    SceneImpl::ComponentDependencies SceneImpl::_compDeps;

    void SceneImpl::registerComponentDependency(entt::id_type typeId1, entt::id_type typeId2)
    {
        if (typeId1 == typeId2)
        {
            throw std::invalid_argument("dependency loop");
        }
        _compDeps[typeId1].insert(typeId2);
        _compDeps[typeId2].insert(typeId1);
    }

    bool SceneImpl::removeComponent(Entity entity, entt::id_type typeId) noexcept
    {
        auto& reg = getRegistry();
        auto found = reg.storage(typeId)->remove(entity);
        auto itr = _compDeps.find(typeId);
        if (itr != _compDeps.end())
        {
            for (auto& depTypeId : itr->second)
            {
                if (removeComponent(entity, depTypeId))
                {
                    found = true;
                }
            }
        }
        return found;
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

    EntityRegistry& Scene::getRegistry()
    {
        return _impl->getRegistry();
    }

    const EntityRegistry& Scene::getRegistry() const
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
        return _impl->destroyEntities(filter);
    }

    void Scene::destroyEntity(Entity entity) noexcept
    {
        return _impl->destroyEntity(entity);
    }

    void Scene::destroyEntityImmediate(Entity entity) noexcept
    {
        return _impl->destroyEntity(entity);
    }

    void Scene::destroyEntitiesImmediate() noexcept
    {
        return _impl->destroyEntitiesImmediate();
    }

    void Scene::destroyEntitiesImmediate(const EntityFilter& filter) noexcept
    {
        return _impl->destroyEntitiesImmediate(filter);
    }

    bool Scene::isValidEntity(Entity entity) const noexcept
    {
        return getRegistry().valid(entity);
    }

    Entity Scene::getEntity(entt::id_type type, const void* ptr) noexcept
    {
        auto storage = getRegistry().storage(type);
        if (!storage)
        {
            return entt::null;
        }

        // TODO: check why this does not work
        for (auto itr = storage->rbegin(), last = storage->rend(); itr < last; ++itr)
        {
            auto comp = storage->value(*itr);
            if (comp == ptr)
            {
                return *itr;
            }
        }
        return entt::null;
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
            return EntityView(getRegistry(), filter);
        }
        return EntityView(getRegistry(), EntityFilter(typeId) & filter);
    }

    void Scene::registerComponentDependency(entt::id_type typeId1, entt::id_type typeId2)
    {
        SceneImpl::registerComponentDependency(typeId1, typeId2);
    }

    bool Scene::removeComponent(Entity entity, entt::id_type typeId) noexcept
    {
        return _impl->removeComponent(entity, typeId);
    }

    bool Scene::hasComponent(Entity entity, entt::id_type typeId) const noexcept
    {
        auto storage = getRegistry().storage(typeId);
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

    Scene& Scene::setViewport(const std::optional<Viewport>& vp) noexcept
    {
        _impl->setViewport(vp);
        return *this;
    }

    Viewport Scene::getCurrentViewport() const noexcept
    {
        return _impl->getCurrentViewport();
    }

    void Scene::addSceneComponent(std::unique_ptr<ISceneComponent>&& component) noexcept
    {
        return _impl->addSceneComponent(std::move(component));
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