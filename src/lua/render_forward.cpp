#include "render_forward.hpp"
#include "camera.hpp"
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>

namespace darmok
{
    LuaForwardRenderer::LuaForwardRenderer(ForwardRenderer& renderer) noexcept
        : _renderer(renderer)
    {
    }

    LuaForwardRenderer LuaForwardRenderer::addRenderer(LuaCamera& cam) noexcept
    {
        return LuaForwardRenderer(cam.getReal().addRenderer<ForwardRenderer>());
    }

    void LuaForwardRenderer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaForwardRenderer>("ForwardRenderer", sol::no_constructor,
            "add_renderer", &LuaForwardRenderer::addRenderer
        );
    }
}