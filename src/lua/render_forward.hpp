#pragma once

#include "lua/lua.hpp"
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class ForwardRenderer;
    class Camera;

    class LuaForwardRenderer final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:

        static ForwardRenderer& addCameraComponent(Camera& cam) noexcept;
        static OptionalRef<ForwardRenderer>::std_t getCameraComponent(Camera& cam) noexcept;
    };
}