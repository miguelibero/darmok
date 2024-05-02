#pragma once

#include <string>

namespace CEGUI
{
    class String;
}

namespace darmok
{
    struct CeguiUtils final
    {
        static std::string convert(const CEGUI::String& str) noexcept;
    };
}