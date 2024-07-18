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
        static std::reference_wrapper<ForwardRenderer> addRenderer(LuaCamera& cam) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<ForwardRenderer> _renderer;
    };
}