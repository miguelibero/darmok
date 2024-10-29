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
        scene.onDestroyComponent<Renderable>().connect<&OcclusionCuller::onRenderableDestroyed>(*this);
    }

    void OcclusionCuller::onRenderableDestroyed(EntityRegistry& registry, Entity entity) noexcept
    {
        auto itr = _queries.find(entity);
        if (itr != _queries.end())
        {
            _freeQueries.push_back(itr->second);
            _queries.erase(itr);
        }
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
        if (_scene)
        {
            _scene->onDestroyComponent<Renderable>().disconnect<&OcclusionCuller::onRenderableDestroyed>(*this);
            _scene.reset();
        }
        _cam.reset();
        _viewId.reset();
        _materials.reset();
        for (auto& [entity, query] : _queries)
        {
            bgfx::destroy(query);

        }
        _queries.clear();
        for (auto& query : _freeQueries)
        {
            bgfx::destroy(query);

        }
        _freeQueries.clear();
    }

    void OcclusionCuller::render() noexcept
    {
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
            auto occlusion = getQuery(entity);
            _materials->renderSubmit(viewId, encoder, *renderable->getMaterial(), occlusion);
        }

        bgfx::end(&encoder);
    }

    bgfx::OcclusionQueryHandle OcclusionCuller::getQuery(Entity entity) noexcept
    {
        auto itr = _queries.find(entity);
        if (itr != _queries.end())
        {
            return itr->second;
        }
        if (!_freeQueries.empty())
        {
            auto query = _freeQueries.back();
            _freeQueries.pop_back();
            _queries.insert(itr, std::make_pair(entity, query));
            return query;
        }
        auto query = bgfx::createOcclusionQuery();
        _queries[entity] = query;
        return query;
    }

    bool OcclusionCuller::shouldEntityBeCulled(Entity entity) noexcept
    {
        auto itr = _queries.find(entity);
        if (itr == _queries.end())
        {
            return false;
        }
        auto result = bgfx::getResult(itr->second);
        return result == bgfx::OcclusionQueryResult::Invisible;
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