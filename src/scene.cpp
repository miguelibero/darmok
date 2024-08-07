#include "scene.hpp"
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace darmok
{
    SceneImpl::SceneImpl(Scene& scene) noexcept
        : _scene(scene)
    {
    }

    SceneImpl::~SceneImpl() noexcept
    {
        // empty on purpose
    }

    void SceneImpl::addSceneComponent(entt::id_type type, std::unique_ptr<ISceneComponent>&& component) noexcept
    {
        if (_app)
        {
            component->init(_scene, _app.value());
        }
        _components.emplace_back(type, std::move(component));
    }

    bool SceneImpl::removeSceneComponent(entt::id_type type) noexcept
    {
        auto itr = findComponent(type);
        if (itr == _components.end())
        {
            return false;
        }
        _components.erase(itr);
        return true;
    }

    bool SceneImpl::hasSceneComponent(entt::id_type type) const noexcept
    {
        auto itr = findComponent(type);
        return itr != _components.end();
    }

    OptionalRef<ISceneComponent> SceneImpl::getSceneComponent(entt::id_type type) noexcept
    {
        auto itr = findComponent(type);
        if (itr == _components.end())
        {
            return nullptr;
        }
        return itr->second.get();
    }

    OptionalRef<const ISceneComponent> SceneImpl::getSceneComponent(entt::id_type type) const noexcept
    {
        auto itr = findComponent(type);
        if (itr == _components.end())
        {
            return nullptr;
        }
        return itr->second.get();
    }

    SceneImpl::Components::iterator SceneImpl::findComponent(entt::id_type type) noexcept
    {
        return std::find_if(_components.begin(), _components.end(), [type](auto& elm) { return elm.first == type; });
    }

    SceneImpl::Components::const_iterator SceneImpl::findComponent(entt::id_type type) const noexcept
    {
        return std::find_if(_components.begin(), _components.end(), [type](auto& elm) { return elm.first == type; });
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

    OptionalRef<App> SceneImpl::getApp() noexcept
    {
        return _app;
    }

    OptionalRef<const App> SceneImpl::getApp() const noexcept
    {
        return _app;
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

        for (auto& [type, comp] : _components)
        {
            comp->init(_scene, app);
        }

        auto& cams = _registry.storage<Camera>();
        for (auto itr = cams.rbegin(), last = cams.rend(); itr != last; ++itr)
        {
            itr->init(_scene, app);
        }

        _registry.on_construct<Camera>().connect<&SceneImpl::onCameraConstructed>(*this);
        _registry.on_destroy<Camera>().connect< &SceneImpl::onCameraDestroyed>(*this);
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

        for (auto itr = _components.rbegin(); itr != _components.rend(); ++itr)
        {
            itr->second->shutdown();
        }

        _registry.on_construct<Camera>().disconnect<&SceneImpl::onCameraConstructed>(*this);
        _registry.on_destroy<Camera>().disconnect< &SceneImpl::onCameraDestroyed>(*this);

        _app.reset();
    }

    void SceneImpl::renderReset()
    {
        // iteration in reverse to maintain the order in wich the cameras where added
        auto& cams = _registry.storage<Camera>();
        for (auto itr = cams.rbegin(), last = cams.rend(); itr != last; ++itr)
        {
            itr->renderReset();
        }
    }

    void SceneImpl::update(float dt)
    {
        auto& cams = _registry.storage<Camera>();
        for (auto itr = cams.rbegin(), last = cams.rend(); itr != last; ++itr)
        {
            itr->update(dt);
        }

        auto& trans = _registry.storage<Transform>();
        for (auto itr = trans.rbegin(), last = trans.rend(); itr != last; ++itr)
        {
            itr->update();
        }

        for (auto& [type, comp] : _components)
        {
            comp->update(dt);
        }
    }

    Scene::Scene() noexcept
    : _impl(std::make_unique<SceneImpl>(*this))
    {
    }

    Scene::~Scene() noexcept
    {
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

    bool Scene::destroyEntity(Entity entity) noexcept
    {
        if (entity == entt::null)
        {
            return false;
        }
        return getRegistry().destroy(entity) > 0;
    }


    SceneImpl& Scene::getImpl() noexcept
    {
        return *_impl;
    }

    const SceneImpl& Scene::getImpl() const noexcept
    {
        return *_impl;
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

    void Scene::addSceneComponent(entt::id_type type, std::unique_ptr<ISceneComponent>&& component) noexcept
    {
        return _impl->addSceneComponent(type, std::move(component));
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

    SceneAppComponent::SceneAppComponent(const std::shared_ptr<Scene>& scene) noexcept
        : _scene(scene == nullptr ? std::make_shared<Scene>() : scene)
    {
    }

    std::shared_ptr<Scene> SceneAppComponent::getScene() const noexcept
    {
        return _scene;
    }

    SceneAppComponent& SceneAppComponent::setScene(const std::shared_ptr<Scene>& scene) noexcept
    {
        if (_scene == scene)
        {
            return *this;
        }
        if (_app && _scene)
        {
            _scene->shutdown();
        }
        _scene = scene;
        if (_app && _scene)
        {
            _scene->init(*_app);
        }
        return *this;
    }

    void SceneAppComponent::init(App& app)
    {
        if (_app)
        {
            shutdown();
        }
        _app = app;
        if(_scene)
        {
            _scene->init(app);
        }
    }

    void SceneAppComponent::renderReset()
    {
        if (_scene)
        {
            _scene->renderReset();
        }
    }

    void SceneAppComponent::shutdown()
    {
        _app = nullptr;
        if (_scene)
        {
            _scene->shutdown();
        }
    }

    void SceneAppComponent::update(float deltaTime)
    {
        if (_scene)
        {
            _scene->update(deltaTime);
        }
    }
}