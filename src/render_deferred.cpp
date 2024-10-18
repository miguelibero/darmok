#include <darmok/render_deferred.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
    void DeferredRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
    }

    bgfx::ViewId DeferredRenderer::renderReset(bgfx::ViewId viewId) noexcept
    {
        return viewId;
    }

    void DeferredRenderer::shutdown() noexcept
    {
        _cam.reset();
    }
}