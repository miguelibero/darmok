#pragma once

#include <memory>
#include <vector>
#include <darmok/export.h>
#include <darmok/render_scene.hpp>

namespace darmok
{
    class Program;
    class Texture;
    class Transform;
    class ShadowRenderer;

    class DARMOK_EXPORT ShadowRenderPass final
    {
    public:
        ShadowRenderPass();
        void init(ShadowRenderer& renderer, uint16_t index, uint8_t cascade) noexcept;
        void shutdown() noexcept;
        bool configure(Entity entity = entt::null) noexcept;

        bgfx::ViewId renderReset(bgfx::ViewId viewId) noexcept;
        void render(bgfx::Encoder& encoder) noexcept;
    private:
        std::optional<bgfx::ViewId> _viewId;
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

    class DARMOK_EXPORT ShadowRenderer final : public ITypeCameraComponent<ShadowRenderer>
    {
    public:
        using Config = ShadowRendererConfig;
        ShadowRenderer(const Config& config) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void update(float deltaTime) override;
        bgfx::ViewId renderReset(bgfx::ViewId viewId) noexcept override;
        void render() noexcept override;
        void shutdown() noexcept override;
        void beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;

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

        std::vector<glm::mat4> _camProjs;
        glm::mat4 _crop;

        bgfx::UniformHandle _shadowMapUniform;
        bgfx::UniformHandle _shadowDataUniform;
        bgfx::DynamicVertexBufferHandle _shadowTransBuffer;
        bgfx::VertexLayout _shadowTransLayout;

        void updateCamera() noexcept;
        void updateLights() noexcept;
        void updateBuffer() noexcept;

        void configureUniforms(bgfx::Encoder& encoder) const noexcept;
        void drawDebug() noexcept;
    };

    struct MeshData;

    class DARMOK_EXPORT ShadowDebugRenderer final : public ITypeCameraComponent<ShadowDebugRenderer>
    {
    public:
        ShadowDebugRenderer(ShadowRenderer& renderer) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;
    private:
        ShadowRenderer& _renderer;
        OptionalRef<Scene> _scene;
        std::shared_ptr<Program> _prog;
        bgfx::UniformHandle _hasTexturesUniform;
        bgfx::UniformHandle _colorUniform;

        void renderMesh(MeshData& meshData, uint8_t debugColor, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept;
    };
}