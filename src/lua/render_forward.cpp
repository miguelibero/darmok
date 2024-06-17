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

    LuaForwardRenderer LuaForwardRenderer::setCameraRenderer(LuaCamera& cam) noexcept
    {
        return LuaForwardRenderer(cam.getReal().setRenderer<ForwardRenderer>());
    }

    void LuaForwardRenderer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaForwardRenderer>("ForwardRenderer", sol::no_constructor,
            "set_camera_renderer", &LuaForwardRenderer::setCameraRenderer
        );
    }
}