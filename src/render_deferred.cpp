#include <darmok/render_deferred.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
    void DeferredRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        renderReset();
    }

    void DeferredRenderer::renderReset() noexcept
    {
        _cam->getRenderGraph().addPass(_geoPass);
    }

    void DeferredRenderer::shutdown() noexcept
    {
        _cam.reset();
    }

    void DeferredGeometryRenderPass::renderPassConfigure(bgfx::ViewId viewId)
    {
        _viewId = viewId;
    }

    void DeferredGeometryRenderPass::renderPassExecute(IRenderGraphContext& context)
    {

    }

    void DeferredGeometryRenderPass::renderPassDefine(RenderPassDefinition& def)
    {
        def.setName("Deferred Geometry");
        def.getWriteResources().add<Texture>("geometry");
    }
}