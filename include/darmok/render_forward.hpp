#pragma once

#include <memory>
#include <darmok/export.h>
#include <darmok/render.hpp>
#include <darmok/render_graph.hpp>

namespace darmok
{
    class Program;
    class Scene;
    class App;
    class Material;

    class DARMOK_EXPORT ForwardRenderer final : public Renderer, public IRenderPass
    {
    public:
        ForwardRenderer() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(RenderGraphResources& res) noexcept override;

    private:
        bgfx::ViewId _viewId;
    };
}