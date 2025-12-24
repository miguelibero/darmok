#include <darmok/render_deferred.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
    DeferredRenderer::Definition DeferredRenderer::createDefinition() noexcept
    {
        Definition def;
        return def;
    }

    expected<void, std::string> DeferredRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        return {};
    }

    expected<void, std::string> DeferredRenderer::load(const Definition& def) noexcept
    {
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