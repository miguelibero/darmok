#pragma once

#include <darmok/export.h>
#include <sol/sol.hpp>

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