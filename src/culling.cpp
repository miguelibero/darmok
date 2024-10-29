#include <darmok/culling.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/shape.hpp>
#include <darmok/physics3d.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace darmok
{
    struct CullingUtils final
    {
        static std::optional<BoundingBox> getEntityBounds(Scene& scene, Entity& entity) noexcept
        {
            if (auto bbox = scene.getComponent<BoundingBox>(entity))
            {
                return bbox.value();
            }
            if (auto body = scene.getComponent<physics3d::PhysicsBody>(entity))
            {
                return body->getLocalBounds();
            }
            return std::nullopt;
        }

        static const EntityFilter& getEntityFilter() noexcept
        {
            static const EntityFilter filter = EntityFilter::create<Renderable>() & (EntityFilter::create<BoundingBox>() | EntityFilter::create<physics3d::PhysicsBody>());
            return filter;
        }
    };


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
        _prog = std::make_unique<Program>(StandardProgramType::Unlit);
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
        _prog.reset();
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

    void OcclusionCuller::update(float deltaTime) noexcept
    {
        updateQueries();
    }

    void OcclusionCuller::updateQueries() noexcept
    {
        if (!_scene || !_viewId || !_cam || !_cam->isEnabled())
        {
            return;
        }

        static const uint64_t state = BGFX_STATE_DEPTH_TEST_LEQUAL | BGFX_STATE_CULL_CCW;

        auto viewId = _viewId.value();
        auto& encoder = *bgfx::begin();
        auto prog = _prog->getHandle();
        auto& layout = _prog->getVertexLayout();
        auto& scene = _scene.value();
        auto entities = _cam->getEntities(CullingUtils::getEntityFilter());
        _cam->setViewTransform(viewId);
        for (auto entity : entities)
        {
            if (auto bbox = CullingUtils::getEntityBounds(scene, entity))
            {
                _cam->setEntityTransform(entity, encoder);
                MeshData meshData(bbox.value());
                meshData.type = MeshType::Transient;
                auto mesh = meshData.createMesh(layout);
                mesh->render(encoder);
                auto occlusion = getQuery(entity);
                bgfx::setCondition(occlusion, true);
                encoder.submit(viewId, prog, occlusion);
            }
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

    std::optional<bgfx::OcclusionQueryResult::Enum> OcclusionCuller::getQueryResult(Entity entity) const noexcept
    {
        auto itr = _queries.find(entity);
        if (itr != _queries.end())
        {
            auto result = bgfx::getResult(itr->second);
            return result;
        }
        return std::nullopt;
    }

    void OcclusionCuller::beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        auto itr = _queries.find(entity);
        if (itr != _queries.end())
        {
            encoder.setCondition(itr->second, true);
        }
    }

    void FrustumCuller::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
    }

    void FrustumCuller::shutdown() noexcept
    {
        _cam.reset();
        _scene.reset();
    }

    void FrustumCuller::update(float deltaTime) noexcept
    {
        updateCulled();
    }

    void FrustumCuller::updateCulled() noexcept
    {
        auto& scene = _scene.value();
        auto entities = _cam->getEntities(CullingUtils::getEntityFilter());
        Frustum camFrustum = _cam->getProjectionMatrix();
        _culled.clear();
        for (auto entity : entities)
        {
            if (auto bbox = CullingUtils::getEntityBounds(scene, entity))
            {
                Frustum frustum = camFrustum;
                if (auto trans = scene.getComponent<const Transform>(entity))
                {
                    frustum *= trans->getWorldInverse();
                }
                if (!frustum.intersect(bbox.value()))
                {
                    _culled.insert(entity);
                }
            }
        }
    }

    bool FrustumCuller::shouldEntityBeCulled(Entity entity) noexcept
    {
        return _culled.contains(entity);
    }
}