#pragma once

#include <memory>
#include <vector>
#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/render_debug.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/program.hpp>
#include <darmok/protobuf/camera.pb.h>

namespace darmok
{
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

    class DARMOK_EXPORT ShadowRenderer final : public ITypeCameraComponent<ShadowRenderer>
    {
    public:
        using Definition = protobuf::ShadowRenderer;

        static Definition createDefinition() noexcept;

        ShadowRenderer(const Definition& def = createDefinition()) noexcept;
        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> load(const Definition& def) noexcept;
        expected<void, std::string> update(float deltaTime) noexcept override;
        expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
        expected<void, std::string> render() noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;

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
        glm::mat4 getPointLightMapMatrix(const PointLight& light, const OptionalRef<const Transform>& lightTran, uint8_t face = 0) const noexcept;

        glm::mat4 getLightViewMatrix(const OptionalRef<const Transform>& lightTrans) const noexcept;

        const Definition& getDefinition() const noexcept;
        ProgramHandle getProgramHandle() const noexcept;
        TextureHandle getTextureHandle() const noexcept;
        OptionalRef<Camera> getCamera() noexcept;
        OptionalRef<Scene> getScene() noexcept;
        OptionalRef<const Camera> getCamera() const noexcept;
        OptionalRef<const Scene> getScene() const noexcept;

    private:
        Definition _def;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        std::unique_ptr<Program> _program;
        std::vector<ShadowRenderPass> _passes;
        std::unique_ptr<Texture> _tex;
        std::vector<glm::mat4> _camProjs;
        glm::mat4 _crop;
        size_t _dirAmount;
        size_t _spotAmount;
        size_t _pointAmount;

        UniformHandle _shadowMapUniform;
        UniformHandle _shadowData1Uniform;
        UniformHandle _shadowData2Uniform;
        DynamicVertexBuffer _shadowTransBuffer;
        bgfx::VertexLayout _shadowTransLayout;
        DynamicVertexBuffer _shadowLightDataBuffer;
        bgfx::VertexLayout _shadowLightDataLayout;

        static const size_t _pointLightFaceAmount;

        void updateCamera() noexcept;
        void updateLights() noexcept;
        void updateBuffers() noexcept;
        size_t getShadowMapAmount() const noexcept;

        void configureUniforms(bgfx::Encoder& encoder) const noexcept;
        void drawDebug() noexcept;
        expected<void, std::string> doLoad() noexcept;
    };

    struct MeshData;

    class DARMOK_EXPORT ShadowDebugRenderer final : public ITypeCameraComponent<ShadowDebugRenderer>
    {
    public:
        using Definition = protobuf::ShadowDebugRenderer;

        static Definition createDefinition() noexcept;

        ShadowDebugRenderer(const OptionalRef<const Camera>& mainCam = nullptr) noexcept;
        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> load(const Definition& def) noexcept;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;
    private:
        OptionalRef<const Camera> _mainCam;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        DebugRenderer _debugRender;
    };
}