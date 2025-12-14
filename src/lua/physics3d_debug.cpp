
#ifdef _DEBUG

#include "lua/physics3d_debug.hpp"
#include "lua/physics3d.hpp"
#include "lua/protobuf.hpp"
#include <darmok/physics3d_debug.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>

namespace darmok::physics3d
{
    std::reference_wrapper<PhysicsDebugRenderer> LuaPhysicsDebugRenderer::addCameraComponent1(Camera& cam)
    {
        return LuaUtils::unwrapExpected(cam.addComponent<PhysicsDebugRenderer>());
    }

    std::reference_wrapper<PhysicsDebugRenderer> LuaPhysicsDebugRenderer::addCameraComponent2(Camera& cam, const Definition& def)
    {
        return LuaUtils::unwrapExpected(cam.addComponent<PhysicsDebugRenderer>(def));
    }

    OptionalRef<PhysicsDebugRenderer>::std_t LuaPhysicsDebugRenderer::getCameraComponent(Camera& cam) noexcept
    {
        return cam.getComponent<PhysicsDebugRenderer>();
    }

    void LuaPhysicsDebugRenderer::bind(sol::state_view& lua) noexcept
    {
		LuaProtobufBinding<PhysicsDebugRenderer::Definition>(lua, "PhysicsDebugRendererDefinition")
            .protobufProperty<InputEvent>("enable_events");

        lua.new_usertype<PhysicsDebugRenderer>("PhysicsDebugRenderer",
            sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<PhysicsDebugRenderer>::value),
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