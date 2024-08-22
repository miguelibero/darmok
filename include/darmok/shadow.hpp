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

    class DARMOK_EXPORT ShadowRenderer final : public IRenderer, public IRenderPass
    {
    public:
        ShadowRenderer(const glm::uvec2& mapSize = glm::uvec2(512), const glm::vec3& mapMargin = glm::vec3(0.01F)) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void update(float deltaTime) override;
        void renderReset() noexcept override;
        void shutdown() noexcept override;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(IRenderGraphContext& context) noexcept override;

        glm::mat4 getLightMapMatrix(Entity entity) const noexcept;
    private:
        glm::uvec2 _mapSize;
        glm::vec3 _mapMargin;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        std::unique_ptr<Program> _shadowProg;
        std::unique_ptr<Texture> _shadowTex;
        bgfx::FrameBufferHandle _shadowFb;
        std::vector<Entity> _lights;
        std::unordered_map<bgfx::ViewId, Entity> _lightsByViewId;
        glm::mat4 _camProjView;

        bool updateLights() noexcept;
        void updateCamera() noexcept;
        void renderEntities(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept;
        glm::mat4 getLightProjMatrix(OptionalRef<const Transform> trans) const noexcept;
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