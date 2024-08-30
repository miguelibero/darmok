#pragma once

#ifdef _DEBUG

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
    class PhysicsDebugCameraComponent;
    struct PhysicsDebugConfig;

    class LuaPhysicsDebugCameraComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        using Config = PhysicsDebugConfig;

        static PhysicsDebugCameraComponent& addCameraComponent1(Camera& cam) noexcept;
        static PhysicsDebugCameraComponent& addCameraComponent2(Camera& cam, const Config& config) noexcept;
    };
}

#endif