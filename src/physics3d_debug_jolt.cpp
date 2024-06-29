#include "physics3d_debug_jolt.hpp"
#include <darmok/physics3d_debug.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
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
        , _meshCreator(program->getVertexLayout())
    {
        Initialize();
    }

    bgfx::ViewId PhysicsDebugRendererImpl::render(bgfx::ViewId viewId)
    {
        auto joltSystem = _system.getJolt();
        if (!joltSystem)
        {
            return viewId;
        }

        _encoder = bgfx::begin();
        _viewId = viewId;
        JPH::BodyManager::DrawSettings settings;
        joltSystem->DrawBodies(settings, this, nullptr);

        bgfx::end(_encoder.ptr());
        _encoder.reset();

        return ++viewId;
    }

    void PhysicsDebugRendererImpl::renderSubmit(EDrawMode mode)
    {
        auto primType = mode == EDrawMode::Wireframe ? MaterialPrimitiveType::Line : MaterialPrimitiveType::Triangle;
        _material.setPrimitiveType(primType);
        _material.renderSubmit(_encoder.value(), _viewId);
    }

    void PhysicsDebugRendererImpl::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
    {
        _cam.beforeRenderView(_encoder.value(), _viewId);

        _meshCreator.config.color = JoltUtils::convert(inColor);
        _meshCreator.config.type = MeshType::Transient;
        auto mesh = _meshCreator.createLine({ JoltUtils::convert(inFrom), JoltUtils::convert(inTo) });
        mesh->render(_encoder.value());

        renderSubmit(EDrawMode::Wireframe);
    }

    void PhysicsDebugRendererImpl::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow)
    {
        _cam.beforeRenderView(_encoder.value(), _viewId);

        _meshCreator.config.color = JoltUtils::convert(inColor);
        _meshCreator.config.type = MeshType::Transient;
        auto mesh = _meshCreator.createTriangle({ JoltUtils::convert(inV1), JoltUtils::convert(inV2), JoltUtils::convert(inV3) });
        mesh->render(_encoder.value());
        // TODO: add shadow support
        renderSubmit(EDrawMode::Solid);
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
        auto mesh = _meshCreator.createMesh(data);
        return new JoltMeshBatch(std::move(mesh));
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
        auto mesh = _meshCreator.createMesh(data);
        return new JoltMeshBatch(std::move(mesh));
    }

    void PhysicsDebugRendererImpl::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
    {
        if (inGeometry->mLODs.empty())
        {
            return;
        }
        // TODO: decide lod
        auto lod = inGeometry->mLODs[0];
        auto& batch = *(JoltMeshBatch*)lod.mTriangleBatch.GetPtr();

        _cam.beforeRenderView(_encoder.value(), _viewId);
        auto transMtx = JoltUtils::convert(inModelMatrix);
        _encoder->setTransform(glm::value_ptr(transMtx));

        batch.mesh->render(_encoder.value());
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

    bgfx::ViewId PhysicsDebugRenderer::render(bgfx::ViewId viewId) const
    {
        return _impl->render(viewId);
    }
}