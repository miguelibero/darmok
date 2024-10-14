#pragma once

#include "lua.hpp"

namespace darmok
{
    class Font;
    class Text;

    class LuaText final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    };
}