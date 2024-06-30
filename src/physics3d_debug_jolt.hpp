#pragma once

#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include "jolt.hpp"
#include <Jolt/Renderer/DebugRenderer.h>

namespace darmok
{
    class Transform;
    class Scene;
    class App;
    class Camera;
    class Program;
}

namespace darmok::physics3d
{
    struct JoltMeshBatch final : public JPH::RefTarget<JoltMeshBatch>, public JPH::RefTargetVirtual
    {
        JoltMeshBatch(std::unique_ptr<IMesh>&& mesh) noexcept;
        void AddRef() noexcept override;
        void Release() noexcept override;

        std::unique_ptr<IMesh> mesh;
    };

    class PhysicsSystemImpl;

    class PhysicsDebugRendererImpl final : public JPH::DebugRenderer
    {
    public:
        PhysicsDebugRendererImpl(PhysicsSystemImpl& system, const std::shared_ptr<Program>& program = nullptr) noexcept;
        void init(Camera& cam, Scene& scene, App& app);
        void shutdown();
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId);

        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
        void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow = ECastShadow::Off) override;
        JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Triangle* inTriangles, int inTriangleCount) override;
        JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) override;
        void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid) override;
        void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f) override;
    private:
        PhysicsSystemImpl& _system;
        OptionalRef<Camera> _cam;
        Material _material;
        OptionalRef<bgfx::Encoder> _encoder;
        bgfx::ViewId _viewId;
        bgfx::VertexLayout _vertexLayout;
        MeshData _drawLines;
        MeshData _drawTris;

        void renderMesh(const IMesh& mesh, EDrawMode mode = EDrawMode::Solid);
    };
}