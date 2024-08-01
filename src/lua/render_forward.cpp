#include "render_forward.hpp"
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>

namespace darmok
{
    ForwardRenderer& LuaForwardRenderer::addRenderer(Camera& cam) noexcept
    {
        return cam.addRenderer<ForwardRenderer>();
    }

    void LuaForwardRenderer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<ForwardRenderer>("ForwardRenderer", sol::no_constructor,
            "add_renderer", &LuaForwardRenderer::addRenderer
        );
    }
}