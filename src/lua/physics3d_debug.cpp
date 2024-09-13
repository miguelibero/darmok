
#ifdef _DEBUG

#include "physics3d_debug.hpp"
#include "physics3d.hpp"
#include <darmok/physics3d_debug.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>

namespace darmok::physics3d
{
    PhysicsDebugRenderer& LuaPhysicsDebugRenderer::addCameraComponent1(Camera& cam) noexcept
    {
        return cam.addComponent<PhysicsDebugRenderer>();
    }

    PhysicsDebugRenderer& LuaPhysicsDebugRenderer::addCameraComponent2(Camera& cam, const Config& config) noexcept
    {
        return cam.addComponent<PhysicsDebugRenderer>(config);
    }

    OptionalRef<PhysicsDebugRenderer>::std_t LuaPhysicsDebugRenderer::getCameraComponent(Camera& cam) noexcept
    {
        return cam.getComponent<PhysicsDebugRenderer>();
    }

    void LuaPhysicsDebugRenderer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<PhysicsDebugConfig>("PhysicsDebugConfig", sol::default_constructor,
            "enable_event", &PhysicsDebugConfig::enableEvent
        );
        lua.new_usertype<PhysicsDebugRenderer>("PhysicsDebugRenderer",
            sol::no_constructor,
            "enabled", sol::property(&PhysicsDebugRenderer::isEnabled, &PhysicsDebugRenderer::setEnabled),
            "get_camera_component", &LuaPhysicsDebugRenderer::getCameraComponent,
            "add_camera_component", sol::overload(
                &LuaPhysicsDebugRenderer::addCameraComponent1,
                &LuaPhysicsDebugRenderer::addCameraComponent1
            )
        );
    }
}

#endif