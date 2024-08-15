#pragma once

#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/physics3d_debug.hpp>
#include <darmok/render_graph.hpp>
#include "jolt.hpp"
#include <Jolt/Renderer/DebugRenderer.h>

namespace darmok
{
    class Transform;
    class Scene;
    class App;
    class Camera;
    class Program;
    class Input;
    class IFont;
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
    struct PhysicsDebugConfig;

    class PhysicsDebugRendererImpl final : public JPH::DebugRenderer, public IRenderer, public IRenderPass, public IInputEventListener
    {
    public:
        using Config = PhysicsDebugConfig;
        PhysicsDebugRendererImpl(const Config& config = {}) noexcept;
        void init(Camera& cam, Scene& scene, App& app);
        void shutdown();
        void renderReset();

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(IRenderGraphContext& context) override;

        void onInputEvent(const std::string& tag) noexcept override;

        bool isEnabled() const noexcept;
        void setEnabled(bool enabled) noexcept;
        void setFont(const std::shared_ptr<IFont>& font) noexcept;
        void setMeshBatchSize(size_t size) noexcept;

        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
        void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow = ECastShadow::Off) override;
        JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Triangle* inTriangles, int inTriangleCount) override;
        JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) override;
        void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid) override;
        void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f) override;
    private:
        struct TextData final
        {
            glm::vec3 position;
            Utf8Vector content;
            Color color;
            float height;
        };
        size_t _meshBatchSize;
        bool _enabled;
        OptionalRef<PhysicsSystemImpl> _system;
        OptionalRef<Camera> _cam;
        OptionalRef<Input> _input;
        Config _config;
        OptionalRef<bgfx::Encoder> _encoder;
        bgfx::ViewId _viewId;
        bgfx::VertexLayout _vertexLayout;
        std::shared_ptr<IFont> _font;
        MeshData _solidMeshData;
        MeshData _wireMeshData;
        std::vector<TextData> _textData;
        bgfx::UniformHandle _colorUniform;

        void renderMesh(const IMesh& mesh, EDrawMode mode = EDrawMode::Solid, const Color& color = Colors::white());
        void renderMesh(MeshData& meshData, EDrawMode mode = EDrawMode::Solid, const Color& color = Colors::white());
        void renderSubmit(EDrawMode mode = EDrawMode::Solid, const Color& color = Colors::white());
        void renderText();

        bool tryRenderMeshBatch(MeshData& meshData, EDrawMode mode = EDrawMode::Solid);
    };
}