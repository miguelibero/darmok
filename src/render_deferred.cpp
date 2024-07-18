#include <darmok/render_deferred.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
    void DeferredRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        Renderer::init(cam, scene, app);
    }

    void DeferredRenderer::shutdown() noexcept
    {
        Renderer::shutdown();
    }
}