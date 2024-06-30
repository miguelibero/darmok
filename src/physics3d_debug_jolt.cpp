#include "physics3d_debug_jolt.hpp"
#include <darmok/physics3d_debug.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/program_standard.hpp>
#include <darmok/texture.hpp>
#include "physics3d_jolt.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace darmok::physics3d
{
    JoltMeshBatch::JoltMeshBatch(std::unique_ptr<IMesh>&& mesh) noexcept
        : mesh(std::move(mesh))
    {
    }

    void JoltMeshBatch::AddRef() noexcept
    {
        JPH::RefTarget<JoltMeshBatch>::AddRef();
    }

    void JoltMeshBatch::Release() noexcept
    {
        if (--mRefCount == 0 && mesh != nullptr)
        {
            mesh.reset();
        }
    }

    PhysicsDebugRendererImpl::PhysicsDebugRendererImpl(PhysicsSystemImpl& system, const Camera& cam, const std::shared_ptr<Program>& program) noexcept
        : _system(system)
        , _cam(cam)
        , _material(program)
        , _viewId(-1)
    {
        Initialize();
    }

    void PhysicsDebugRendererImpl::init(App& app)
    {
        if (_material.getProgram() == nullptr)
        {
            auto prog = app.getAssets().getStandardProgramLoader()(StandardProgramType::Debug);
            _material.setProgram(prog);
        }
        _vertexLayout = _material.getProgram()->getVertexLayout();
    }

    void PhysicsDebugRendererImpl::shutdown()
    {
    }

    bgfx::ViewId PhysicsDebugRendererImpl::render(bgfx::ViewId viewId)
    {
        auto joltSystem = _system.getJolt();
        if (!joltSystem)
        {
            return viewId;
        }

        _directDraws.vertices.clear();

        _encoder = bgfx::begin();
        _viewId = viewId;
        JPH::BodyManager::DrawSettings settings;
        settings.mDrawBoundingBox = true;
        settings.mDrawGetSupportFunction = true;
        settings.mDrawSupportDirection = true;
        settings.mDrawGetSupportingFace = true;
        settings.mDrawShape = true;					
        settings.mDrawShapeWireframe = true;
        settings.mDrawBoundingBox = true;
        settings.mDrawCenterOfMassTransform = true;
        settings.mDrawWorldTransform = true;
        settings.mDrawVelocity = true;
        settings.mDrawMassAndInertia = true;
        settings.mDrawSleepStats = true;
        settings.mDrawSoftBodyVertices = true;
        settings.mDrawSoftBodyVertexVelocities = true;
        settings.mDrawSoftBodyEdgeConstraints = true;
        settings.mDrawSoftBodyBendConstraints = true;
        settings.mDrawSoftBodyVolumeConstraints = true;
        settings.mDrawSoftBodySkinConstraints = true;
        settings.mDrawSoftBodyLRAConstraints = true;
        settings.mDrawSoftBodyPredictedBounds = true;

        joltSystem->DrawBodies(settings, this, nullptr);

        if (!_directDraws.vertices.empty())
        {
            _cam.beforeRenderView(_encoder.value(), _viewId);
            renderSubmit(EDrawMode::Solid);
        }

        bgfx::end(_encoder.ptr());
        _encoder.reset();

        return ++viewId;
    }

    void PhysicsDebugRendererImpl::renderSubmit(EDrawMode mode)
    {
        auto primType = mode == EDrawMode::Wireframe ? MaterialPrimitiveType::Line : MaterialPrimitiveType::Triangle;
        _material.setPrimitiveType(primType);
    }

    void PhysicsDebugRendererImpl::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
    {
        auto data = MeshData(Line(JoltUtils::convert(inTo), JoltUtils::convert(inTo)));
        data.config.color = JoltUtils::convert(inColor);
        _directDraws += data;
    }

    void PhysicsDebugRendererImpl::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow)
    {
        auto data = MeshData(darmok::Triangle(JoltUtils::convert(inV1), JoltUtils::convert(inV2), JoltUtils::convert(inV3)));
        data.config.color = JoltUtils::convert(inColor);
        _directDraws += data;
        // TODO: add shadow support
    }

    JPH::DebugRenderer::Batch PhysicsDebugRendererImpl::CreateTriangleBatch(const JPH::DebugRenderer::Triangle* inTriangles, int inTriangleCount)
    {
        MeshData data;
        for (int i = 0; i < inTriangleCount; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                auto vert = inTriangles[i].mV[j];
                data.vertices.emplace_back(
                    JoltUtils::convert(vert.mPosition),
                    JoltUtils::convert(vert.mNormal),
                    JoltUtils::convert(vert.mUV),
                    JoltUtils::convert(vert.mColor)
                );
            }
        }
        return new JoltMeshBatch(data.createMesh(_vertexLayout));
    }

    JPH::DebugRenderer::Batch PhysicsDebugRendererImpl::CreateTriangleBatch(const JPH::DebugRenderer::Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount)
    {
        MeshData data;
        for (int i = 0; i < inVertexCount; i++)
        {
            auto vert = inVertices[i];
            data.vertices.emplace_back(
                JoltUtils::convert(vert.mPosition),
                JoltUtils::convert(vert.mNormal),
                JoltUtils::convert(vert.mUV),
                JoltUtils::convert(vert.mColor)
            );
        }
        for (int i = 0; i < inIndexCount; i++)
        {
            data.indices.emplace_back(inIndices[i]);
        }
        return new JoltMeshBatch(data.createMesh(_vertexLayout));
    }

    void PhysicsDebugRendererImpl::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
    {
        if (inGeometry->mLODs.empty())
        {
            return;
        }
        // TODO: decide lod
        auto& lod = inGeometry->mLODs[0];
        auto& batch = *(JoltMeshBatch*)lod.mTriangleBatch.GetPtr();

        _cam.beforeRenderView(_encoder.value(), _viewId);
        auto transMtx = JoltUtils::convert(inModelMatrix);
        _encoder->setTransform(glm::value_ptr(transMtx));

        batch.mesh->render(_encoder.value());
        _material.setColor(MaterialColorType::Diffuse, JoltUtils::convert(inModelColor));
        renderSubmit(inDrawMode);
    }

    void PhysicsDebugRendererImpl::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight)
    {

    }

    PhysicsDebugRenderer::PhysicsDebugRenderer(PhysicsSystem& system, const Camera& cam, const std::shared_ptr<Program>& prog) noexcept
        : _impl(std::make_unique<PhysicsDebugRendererImpl>(system.getImpl(), cam, prog))
    {
    }

    PhysicsDebugRenderer::~PhysicsDebugRenderer() noexcept
    {
        // empty on purpose
    }

    void PhysicsDebugRenderer::init(App& app)
    {
        _impl->init(app);
    }

    bgfx::ViewId PhysicsDebugRenderer::render(bgfx::ViewId viewId) const
    {
        return _impl->render(viewId);
    }
}