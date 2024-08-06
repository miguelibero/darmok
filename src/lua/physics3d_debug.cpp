
#ifdef _DEBUG

#include "physics3d_debug.hpp"
#include "physics3d.hpp"
#include <darmok/physics3d_debug.hpp>
#include <darmok/material.hpp>
#include <darmok/camera.hpp>

namespace darmok::physics3d
{

    PhysicsDebugRenderer& LuaPhysicsDebugRenderer::addRenderer1(Camera& cam) noexcept
    {
        return cam.addRenderer<PhysicsDebugRenderer>();
    }

    PhysicsDebugRenderer& LuaPhysicsDebugRenderer::addRenderer2(Camera& cam, const Config& config) noexcept
    {
        return cam.addRenderer<PhysicsDebugRenderer>(config);
    }

    void LuaPhysicsDebugRenderer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<PhysicsDebugConfig>("PhysicsDebugConfig", sol::default_constructor,
            "material", &PhysicsDebugConfig::material,
            "enable_event", &PhysicsDebugConfig::enableEvent
        );
        lua.new_usertype<PhysicsDebugRenderer>("PhysicsDebugRenderer",
            sol::no_constructor,
            "enabled", sol::property(&PhysicsDebugRenderer::isEnabled, &PhysicsDebugRenderer::setEnabled),
            "add_renderer", sol::overload(
                &LuaPhysicsDebugRenderer::addRenderer1,
                &LuaPhysicsDebugRenderer::addRenderer2
            )
        );
    }
}

#endif