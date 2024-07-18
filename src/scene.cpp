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

    void SceneImpl::addSceneComponent(std::unique_ptr<ISceneComponent>&& comp) noexcept
    {
        if (_app)
        {
            comp->init(_scene, _app.value());
        }
        _components.push_back(std::move(comp));
    }

    bool SceneImpl::removeSceneComponent(const ISceneComponent& comp) noexcept
    {
        auto itr = std::find_if(_components.begin(), _components.end(), [&comp](auto& elm) { return elm.get() == &comp; });
        if (itr == _components.end())
        {
            return false;
        }
        _components.erase(itr);
        return true;
    }

    bool SceneImpl::hasSceneComponent(const ISceneComponent& comp) const noexcept
    {
        auto itr = std::find_if(_components.begin(), _components.end(), [&comp](auto& elm) { return elm.get() == &comp; });
        return itr != _components.end();
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

        for (auto& comp : _components)
        {
            comp->init(_scene, app);
        }

        for (auto [entity, cam] : _registry.view<Camera>().each())
        {
            cam.init(_scene, app);
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
        for (auto& comp : _components)
        {
            comp->shutdown();
        }

        _registry.clear();

        _registry.on_construct<Camera>().disconnect<&SceneImpl::onCameraConstructed>(*this);
        _registry.on_destroy<Camera>().disconnect< &SceneImpl::onCameraDestroyed>(*this);

        _app.reset();
    }

    void SceneImpl::updateLogic(float dt)
    {
        auto camView = _registry.view<Camera>();
        auto& renderGraph = _app->getRenderGraph();
        for (auto [entity, cam] : camView.each())
        {
            cam.update(dt);
        }

        auto transView = _registry.view<Transform>();
        for (auto [entity, trans] : transView.each())
        {
            trans.update();
        }

        for (auto& comp : _components)
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

    void Scene::shutdown()
    {
        _impl->shutdown();
    }

    void Scene::updateLogic(float dt)
    {
        _impl->updateLogic(dt);
    }

    void Scene::addSceneComponent(std::unique_ptr<ISceneComponent>&& comp) noexcept
    {
        _impl->addSceneComponent(std::move(comp));
    }

    bool Scene::removeSceneComponent(const ISceneComponent& comp) noexcept
    {
        return _impl->removeSceneComponent(comp);
    }

    bool Scene::hasSceneComponent(const ISceneComponent& comp) const noexcept
    {
        return _impl->hasSceneComponent(comp);
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

    void SceneAppComponent::shutdown()
    {
        _app = nullptr;
        if (_scene)
        {
            _scene->shutdown();
        }
    }

    void SceneAppComponent::updateLogic(float deltaTime)
    {
        if (_scene)
        {
            _scene->updateLogic(deltaTime);
        }
    }
}