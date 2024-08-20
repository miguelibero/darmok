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

    class DARMOK_EXPORT ShadowRenderer final : public IRenderer, public IRenderPass
    {
    public:
        ShadowRenderer(const glm::uvec2& mapSize = glm::uvec2(512)) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void update(float deltaTime) override;
        void renderReset() noexcept override;
        void shutdown() noexcept override;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(IRenderGraphContext& context) noexcept override;
    private:
        glm::uvec2 _mapSize;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        std::unique_ptr<Program> _shadowProg;
        std::unique_ptr<Texture> _shadowTex;
        bgfx::FrameBufferHandle _shadowFb;
        std::vector<Entity> _lights;
        std::unordered_map<bgfx::ViewId, Entity> _lightsByViewId;
        glm::mat4 _camOrtho;

        bool updateLights() noexcept;
        void updateCamera() noexcept;
        void renderEntities(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept;
    };

    class DARMOK_EXPORT ShadowRenderComponent final : public IRenderComponent
    {
    public:
        ShadowRenderComponent() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void beforeRenderView(IRenderGraphContext& context) noexcept override;
    private:
        glm::vec4 _depthScaleOffset;
        bgfx::UniformHandle _depthScaleOffsetUniform;
        bgfx::UniformHandle _shadowMapUniform;
    };
}