#pragma once

#include <sol/sol.hpp>
#include <memory>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Camera;
    class Program;
}

namespace darmok::physics3d
{
    class PhysicsDebugRenderer;
    class LuaPhysicsSystem;
    struct PhysicsDebugConfig;

    class LuaPhysicsDebugRenderer final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        using Config = PhysicsDebugConfig;

        static PhysicsDebugRenderer& addRenderer1(Camera& cam, LuaPhysicsSystem& system) noexcept;
        static PhysicsDebugRenderer& addRenderer2(Camera& cam, LuaPhysicsSystem& system, const Config& config) noexcept;
    };
}