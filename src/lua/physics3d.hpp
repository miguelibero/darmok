#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok::physics3d
{
    class PhysicsSystem;
    class RigidBody;

    class LuaPhysicsSystem final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:

    };

    class LuaRigidBody final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    };
}