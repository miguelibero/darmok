#pragma once

#include <memory>
#include <unordered_map>
#include <darmok/export.h>
#include <darmok/render.hpp>
#include <darmok/render_graph.hpp>

namespace darmok
{
    class Program;
    class Texture;
    class Transform;
    class ShadowRenderer;

    class DARMOK_EXPORT ShadowRenderPass final : public IRenderPass
    {
    public:
        ShadowRenderPass();
        void init(ShadowRenderer& renderer, uint16_t index) noexcept;
        void shutdown() noexcept;
        bool setLightEntity(Entity entity) noexcept;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(IRenderGraphContext& context) noexcept override;
    private:
        Entity _lightEntity;
        bgfx::FrameBufferHandle _fb;
        OptionalRef<ShadowRenderer> _renderer;

        void renderEntities(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept;
    };

    struct DARMOK_EXPORT ShadowRendererConfig final
    {
        glm::uvec2 mapSize = glm::uvec2(512);
        glm::vec3 mapMargin = glm::vec3(0.01F);
        uint16_t maxLightAmount = 16;
    };

    class DARMOK_EXPORT ShadowRenderer final : public IRenderer
    {
    public:
        using Config = ShadowRendererConfig;
        ShadowRenderer(const Config& config) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void update(float deltaTime) override;
        void renderReset() noexcept override;
        void shutdown() noexcept override;

        glm::mat4 getMapMatrix(Entity entity) const noexcept;
        glm::mat4 getProjMatrix(OptionalRef<const Transform> trans) const noexcept;

        const Config& getConfig() const noexcept;
        bgfx::ProgramHandle getProgramHandle() const noexcept;
        bgfx::TextureHandle getTextureHandle() const noexcept;
        OptionalRef<Camera> getCamera() noexcept;
        OptionalRef<Scene> getScene() noexcept;

    private:
        Config _config;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        std::unique_ptr<Program> _program;
        std::vector<ShadowRenderPass> _passes;
        std::unique_ptr<Texture> _tex;
        RenderGraphDefinition _renderGraph;

        glm::mat4 _camProjView;

        void updateCamera() noexcept;
        void updateLights() noexcept;
    };

    class DARMOK_EXPORT ShadowRenderComponent final : public IRenderComponent
    {
    public:
        ShadowRenderComponent(ShadowRenderer& renderer) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) noexcept override;
        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void beforeRenderView(IRenderGraphContext& context) noexcept override;
        void beforeRenderEntity(Entity entity, IRenderGraphContext& context) noexcept override;
    private:
        ShadowRenderer& _renderer;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        bgfx::UniformHandle _shadowMapUniform;
        bgfx::UniformHandle _shadowDataUniform;
        bgfx::DynamicVertexBufferHandle _shadowTransBuffer;
        bgfx::VertexLayout _shadowTransLayout;

        void configureUniforms(IRenderGraphContext& context) const noexcept;
    };
}