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

    namespace protobuf
    {
        class PhysicsDebugRenderer;
    }

    class LuaPhysicsDebugRenderer final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        using Definition = protobuf::PhysicsDebugRenderer;

        static std::reference_wrapper<PhysicsDebugRenderer> addCameraComponent1(Camera& cam);
        static std::reference_wrapper<PhysicsDebugRenderer> addCameraComponent2(Camera& cam, const Definition& def);
        static OptionalRef<PhysicsDebugRenderer>::std_t getCameraComponent(Camera& cam) noexcept;
    };
}

#endif