#include <darmok/render_deferred.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
    expected<void, std::string> DeferredRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        return {};
    }

    expected<bgfx::ViewId, std::string> DeferredRenderer::renderReset(bgfx::ViewId viewId) noexcept
    {
        return viewId;
    }

    expected<void, std::string> DeferredRenderer::shutdown() noexcept
    {
        _cam.reset();
        return {};
    }
}