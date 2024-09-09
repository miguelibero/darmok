#pragma once

#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_graph.hpp>

namespace darmok
{
    class Camera;

    class DeferredGeometryRenderPass final : public IRenderPass
    {
    public:
        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) override;
        void renderPassExecute(IRenderGraphContext& context) override;
    private:
        bgfx::ViewId _viewId;
    };

    class DARMOK_EXPORT DeferredRenderer final : public ICameraComponent
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void renderReset() noexcept override;
        void shutdown() noexcept override;
    private:
        OptionalRef<Camera> _cam;
        std::shared_ptr<DeferredGeometryRenderPass> _geoPass;
    };
}