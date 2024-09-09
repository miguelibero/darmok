#pragma once

#include <memory>
#include <vector>
#include <darmok/export.h>
#include <darmok/render_scene.hpp>
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
        void init(ShadowRenderer& renderer, uint16_t index, uint8_t cascade) noexcept;
        void shutdown() noexcept;
        bool configure(Entity entity = entt::null) noexcept;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(IRenderGraphContext& context) noexcept override;
    private:
        Entity _lightEntity;
        uint8_t _cascade;
        bgfx::FrameBufferHandle _fb;
        OptionalRef<ShadowRenderer> _renderer;

        void renderEntities(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept;
    };

    struct DARMOK_EXPORT ShadowRendererConfig final
    {
        unsigned int mapSize = 512;
        float cascadeMargin = 0.02F;
        uint16_t maxLightAmount = 6;
        uint8_t cascadeAmount = 3;
        float bias = 0.005;
        float normalBias = 0.02;
    };

    class DARMOK_EXPORT ShadowRenderer final : public ICameraComponent
    {
    public:
        using Config = ShadowRendererConfig;
        ShadowRenderer(const Config& config) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void update(float deltaTime) override;
        void renderReset() noexcept override;
        void shutdown() noexcept override;

        bool isEnabled() const noexcept;

        glm::mat4 getProjMatrix(uint8_t cascade = 0) const noexcept;
        glm::mat4 getLightProjMatrix(OptionalRef<const Transform> lightTrans, uint8_t cascade = 0) const noexcept;
        glm::mat4 getLightViewMatrix(OptionalRef<const Transform> lightTrans) const noexcept;
        glm::mat4 getLightMatrix(OptionalRef<const Transform> lightTrans, uint8_t cascade = 0) const noexcept;
        glm::mat4 getLightMapMatrix(OptionalRef<const Transform> lightTrans, uint8_t cascade = 0) const noexcept;

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

        std::vector<glm::mat4> _camProjs;
        glm::mat4 _crop;

        void updateCamera() noexcept;
        void updateLights() noexcept;
    };

    class DARMOK_EXPORT ShadowRenderComponent final : public ICameraComponent
    {
    public:
        ShadowRenderComponent(ShadowRenderer& renderer) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) noexcept override;
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
        void drawDebug() noexcept;
    };

    struct MeshData;

    class DARMOK_EXPORT ShadowDebugRenderer final : public ICameraComponent
    {
    public:
        ShadowDebugRenderer(ShadowRenderer& renderer) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void beforeRenderView(IRenderGraphContext& context) noexcept override;
    private:
        ShadowRenderer& _renderer;
        OptionalRef<Scene> _scene;
        std::shared_ptr<Program> _prog;
        bgfx::UniformHandle _hasTexturesUniform;
        bgfx::UniformHandle _colorUniform;

        void renderMesh(MeshData& meshData, uint8_t debugColor, IRenderGraphContext& context) noexcept;
    };
}