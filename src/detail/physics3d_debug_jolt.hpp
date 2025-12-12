#pragma once

#ifdef JPH_DEBUG_RENDERER

#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/physics3d_debug.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <mutex>

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

namespace JPH
{
    class PhysicsSystem;
}

namespace darmok::physics3d
{
    struct JoltMeshBatch final : public JPH::RefTarget<JoltMeshBatch>, public JPH::RefTargetVirtual
    {
        JoltMeshBatch(std::unique_ptr<Mesh>&& mesh) noexcept;
        void AddRef() noexcept override;
        void Release() noexcept override;

        std::unique_ptr<Mesh> mesh;
    };

    class PhysicsSystemImpl;
    struct PhysicsDebugConfig;

    class JoltPhysicsDebugRenderer final : public JPH::DebugRenderer
    {
    public:
        using Config = PhysicsDebugRenderConfig;

        ~JoltPhysicsDebugRenderer() noexcept;
        static expected<void, std::string> render(JPH::PhysicsSystem& joltSystem, const Config& config, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept;
        static void shutdown() noexcept;

        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) noexcept override;
        void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow = ECastShadow::Off) noexcept override;
        JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Triangle* inTriangles, int inTriangleCount) noexcept override;
        JPH::DebugRenderer::Batch CreateTriangleBatch(const JPH::DebugRenderer::Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) noexcept override;
        void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid) noexcept override;
        void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f) noexcept override;
    private:
        struct TextData final
        {
            glm::vec3 position;
            std::u32string content;
            Color color;
            float height;
        };

        static std::unique_ptr<JoltPhysicsDebugRenderer> _instance;
        static std::mutex _instanceLock;

        Config _config;
        OptionalRef<bgfx::Encoder> _encoder;
        std::optional<bgfx::ViewId> _viewId;
        MeshData _solidMeshData;
        MeshData _wireMeshData;
        std::vector<TextData> _textData;
        bgfx::UniformHandle _colorUniform;
        std::shared_ptr<Program> _program;

        JoltPhysicsDebugRenderer() noexcept;
        expected<void, std::string> doRender(JPH::PhysicsSystem& joltSystem, const Config& config, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept;
        expected<void, std::string> renderMesh(const Mesh& mesh, EDrawMode mode = EDrawMode::Solid, const Color& color = Colors::white()) noexcept;
        expected<void, std::string> renderMesh(MeshData& meshData, EDrawMode mode = EDrawMode::Solid, const Color& color = Colors::white()) noexcept;
        expected<void, std::string> renderSubmit(EDrawMode mode = EDrawMode::Solid, const Color& color = Colors::white()) noexcept;
        expected<void, std::string> renderText() noexcept;

        expected<void, std::string> renderMeshBatch(MeshData& meshData, EDrawMode mode = EDrawMode::Solid) noexcept;
		void onError(std::string_view prefix, std::string_view message) noexcept;
    };

    class PhysicsDebugRendererImpl final : public IInputEventListener
    {
    public:
        using Config = PhysicsDebugConfig;
        PhysicsDebugRendererImpl(const Config& config = {}) noexcept;
        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept;
        expected<void, std::string> shutdown() noexcept;
        expected<void, std::string> beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept;

        expected<void, std::string> onInputEvent(const std::string& tag) noexcept override;

        bool isEnabled() const noexcept;
        void setEnabled(bool enabled) noexcept;

       private:
        bool _enabled;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<Input> _input;
        Config _config;
    };
}

#else

namespace darmok::physics3d
{
    class PhysicsDebugRendererImpl final
    {
    };
}

#endif
