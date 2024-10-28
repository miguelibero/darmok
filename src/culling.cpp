#include <darmok/culling.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>
#include <darmok/material.hpp>
#include <darmok/scene_filter.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace darmok
{
    OcclusionCuller::OcclusionCuller() noexcept
    {
    }

    OcclusionCuller::~OcclusionCuller() noexcept
    {
        shutdown();
    }

    void OcclusionCuller::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        _materials = app.getOrAddComponent<MaterialAppComponent>();
    }

    bgfx::ViewId OcclusionCuller::renderReset(bgfx::ViewId viewId) noexcept
    {
        _viewId.reset();
        if (!_cam)
        {
            return viewId;
        }
        _viewId = viewId;
        _cam->configureView(viewId, "Occlusion Culling");
        return ++viewId;
    }

    void OcclusionCuller::shutdown() noexcept
    {
        _cam.reset();
        _scene.reset();
        _viewId.reset();
        _materials.reset();
        for (auto& query : _usedQueries)
        {
            bgfx::destroy(query);

        }
        _usedQueries.clear();
        for (auto& query : _freeQueries)
        {
            bgfx::destroy(query);

        }
        _freeQueries.clear();
    }

    void OcclusionCuller::render() noexcept
    {
        _freeQueries.insert(_freeQueries.end(), _usedQueries.begin(), _usedQueries.end());
        _usedQueries.clear();
        _culled.clear();

        if (!_scene || !_viewId || !_cam || !_cam->isEnabled())
        {
            return;
        }

        auto viewId = _viewId.value();
        auto& encoder = *bgfx::begin();
        _cam->beforeRenderView(viewId, encoder);
        auto entities = _cam->getEntities<Renderable>();
        for (auto entity : entities)
        {
            auto renderable = _scene->getComponent<const Renderable>(entity);
            if (!renderable->valid())
            {
                continue;
            }
            _cam->beforeRenderEntity(entity, viewId, encoder);
            if (!renderable->render(encoder))
            {
                continue;
            }
            auto occlusion = getQuery();
            _materials->renderSubmit(viewId, encoder, *renderable->getMaterial(), occlusion);
            auto result = bgfx::getResult(occlusion);
            if (result == bgfx::OcclusionQueryResult::Invisible)
            {
                _culled.insert(entity);
            }
        }

        bgfx::end(&encoder);
    }

    bgfx::OcclusionQueryHandle OcclusionCuller::getQuery() noexcept
    {
        if (_freeQueries.empty())
        {
            return _usedQueries.emplace_back(bgfx::createOcclusionQuery());
        }
        auto query = _freeQueries.back();
        _freeQueries.pop_back();
        _usedQueries.push_back(query);
        return query;
    }

    bool OcclusionCuller::shouldEntityBeCulled(Entity entity) noexcept
    {
        return _culled.contains(entity);
    }

    void FrustrumCuller::init(Camera& cam, Scene& scene, App& app) noexcept
    {
    }

    void FrustrumCuller::shutdown() noexcept
    {

    }

    bool FrustrumCuller::shouldEntityBeCulled(Entity entity) noexcept
    {
        return false;
    }
}