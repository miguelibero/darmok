#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Font;
    class Text;

    class LuaText final
    {
    public:
        LuaText(Text& text) noexcept;
    private:
        OptionalRef<Text> _text;
    };
}