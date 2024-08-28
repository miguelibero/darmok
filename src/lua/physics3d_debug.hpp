#pragma once

#ifdef _DEBUG

#include <sol/sol.hpp>
#include <memory>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class ForwardRenderer;
    class Program;
}

namespace darmok::physics3d
{
    class PhysicsDebugRenderComponent;
    struct PhysicsDebugConfig;

    class LuaPhysicsDebugRenderComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        using Config = PhysicsDebugConfig;

        static PhysicsDebugRenderComponent& addRenderComponent1(ForwardRenderer& renderer) noexcept;
        static PhysicsDebugRenderComponent& addRenderComponent2(ForwardRenderer& renderer, const Config& config) noexcept;
    };
}

#endif