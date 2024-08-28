
#ifdef _DEBUG

#include "physics3d_debug.hpp"
#include "physics3d.hpp"
#include <darmok/physics3d_debug.hpp>
#include <darmok/program.hpp>
#include <darmok/render_forward.hpp>

namespace darmok::physics3d
{
    PhysicsDebugRenderComponent& LuaPhysicsDebugRenderComponent::addRenderComponent1(ForwardRenderer& renderer) noexcept
    {
        return renderer.addComponent<PhysicsDebugRenderComponent>();
    }

    PhysicsDebugRenderComponent& LuaPhysicsDebugRenderComponent::addRenderComponent2(ForwardRenderer& renderer, const Config& config) noexcept
    {
        return renderer.addComponent<PhysicsDebugRenderComponent>(config);
    }

    void LuaPhysicsDebugRenderComponent::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<PhysicsDebugConfig>("PhysicsDebugConfig", sol::default_constructor,
            "program", &PhysicsDebugConfig::program,
            "enable_event", &PhysicsDebugConfig::enableEvent
        );
        lua.new_usertype<PhysicsDebugRenderComponent>("PhysicsDebugRenderComponent",
            sol::no_constructor,
            "enabled", sol::property(&PhysicsDebugRenderComponent::isEnabled, &PhysicsDebugRenderComponent::setEnabled),
            "add_render_component", sol::overload(
                &LuaPhysicsDebugRenderComponent::addRenderComponent1,
                &LuaPhysicsDebugRenderComponent::addRenderComponent1
            )
        );
    }
}

#endif