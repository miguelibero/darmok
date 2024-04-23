#include "scene.hpp"
#include <darmok/asset.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace darmok
{
    SceneImpl::SceneImpl()
    {
    }

    void SceneImpl::addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater)
    {
        if (_scene)
        {
            updater->init(_scene.value(), _app.value());
        }
        _logicUpdaters.push_back(std::move(updater));
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

        for (auto& updater : _logicUpdaters)
        {
            updater->init(scene, app);
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
        for (auto& updater : _logicUpdaters)
        {
            updater->shutdown();
        }

        _registry.clear();

        _scene = nullptr;
        _app = nullptr;

        _registry.on_construct<Camera>().disconnect<&SceneImpl::onCameraConstructed>(*this);
        _registry.on_destroy<Camera>().disconnect< &SceneImpl::onCameraDestroyed>(*this);
    }

    void SceneImpl::updateLogic(float dt)
    {
        for (auto [entity, cam] : _registry.view<Camera>().each())
        {
            cam.update(dt);
        }

        for (auto [entity, trans] : _registry.view<Transform>().each())
        {
            trans.update();
        }

        for (auto& updater : _logicUpdaters)
        {
            updater->update(dt);
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

    void Scene::addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater)
    {
        _impl->addLogicUpdater(std::move(updater));
    }

    SceneAppComponent::SceneAppComponent(const std::shared_ptr<Scene>& scene) noexcept
        : _mainScene(scene == nullptr ? std::make_shared<Scene>() : scene)
    {
        addScene(_mainScene);
    }

    const std::shared_ptr<Scene>& SceneAppComponent::getScene() const noexcept
    {
        return _mainScene;
    }

    void SceneAppComponent::setScene(const std::shared_ptr<Scene>& scene) noexcept
    {
        if (_mainScene == scene)
        {
            return;
        }
        if (_mainScene != nullptr)
        {
            removeScene(_mainScene);
        }
        _mainScene = scene;
        if (_mainScene != nullptr)
        {
            addScene(_mainScene);
        }
    }

    const std::unordered_set<std::shared_ptr<Scene>>& SceneAppComponent::getScenes() const noexcept
    {
        return _scenes;
    }

    std::shared_ptr<Scene> SceneAppComponent::addScene() noexcept
    {
        auto scene = std::make_shared<Scene>();
        _scenes.insert(scene);
        if (_app)
        {
            scene->init(_app.value());
        }
        return scene;
    }

    bool SceneAppComponent::addScene(const std::shared_ptr<Scene>& scene) noexcept
    {
        if (scene == nullptr)
        {
            return false;
        }
        auto r = _scenes.insert(scene);
        if (!r.second)
        {
            return false;
        }
        if (_app)
        {
            scene->init(_app.value());
        }
        return true;
    }

    bool SceneAppComponent::removeScene(const std::shared_ptr<Scene>& scene) noexcept
    {
        if (scene == nullptr)
        {
            return false;
        }
        auto count = _scenes.erase(scene);
        if (!count)
        {
            return false;
        }
        if (_app)
        {
            scene->shutdown();
        }
        return true;
    }

    void SceneAppComponent::init(App& app)
    {
        _app = app;
        for (auto& scene : _scenes)
        {
            if (scene != nullptr)
            {
                scene->init(app);
            }
        }
    }

    void SceneAppComponent::shutdown()
    {
        _app = nullptr;
        for (auto& scene : _scenes)
        {
            if (scene != nullptr)
            {
                scene->shutdown();
            }
        }
    }

    bgfx::ViewId SceneAppComponent::render(bgfx::ViewId viewId)
    {
        for (auto& scene : _scenes)
        {
            if (scene != nullptr)
            {
                viewId = scene->render(viewId);
            }
        }
        return viewId;
    }

    void SceneAppComponent::updateLogic(float deltaTime)
    {
        for (auto& scene : _scenes)
        {
            if (scene != nullptr)
            {
                scene->updateLogic(deltaTime);
            }
        }
    }
}