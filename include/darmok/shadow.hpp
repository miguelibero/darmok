#pragma once

#include <memory>
#include <vector>
#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/render_debug.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/easing_fwd.hpp>

namespace darmok
{
    class Program;
    class Texture;
    class Transform;
    class ShadowRenderer;
    class SpotLight;
    class PointLight;

    class DARMOK_EXPORT ShadowRenderPass final
    {
    public:
        ShadowRenderPass();
        void init(ShadowRenderer& renderer, uint16_t index) noexcept;
        void shutdown() noexcept;
        void configure(Entity entity = entt::null, uint8_t part = 0) noexcept;

        bgfx::ViewId renderReset(bgfx::ViewId viewId) noexcept;
        void render(bgfx::Encoder& encoder) noexcept;
    private:
        std::optional<bgfx::ViewId> _viewId;
        Entity _lightEntity;
        uint8_t _part;
        bgfx::FrameBufferHandle _fb;
        OptionalRef<ShadowRenderer> _renderer;

        void renderEntities(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept;
        void configureView() noexcept;
    };

    struct DARMOK_EXPORT ShadowRendererConfig final
    {
        unsigned int mapSize = 512;
        float cascadeMargin = 0.02F;
        EasingType cascadeEasing = EasingType::QuadraticIn;
        uint16_t maxPassAmount = 20;
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

        glm::mat4 getCameraProjMatrix(uint8_t cascade = 0) const noexcept;
        
        glm::mat4 getDirLightMatrix(const OptionalRef<const Transform>& lightTrans, uint8_t cascade = 0) const noexcept;
        glm::mat4 getDirLightProjMatrix(const OptionalRef<const Transform>& lightTrans, uint8_t cascade = 0) const noexcept;
        glm::mat4 getDirLightMapMatrix(const OptionalRef<const Transform>& lightTrans, uint8_t cascade = 0) const noexcept;

        glm::mat4 getSpotLightMatrix(const SpotLight& light, const OptionalRef<const Transform>& lightTrans) const noexcept;
        glm::mat4 getSpotLightProjMatrix(const SpotLight& light) const noexcept;
        glm::mat4 getSpotLightMapMatrix(const SpotLight& light, const OptionalRef<const Transform>& lightTrans) const noexcept;

        glm::mat4 getPointLightMatrix(const PointLight& light, const OptionalRef<const Transform>& lightTrans, uint8_t face = 0) const noexcept;
        glm::mat4 getPointLightProjMatrix(const PointLight& light, uint8_t face = 0) const noexcept;
        glm::mat4 getPointLightMapMatrix(const PointLight& light, const OptionalRef<const Transform>& lightTrans, uint8_t face = 0) const noexcept;

        glm::mat4 getLightViewMatrix(const OptionalRef<const Transform>& lightTrans) const noexcept;

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
        uint32_t _softMask;
        size_t _dirAmount;
        size_t _spotAmount;
        size_t _pointAmount;

        bgfx::UniformHandle _shadowMapUniform;
        bgfx::UniformHandle _shadowData1Uniform;
        bgfx::UniformHandle _shadowData2Uniform;
        bgfx::DynamicVertexBufferHandle _shadowTransBuffer;
        bgfx::VertexLayout _shadowTransLayout;
        bgfx::DynamicVertexBufferHandle _shadowDirBuffer;
        bgfx::VertexLayout _shadowDirLayout;

        static const size_t _pointLightFaceAmount;

        void updateCamera() noexcept;
        void updateLights() noexcept;
        void updateTransBuffer() noexcept;
        void updateDirBuffer() noexcept;

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
        DebugRenderer _debugRender;
    };
}