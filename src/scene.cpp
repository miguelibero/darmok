#include "scene.hpp"
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/window.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace darmok
{
    SceneImpl::SceneImpl(Scene& scene) noexcept
        : _scene(scene)
        , _renderChain(*this)
        , _paused(false)
    {
    }

    SceneImpl::~SceneImpl() noexcept
    {
        // empty on purpose
    }

    std::string SceneImpl::toString() const noexcept
    {
        return "Scene(" + _name + ")";
    }

    void SceneImpl::addSceneComponent(std::unique_ptr<ISceneComponent>&& component) noexcept
    {
        removeSceneComponent(component->getSceneComponentType());
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

    RenderGraphDefinition& SceneImpl::getRenderGraph() noexcept
    {
        return _renderGraph;
    }

    const RenderGraphDefinition& SceneImpl::getRenderGraph() const noexcept
    {
        return _renderGraph;
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
        if (_viewport != vp)
        {
            _viewport = vp;
            renderReset();
        }
    }

    Viewport SceneImpl::getCurrentViewport() const noexcept
    {
        if(_viewport)
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

    SceneImpl::ComponentRefs SceneImpl::copyComponentContainer() const noexcept
    {
        ComponentRefs refs;
        refs.reserve(_components.size());
        for (auto& component : _components)
        {
            refs.emplace_back(*component);
        }
        return refs;
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

        _renderGraph.clear();
        std::string name = _name;
        if (name.empty())
        {
            name = "Scene";
        }
        _renderGraph.setName(name);
        _renderChain.init(name + " render chain", -RenderPassDefinition::kMaxPriority);

        for (auto& comp : copyComponentContainer())
        {
            comp.get().init(_scene, app);
        }

        auto& cams = _registry.storage<Camera>();
        for (auto itr = cams.rbegin(), last = cams.rend(); itr != last; ++itr)
        {
            itr->init(_scene, app);
        }

        _registry.on_construct<Camera>().connect<&SceneImpl::onCameraConstructed>(*this);
        _registry.on_destroy<Camera>().connect<&SceneImpl::onCameraDestroyed>(*this);
    }

    void SceneImpl::updateRenderGraph() noexcept
    {
        if (_app)
        {
            _app->getRenderGraph().setChild(_renderGraph);
        }
    }

    Viewport SceneImpl::getRenderChainViewport() const noexcept
    {
        return getCurrentViewport();
    }

    RenderGraphDefinition& SceneImpl::getRenderChainParentGraph() noexcept
    {
        return _renderGraph;
    }

    void SceneImpl::onRenderChainInputChanged() noexcept
    {
        renderReset();
    }

    void SceneImpl::onCameraConstructed(EntityRegistry& registry, Entity entity)
    {
        if (_app)
        {
            auto& cam = registry.get<Camera>(entity);
            cam.init(_scene, _app.value());
        }
    }

    void SceneImpl::onCameraDestroyed(EntityRegistry& registry, Entity entity)
    {
        if (_app)
        {
            auto& cam = registry.get<Camera>(entity);
            cam.shutdown();
        }
    }

    void SceneImpl::shutdown()
    {
        _registry.clear();

        auto components = copyComponentContainer();
        for (auto itr = components.rbegin(); itr != components.rend(); ++itr)
        {
            itr->get().shutdown();
        }
        _renderChain.shutdown();

        _registry.on_construct<Camera>().disconnect<&SceneImpl::onCameraConstructed>(*this);
        _registry.on_destroy<Camera>().disconnect<&SceneImpl::onCameraDestroyed>(*this);

        _app.reset();
    }

    void SceneImpl::renderReset()
    {
        _renderGraph.clear();

        // iteration in reverse to maintain the order in wich the cameras where added
        auto& cams = _registry.storage<Camera>();
        for (auto itr = cams.rbegin(), last = cams.rend(); itr != last; ++itr)
        {
            itr->renderReset();
        }
        _renderChain.renderReset();
    }

    void SceneImpl::destroyPendingEntities() noexcept
    {
        std::vector<Entity> entities(_pendingDestroy);
        _pendingDestroy.clear();

        for (auto entity : entities)
        {
            _registry.destroy(entity);
        }
    }

    void SceneImpl::update(float deltaTime)
    {
        destroyPendingEntities();

        if (!_paused)
        {
            auto entities = _scene.getUpdateEntities<Camera>();
            for (auto entity : entities)
            {
                _scene.getComponent<Camera>(entity)->update(deltaTime);
            }

            entities = _scene.getUpdateEntities<Transform>();
            for (auto entity : entities)
            {
                _scene.getComponent<Transform>(entity)->update();
            }
            
            for (auto& comp : copyComponentContainer())
            {
                comp.get().update(deltaTime);
            }

            _renderChain.update(deltaTime);
        }
        
        updateRenderGraph();
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

    void SceneImpl::setUpdateFilter(const TypeFilter& filter) noexcept
    {
        _updateFilter = filter;
    }

    const TypeFilter& SceneImpl::getUpdateFilter() const noexcept
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
        init(app);
    }

    Scene::~Scene() noexcept
    {
    }

    std::string Scene::toString() const noexcept
    {
        return _impl->toString();
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

    Entity Scene::createEntity() noexcept
    {
        return getRegistry().create();
    }

    void Scene::destroyEntity(Entity entity) noexcept
    {
        return _impl->destroyEntity(entity);
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
        for (auto itr = storage->rbegin(), last = storage->rend(); itr < last; ++itr)
        {
            if (storage->value(*itr) == ptr)
            {
                return *itr;
            }
        }
        return entt::null;
    }

    EntityRuntimeView Scene::createEntityRuntimeView(const TypeFilter& filter) const noexcept
    {
        EntityRuntimeView view;
        auto& reg = getRegistry();
        for (auto include : filter.getIncludes())
        {
            if (auto store = reg.storage(include))
            {
                view.iterate(*store);
            }
        }
        for (auto exclude : filter.getExcludes())
        {
            if (auto store = reg.storage(exclude))
            {
                view.exclude(*store);
            }
        }
        return view;
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

    void Scene::init(App& app)
    {
        _impl->init(app);
    }

    void Scene::renderReset()
    {
        _impl->renderReset();
    }

    void Scene::shutdown()
    {
        _impl->shutdown();
    }

    void Scene::update(float dt)
    {
        _impl->update(dt);
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

    Scene& Scene::setPaused(bool paused) noexcept
    {
        _impl->setPaused(paused);
        return *this;
    }

    bool Scene::isPaused() const noexcept
    {
        return _impl->isPaused();
    }

    RenderGraphDefinition& Scene::getRenderGraph() noexcept
    {
        return _impl->getRenderGraph();
    }

    const RenderGraphDefinition& Scene::getRenderGraph() const noexcept
    {
        return _impl->getRenderGraph();
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

    Scene& Scene::setUpdateFilter(const TypeFilter& filter) noexcept
    {
        _impl->setUpdateFilter(filter);
        return *this;
    }

    const TypeFilter& Scene::getUpdateFilter() const noexcept
    {
        return _impl->getUpdateFilter();
    }

    SceneAppComponent::SceneAppComponent(const std::shared_ptr<Scene>& scene) noexcept
    {
        _scenes.push_back(scene == nullptr ? std::make_shared<Scene>() : scene);
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
        if (i < 0)
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
            oldScene->shutdown();
        }
        _scenes[i] = scene;
        if (_app && scene)
        {
            scene->init(*_app);
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
        _scenes.push_back(scene);
        if (_app && scene)
        {
            scene->init(*_app);
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
            scene->init(app);
        }
    }

    void SceneAppComponent::renderReset()
    {
        for (auto& scene : _scenes)
        {
            if (scene)
            {
                scene->renderReset();
            }
        }
    }

    void SceneAppComponent::shutdown()
    {
        _app = nullptr;
        for (auto itr = _scenes.rbegin(); itr != _scenes.rend(); ++itr)
        {
            if (auto scene = *itr)
            {
                scene->shutdown();
            }
        }
    }

    void SceneAppComponent::update(float deltaTime)
    {
        for (auto& scene : _scenes)
        {
            if (scene)
            {
                scene->update(deltaTime);
            }
        }
    }
}