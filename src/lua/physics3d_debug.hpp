#pragma once

#ifdef _DEBUG

#include "lua/lua.hpp"
#include <darmok/optional_ref.hpp>
#include <memory>

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

        static std::reference_wrapper<PhysicsDebugRenderer> addCameraComponent1(Camera& cam) noexcept;
        static std::reference_wrapper<PhysicsDebugRenderer> addCameraComponent2(Camera& cam, const Config& config) noexcept;
        static OptionalRef<PhysicsDebugRenderer>::std_t getCameraComponent(Camera& cam) noexcept;
    };
}

#endif