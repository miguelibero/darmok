#include "physics3d_debug_jolt.hpp"
#include <darmok/physics3d_debug.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/program_standard.hpp>
#include <darmok/texture.hpp>
#include <darmok/vertex.hpp>
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

    PhysicsDebugRendererImpl::PhysicsDebugRendererImpl(PhysicsSystemImpl& system, const std::shared_ptr<Program>& program) noexcept
        : _system(system)
        , _material(program)
        , _viewId(-1)
    {
        _drawLines.config.type = MeshType::Transient;
        _drawTris.config.type = MeshType::Transient;
    }

    void PhysicsDebugRendererImpl::init(Camera& cam, Scene& scene, App& app)
    {
        if (_material.getProgram() == nullptr)
        {
            auto prog = app.getAssets().getStandardProgramLoader()(StandardProgramType::Debug);
            _material.setProgram(prog);
        }
        _vertexLayout = _material.getProgram()->getVertexLayout();
        _cam = cam;
        Initialize();
    }

    void PhysicsDebugRendererImpl::shutdown()
    {
    }

    void PhysicsDebugRendererImpl::render(bgfx::Encoder& encoder, bgfx::ViewId viewId)
    {
        auto joltSystem = _system.getJolt();
        if (!joltSystem)
        {
            return;
        }

        _encoder = encoder;
        _viewId = viewId;

        JPH::BodyManager::DrawSettings settings;

        _cam->beforeRenderView(_encoder.value(), _viewId);

        joltSystem->DrawBodies(settings, this, nullptr);

        if (!_drawLines.empty())
        {
            auto mesh = _drawLines.createMesh(_vertexLayout);
            _drawLines.clear();
            renderMesh(*mesh, EDrawMode::Wireframe);
        }
        if (!_drawTris.empty())
        {
            auto mesh = _drawTris.createMesh(_vertexLayout);
            _drawTris.clear();
            renderMesh(*mesh, EDrawMode::Solid);
        }

        _encoder.reset();
    }

    void PhysicsDebugRendererImpl::renderMesh(const IMesh& mesh, EDrawMode mode)
    {
        _cam->beforeRenderEntity(entt::null, _encoder.value(), _viewId);
        mesh.render(_encoder.value());
        auto primType = mode == EDrawMode::Wireframe ? MaterialPrimitiveType::Line : MaterialPrimitiveType::Triangle;
        _material.setPrimitiveType(primType);
        _material.renderSubmit(_encoder.value(), _viewId);
    }

    void PhysicsDebugRendererImpl::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
    {
        MeshData data(Line(JoltUtils::convert(inFrom), JoltUtils::convert(inTo)));
        data.config.color = JoltUtils::convert(inColor);
        _drawLines += data;
    }

    void PhysicsDebugRendererImpl::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow)
    {
        MeshData data(darmok::Triangle(JoltUtils::convert(inV1), JoltUtils::convert(inV2), JoltUtils::convert(inV3)));
        data.config.color = JoltUtils::convert(inColor);
        _drawTris += data;
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

        auto transMtx = JoltUtils::convert(inModelMatrix);
        _encoder->setTransform(glm::value_ptr(transMtx));

        batch.mesh->render(_encoder.value());
        auto oldColor = _material.getColor(MaterialColorType::Diffuse);
        _material.setColor(MaterialColorType::Diffuse, JoltUtils::convert(inModelColor));
        renderMesh(*batch.mesh, inDrawMode);
        _material.setColor(MaterialColorType::Diffuse, oldColor);
    }

    void PhysicsDebugRendererImpl::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight)
    {
        // TODO: render text support
    }

    PhysicsDebugRenderer::PhysicsDebugRenderer(PhysicsSystem& system, const std::shared_ptr<Program>& prog) noexcept
        : _impl(std::make_unique<PhysicsDebugRendererImpl>(system.getImpl(), prog))
    {
    }

    PhysicsDebugRenderer::~PhysicsDebugRenderer() noexcept
    {
        // empty on purpose
    }

    void PhysicsDebugRenderer::init(Camera& cam, Scene& scene, App& app)
    {
        _impl->init(cam, scene, app);
    }

    void PhysicsDebugRenderer::shutdown()
    {
        _impl->shutdown();
    }

    void PhysicsDebugRenderer::afterRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId)
    {
        _impl->render(encoder, viewId);
    }
}