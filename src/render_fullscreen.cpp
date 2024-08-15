#include <darmok/render_fullscreen.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
    FullscreenRenderer::FullscreenRenderer() noexcept
    {
    }

    FullscreenRenderer& FullscreenRenderer::setTexture(const std::shared_ptr<Texture>& texture) noexcept
    {
        _texture = texture;
        return *this;
    }

    const std::shared_ptr<Texture>& FullscreenRenderer::getTexture() const noexcept
    {
        return _texture;
    }

    void FullscreenRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
    }

    void FullscreenRenderer::renderReset() noexcept
    {
        _cam
    }

    void FullscreenRenderer::shutdown() noexcept
    {
        _cam.reset();
    }

    void FullscreenRenderer::renderPassDefine(RenderPassDefinition& def) noexcept
    {

    }

    void FullscreenRenderer::renderPassConfigure(bgfx::ViewId viewId) noexcept
    {

    }

    void FullscreenRenderer::renderPassExecute(IRenderGraphContext& context) noexcept
    {

    }
}