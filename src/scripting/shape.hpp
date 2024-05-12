#pragma once

#include <sol/sol.hpp>

namespace darmok
{
    struct LuaShape final
    {
        static void configure(sol::state_view& lua) noexcept;
    };
}