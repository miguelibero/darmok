#pragma once

#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/texture.hpp>
#include <darmok/uniform.hpp>
#include <darmok/protobuf/camera.pb.h>

#include <memory>
#include <optional>
#include <bgfx/bgfx.h>

namespace darmok
{
    class Camera;
    class Scene;
    class App;
    class Program;
    class Mesh;
    class DeferredRenderer;

    struct DARMOK_EXPORT DeferredGeoBuffer final
    {
        DeferredGeoBuffer(
            Texture albedoMetallicTex,
            Texture normalRoughnessOcclusionTex,
            Texture emissiveTex,
            Texture depthTex
        ) noexcept;

        static expected<DeferredGeoBuffer, std::string> load(const glm::uvec2& size) noexcept;
        void configureView(bgfx::ViewId viewId) const noexcept;
        expected<void, std::string> render(bgfx::Encoder& encoder) const noexcept;
        glm::uvec2 getSize() const noexcept;

    private:
        Texture _albedoMetallicTex;
        Texture _normalRoughnessOcclusionTex;
        Texture _emissiveTex;
        Texture _depthTex;
        FrameBufferOwnedHandle _handle;

        UniformHandle _albedoMetallicUniform;
        UniformHandle _normalRoughnessOcclusionUniform;
        UniformHandle _emissiveUniform;
        UniformHandle _depthUniform;
    };

    class DARMOK_EXPORT DeferredLightingRenderStep final : public IRenderChainStep
    {
    public:
        DeferredLightingRenderStep(const std::shared_ptr<Program>& prog) noexcept;
        ~DeferredLightingRenderStep() noexcept;

        expected<void, std::string> init(RenderChain& chain) noexcept override;
        expected<void, std::string> updateRenderChain(FrameBuffer& read, OptionalRef<FrameBuffer> write) noexcept override;
        expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
        expected<void, std::string> render(bgfx::Encoder& encoder) noexcept override;
        expected<void, std::string> shutdown() noexcept override;

        void setCamera(OptionalRef<Camera> cam) noexcept;
        void setGeoBuffer(const DeferredGeoBuffer& gbuffer) noexcept;

    private:
        std::shared_ptr<Program> _prog;
        std::unique_ptr<Mesh> _mesh;
        OptionalRef<RenderChain> _chain;
        OptionalRef<Camera> _cam;
        OptionalRef<FrameBuffer> _writeBuffer;
        std::optional<bgfx::ViewId> _viewId;
        OptionalRef<const DeferredGeoBuffer> _gbuffer;
        BasicUniforms _basicUniforms;
    };

    class DARMOK_EXPORT DeferredRenderer final : public ITypeCameraComponent<DeferredRenderer>
    {
    public:
        using Definition = protobuf::DeferredRenderer;

        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> load(const Definition& def) noexcept;
        expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
        expected<void, std::string> render() noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;

        static Definition createDefinition() noexcept;

    private:
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        OptionalRef<MaterialAppComponent> _materials;
        OptionalRef<DeferredLightingRenderStep> _lightingStep;

        std::shared_ptr<Program> _geometryProg;
        std::optional<bgfx::ViewId> _viewId;
        std::optional<DeferredGeoBuffer> _gbuffer;

        expected<void, std::string> recreateGeoBuffer() noexcept;
    };
}
