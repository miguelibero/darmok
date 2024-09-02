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
    class PhysicsDebugRenderer;
    struct PhysicsDebugConfig;

    class LuaPhysicsDebugRenderer final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        using Config = PhysicsDebugConfig;

        static PhysicsDebugRenderer& addCameraComponent1(Camera& cam) noexcept;
        static PhysicsDebugRenderer& addCameraComponent2(Camera& cam, const Config& config) noexcept;
        static OptionalRef<PhysicsDebugRenderer>::std_t getCameraComponent(Camera& cam) noexcept;
    };
}

#endif