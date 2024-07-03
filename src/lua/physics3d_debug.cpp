#include "physics3d_debug.hpp"
#include "physics3d.hpp"
#include "camera.hpp"
#include <darmok/physics3d_debug.hpp>

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

    LuaPhysicsDebugRenderer LuaPhysicsDebugRenderer::addCameraComponent1(LuaCamera& cam, LuaPhysicsSystem& system) noexcept
    {
        return LuaPhysicsDebugRenderer(cam.getReal().addComponent<PhysicsDebugRenderer>(system.getReal()));
    }

    LuaPhysicsDebugRenderer LuaPhysicsDebugRenderer::addCameraComponent2(LuaCamera& cam, LuaPhysicsSystem& system, const std::shared_ptr<Program>& prog) noexcept
    {
        return LuaPhysicsDebugRenderer(cam.getReal().addComponent<PhysicsDebugRenderer>(system.getReal(), prog));
    }

    void LuaPhysicsDebugRenderer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaPhysicsDebugRenderer>("PhysicsDebugRenderer",
            sol::no_constructor,
            "enabled", sol::property(&LuaPhysicsDebugRenderer::getEnabled, &LuaPhysicsDebugRenderer::setEnabled),
            "add_camera_component", sol::overload(
                &LuaPhysicsDebugRenderer::addCameraComponent1,
                &LuaPhysicsDebugRenderer::addCameraComponent2
            )
        );
    }

}