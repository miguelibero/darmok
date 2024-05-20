#pragma once

#include <sol/sol.hpp>

namespace darmok
{
    struct LuaShape final
    {
        static void bind(sol::state_view& lua) noexcept;
    };
}