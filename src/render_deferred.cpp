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
        if (_cam)
        {
            _cam->getRenderGraph().addPass(_geoPass);
        }
    }

    void DeferredRenderer::shutdown() noexcept
    {
        if (_cam)
        {
            _cam->getRenderGraph().removePass(_geoPass);
        }
        _cam.reset();
    }

    void DeferredGeometryRenderPass::renderPassDefine(RenderPassDefinition& def) noexcept
    {
        def.setName("Deferred Geometry");
        def.getWriteResources().add<Texture>("geometry");
    }

    void DeferredGeometryRenderPass::renderPassConfigure(bgfx::ViewId viewId)
    {
        _viewId = viewId;
    }

    void DeferredGeometryRenderPass::renderPassExecute(IRenderGraphContext& context)
    {
    }
}