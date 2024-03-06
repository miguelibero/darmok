#include "scene.hpp"
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace darmok
{
    SceneImpl::SceneImpl()
        : _init(false)
    {
    }

    void SceneImpl::addRenderer(std::unique_ptr<ISceneRenderer>&& renderer)
    {
        if (_init)
        {
            renderer->init(_registry);
        }
        _renderers.push_back(std::move(renderer));
    }

    void SceneImpl::addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater)
    {
        if (_init)
        {
            updater->init(_registry);
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

    void SceneImpl::init()
    {
        for (auto& renderer : _renderers)
        {
            renderer->init(_registry);
        }
        for (auto& updater : _logicUpdaters)
        {
            updater->init(_registry);
        }

        _init = true;
    }

    void SceneImpl::updateLogic(float dt)
    {
        auto cams = _registry.view<Camera>();
        for (auto [entity, cam] : cams.each())
        {
            auto filter = cam.getEntityFilter();
            if (filter.hasValue())
            {
                filter.value().init(_registry);
            }
        }

        auto transforms = _registry.view<Transform>();
        for (auto [entity, trans] : transforms.each())
        {
            trans.update();
        }

        for (auto& updater : _logicUpdaters)
        {
            updater->update(dt);
        }
    }

    void SceneImpl::render(bgfx::ViewId viewId)
    {
        auto encoder = bgfx::begin();

        auto cams = _registry.view<const Camera>();
        for (auto [entity, cam] : cams.each())
        {
            if (cam.getViewId() != viewId)
            {
                continue;
            }
            auto& proj = cam.getMatrix();
            auto trans = _registry.try_get<const Transform>(entity);
            const void* viewPtr = nullptr;
            if (trans != nullptr)
            {
                viewPtr = glm::value_ptr(trans->getInverse());
            }

            bgfx::setViewTransform(viewId, viewPtr, glm::value_ptr(proj));

            auto viewRect = _registry.try_get<const ViewRect>(entity);
            if (viewRect != nullptr)
            {
                viewRect->bgfxConfig(viewId);
            }

            auto filter = cam.getEntityFilter();

            for (auto& renderer : _renderers)
            {
                EntityRuntimeView view;
                if (filter.hasValue())
                {
                    filter.value()(view);
                }                
                renderer->render(view, *encoder, viewId);
            }
        }
        bgfx::end(encoder);
    }

    Scene::Scene()
    : _impl(std::make_unique<SceneImpl>())
    {
    }

    Scene::~Scene()
    {
    }

    Entity Scene::createEntity()
    {
        return _impl->getRegistry().create();
    }

    EntityRegistry& Scene::getRegistry()
    {
        return _impl->getRegistry();
    }

    const EntityRegistry& Scene::getRegistry() const
    {
        return _impl->getRegistry();
    }

    void Scene::init()
    {
        _impl->init();
    }

    void  Scene::updateLogic(float dt)
    {
        _impl->updateLogic(dt);
    }

    void Scene::render(bgfx::ViewId viewId)
    {
        _impl->render(viewId);
    }

    void Scene::addRenderer(std::unique_ptr<ISceneRenderer>&& renderer)
    {
        _impl->addRenderer(std::move(renderer));
    }

    void Scene::addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater)
    {
        _impl->addLogicUpdater(std::move(updater));
    }

    Scene& SceneAppComponent::getScene()
    {
        return _scene;
    }

    const Scene& SceneAppComponent::getScene() const
    {
        return _scene;
    }

    void SceneAppComponent::init()
    {
        _scene.init();
    }

    void SceneAppComponent::render(bgfx::ViewId viewId)
    {
        _scene.render(viewId);
    }

    void SceneAppComponent::updateLogic(float dt)
    {
        _scene.updateLogic(dt);
    }
}