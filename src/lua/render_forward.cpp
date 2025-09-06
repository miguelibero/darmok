#include "lua/render_forward.hpp"
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>

namespace darmok
{
    ForwardRenderer& LuaForwardRenderer::addCameraComponent(Camera& cam) noexcept
    {
        return cam.addComponent<ForwardRenderer>();
    }

    OptionalRef<ForwardRenderer>::std_t LuaForwardRenderer::getCameraComponent(Camera& cam) noexcept
    {
        return cam.getComponent<ForwardRenderer>();
    }

    void LuaForwardRenderer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<ForwardRenderer>("ForwardRenderer", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<ForwardRenderer>::value),
            "add_camera_component", &LuaForwardRenderer::addCameraComponent,
            "get_camera_component", &LuaForwardRenderer::getCameraComponent
        );
    }
}