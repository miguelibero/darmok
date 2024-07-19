#pragma once

#include <darmok/optional_ref.hpp>
#include <sol/sol.hpp>

namespace darmok
{
    class ForwardRenderer;
    class Camera;

    class LuaForwardRenderer final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:

        static ForwardRenderer& addRenderer(Camera& cam) noexcept;
    };
}