#include "scene.hpp"
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace darmok
{
    SceneImpl::SceneImpl()
    {
    }

    SceneImpl::~SceneImpl()
    {
        // empty on purpose
    }

    void SceneImpl::addComponent(std::unique_ptr<ISceneComponent>&& comp)
    {
        if (_scene)
        {
            comp->init(_scene.value(), _app.value());
        }
        _components.push_back(std::move(comp));
    }

    EntityRegistry& SceneImpl::getRegistry()
    {
        return _registry;
    }

    const EntityRegistry& SceneImpl::getRegistry() const
    {
        return _registry;
    }

    void SceneImpl::init(Scene& scene, App& app)
    {
        if (_scene == scene && _app == app)
        {
            return;
        }

        if (_scene || _app)
        {
            shutdown();
        }

        _scene = scene;
        _app = app;

        for (auto& comp : _components)
        {
            comp->init(scene, app);
        }

        for (auto [entity, cam] : _registry.view<Camera>().each())
        {
            cam.init(scene, app);
        }

        _registry.on_construct<Camera>().connect<&SceneImpl::onCameraConstructed>(*this);
        _registry.on_destroy<Camera>().connect< &SceneImpl::onCameraDestroyed>(*this);
    }

    void SceneImpl::onCameraConstructed(EntityRegistry& registry, Entity entity)
    {
        if (_scene && _app)
        {
            auto& cam = registry.get<Camera>(entity);
            cam.init(_scene.value(), _app.value());
        }
    }

    void SceneImpl::onCameraDestroyed(EntityRegistry& registry, Entity entity)
    {
        if (_scene && _app)
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

        _scene.reset();
        _app.reset();
    }

    void SceneImpl::updateLogic(float dt)
    {
        auto camView = _registry.view<Camera>();
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

    bgfx::ViewId SceneImpl::render(bgfx::ViewId viewId)
    {
        auto encoder = bgfx::begin();
        for (auto [entity, cam] : _registry.view<const Camera>().each())
        {
            viewId = cam.render(*encoder, viewId);
        }
        bgfx::end(encoder);
        return viewId;
    }

    Scene::Scene() noexcept
    : _impl(std::make_unique<SceneImpl>())
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

    void Scene::init(App& app)
    {
        _impl->init(*this, app);
    }

    void Scene::shutdown()
    {
        _impl->shutdown();
    }

    void Scene::updateLogic(float dt)
    {
        _impl->updateLogic(dt);
    }

    bgfx::ViewId Scene::render(bgfx::ViewId viewId)
    {
        return _impl->render(viewId);
    }

    void Scene::addComponent(std::unique_ptr<ISceneComponent>&& comp)
    {
        _impl->addComponent(std::move(comp));
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

    bgfx::ViewId SceneAppComponent::render(bgfx::ViewId viewId) const
    {
        if (_scene)
        {
            viewId = _scene->render(viewId);
        }
        return viewId;
    }

    void SceneAppComponent::updateLogic(float deltaTime)
    {
        if (_scene)
        {
            _scene->updateLogic(deltaTime);
        }
    }
}