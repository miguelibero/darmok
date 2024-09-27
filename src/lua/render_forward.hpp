#pragma once

#include <darmok/optional_ref.hpp>
#include <sol/sol.hpp>

namespace darmok
{
    class ForwardRenderer;
    class LuaCamera;

    class LuaForwardRenderer final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:

        static ForwardRenderer& addCameraComponent(LuaCamera& cam) noexcept;
        static OptionalRef<ForwardRenderer>::std_t getCameraComponent(LuaCamera& cam) noexcept;
    };
}