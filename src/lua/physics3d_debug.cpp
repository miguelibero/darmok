
#ifdef _DEBUG

#include "physics3d_debug.hpp"
#include "physics3d.hpp"
#include <darmok/physics3d_debug.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>

namespace darmok::physics3d
{
    PhysicsDebugCameraComponent& LuaPhysicsDebugCameraComponent::addCameraComponent1(Camera& cam) noexcept
    {
        return cam.addComponent<PhysicsDebugCameraComponent>();
    }

    PhysicsDebugCameraComponent& LuaPhysicsDebugCameraComponent::addCameraComponent2(Camera& cam, const Config& config) noexcept
    {
        return cam.addComponent<PhysicsDebugCameraComponent>(config);
    }

    void LuaPhysicsDebugCameraComponent::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<PhysicsDebugConfig>("PhysicsDebugConfig", sol::default_constructor,
            "program", &PhysicsDebugConfig::program,
            "enable_event", &PhysicsDebugConfig::enableEvent
        );
        lua.new_usertype<PhysicsDebugCameraComponent>("PhysicsDebugCameraComponent",
            sol::no_constructor,
            "enabled", sol::property(&PhysicsDebugCameraComponent::isEnabled, &PhysicsDebugCameraComponent::setEnabled),
            "add_camera_component", sol::overload(
                &LuaPhysicsDebugCameraComponent::addCameraComponent1,
                &LuaPhysicsDebugCameraComponent::addCameraComponent1
            )
        );
    }
}

#endif