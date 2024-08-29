#include "render_forward.hpp"
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>

namespace darmok
{
    ForwardRenderer& LuaForwardRenderer::addCameraComponent(Camera& cam) noexcept
    {
        return cam.addComponent<ForwardRenderer>();
    }

    void LuaForwardRenderer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<ForwardRenderer>("ForwardRenderer", sol::no_constructor,
            "add_camera_component", &LuaForwardRenderer::addCameraComponent
        );
    }
}