#include <darmok/culling.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/shape.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/transform.hpp>

#ifdef DARMOK_JOLT
#include <darmok/physics3d.hpp>
#endif

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
            #ifdef DARMOK_JOLT
                if (auto body = scene.getComponent<physics3d::PhysicsBody>(entity))
                {
                    return body->getLocalBounds();
                }
            #endif
            return std::nullopt;
        }

        static const EntityFilter& getEntityFilter() noexcept
        {
            static const EntityFilter filter = EntityFilter::create<Renderable>() & (EntityFilter::create<BoundingBox>()
#ifdef DARMOK_JOLT
                 | EntityFilter::create<physics3d::PhysicsBody>()
#endif
            );
            return filter;
        }
    };


    OcclusionCuller::OcclusionCuller() noexcept
    {
    }

    OcclusionCuller::~OcclusionCuller() noexcept = default;

    expected<void, std::string> OcclusionCuller::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        _prog = StandardProgramLoader::load(Program::Standard::Unlit);
        scene.onDestroyComponent<Renderable>().connect<&OcclusionCuller::onRenderableDestroyed>(*this);
        return {};
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

    expected<bgfx::ViewId, std::string> OcclusionCuller::renderReset(bgfx::ViewId viewId) noexcept
    {
        _viewId.reset();
        if (!_cam)
        {
            _frameBuffer.reset();
            return viewId;
        }
        _viewId = viewId;
        _cam->configureView(viewId, "Occlusion Culling");

        auto vp = _cam->getCombinedViewport();
        auto size = vp.origin + vp.size;
        if (!_frameBuffer || _frameBuffer->getSize() != size)
        {
            _frameBuffer = std::make_unique<FrameBuffer>(size);
        }
        _frameBuffer->configureView(viewId);

        return ++viewId;
    }

    expected<void, std::string> OcclusionCuller::shutdown() noexcept
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
        return {};
    }

    expected<void, std::string> OcclusionCuller::render() noexcept
    {
        updateQueries();
        return {};
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
            if (auto bounds = CullingUtils::getEntityBounds(scene, entity))
            {
                _cam->setEntityTransform(entity, encoder);
                MeshData meshData(Cube{ *bounds });
                meshData.type = Mesh::Definition::Transient;
                auto mesh = meshData.createMesh(layout);
                mesh.render(encoder);
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

    expected<void, std::string> OcclusionCuller::beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        auto itr = _queries.find(entity);
        if (itr != _queries.end())
        {
            encoder.setCondition(itr->second, true);
        }
        return {};
    }

    expected<void, std::string> FrustumCuller::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        return {};
    }

    expected<void, std::string> FrustumCuller::shutdown() noexcept
    {
        _cam.reset();
        _scene.reset();
        return {};
    }

    expected<void, std::string> FrustumCuller::update(float deltaTime) noexcept
    {
        updateCulled();
        return {};
    }

    void FrustumCuller::updateCulled() noexcept
    {
        auto& scene = _scene.value();
        auto entities = _cam->getEntities(CullingUtils::getEntityFilter());
        const Frustum camFrust{ _cam->getViewProjectionMatrix() };
        _culled.clear();
        for (auto entity : entities)
        {
            if (auto bounds = CullingUtils::getEntityBounds(scene, entity))
            {
                Frustum frust = camFrust;
                if (auto trans = scene.getComponent<const Transform>(entity))
                {
                    frust *= trans->getWorldInverse();
                }
                if (!frust.canSee(bounds.value()))
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

    CullingDebugRenderer::CullingDebugRenderer(const OptionalRef<const Camera>& mainCam) noexcept
        : _mainCam{ mainCam }
    {
    }

    expected<void, std::string> CullingDebugRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        _debugRender.init(app);
        return {};
    }

    expected<void, std::string> CullingDebugRenderer::shutdown() noexcept
    {
        _cam.reset();
        _scene.reset();
        _debugRender.shutdown();
        return {};
    }

    expected<void, std::string> CullingDebugRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        uint8_t debugColor = 0;
        MeshData meshData;

        auto mainCam = _mainCam ? _mainCam : _cam;
        if (mainCam)
        {
            const Frustum frust{ mainCam->getViewProjectionMatrix() };
            for (auto& plane : frust.getPlanes())
            {
                meshData += MeshData{plane, Mesh::Definition::OutlineRectangle};
                _debugRender.renderMesh(meshData, viewId, encoder, debugColor, false);
                ++debugColor;
            }
        }

        if (_cam && _scene)
        {
            auto& scene = _scene.value();
            auto entities = _cam->getEntities(CullingUtils::getEntityFilter());
            for (auto entity : entities)
            {
                if (auto bbox = CullingUtils::getEntityBounds(scene, entity))
                {
                    meshData += MeshData{ Cube{ *bbox }, Mesh::Definition::OutlineRectangle };
                    _cam->setEntityTransform(entity, encoder);
                    _debugRender.renderMesh(meshData, viewId, encoder, debugColor, true);
                    ++debugColor;
                }
            }
        }
        return {};
    }
}