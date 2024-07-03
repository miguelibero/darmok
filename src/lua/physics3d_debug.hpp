#pragma once

#include <sol/sol.hpp>
#include <memory>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class LuaCamera;
    class Program;
}

namespace darmok::physics3d
{
    class PhysicsDebugRenderer;
    class LuaPhysicsSystem;

    class LuaPhysicsDebugRenderer final
    {
    public:
        LuaPhysicsDebugRenderer(PhysicsDebugRenderer& renderer) noexcept;
        bool getEnabled() const noexcept;
        void setEnabled(bool enabled) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<PhysicsDebugRenderer> _renderer;

        static LuaPhysicsDebugRenderer addCameraComponent1(LuaCamera& cam, LuaPhysicsSystem& system) noexcept;
        static LuaPhysicsDebugRenderer addCameraComponent2(LuaCamera& cam, LuaPhysicsSystem& system, const std::shared_ptr<Program>& prog) noexcept;
    };
}