#include "physics3d_debug.hpp"
#include "physics3d.hpp"
#include "camera.hpp"
#include <darmok/physics3d_debug.hpp>
#include <darmok/material.hpp>
#include <darmok/camera.hpp>

namespace darmok::physics3d
{
    LuaPhysicsDebugRenderer::LuaPhysicsDebugRenderer(PhysicsDebugRenderer& renderer) noexcept
        : _renderer(renderer)
    {
    }

    bool LuaPhysicsDebugRenderer::getEnabled() const noexcept
    {
        return _renderer->isEnabled();
    }

    void LuaPhysicsDebugRenderer::setEnabled(bool enabled) noexcept
    {
        _renderer->setEnabled(enabled);
    }

    LuaPhysicsDebugRenderer LuaPhysicsDebugRenderer::addRenderer1(LuaCamera& cam, LuaPhysicsSystem& system) noexcept
    {
        return LuaPhysicsDebugRenderer(cam.getReal().addRenderer<PhysicsDebugRenderer>(system.getReal()));
    }

    LuaPhysicsDebugRenderer LuaPhysicsDebugRenderer::addRenderer2(LuaCamera& cam, LuaPhysicsSystem& system, const Config& config) noexcept
    {
        return LuaPhysicsDebugRenderer(cam.getReal().addRenderer<PhysicsDebugRenderer>(system.getReal(), config));
    }

    void LuaPhysicsDebugRenderer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<PhysicsDebugConfig>("PhysicsDebugConfig", sol::default_constructor,
            "material", &PhysicsDebugConfig::material,
            "binding_key", &PhysicsDebugConfig::bindingKey
        );
        lua.new_usertype<LuaPhysicsDebugRenderer>("PhysicsDebugRenderer",
            sol::no_constructor,
            "enabled", sol::property(&LuaPhysicsDebugRenderer::getEnabled, &LuaPhysicsDebugRenderer::setEnabled),
            "add_renderer", sol::overload(
                &LuaPhysicsDebugRenderer::addRenderer1,
                &LuaPhysicsDebugRenderer::addRenderer2
            )
        );
    }

}