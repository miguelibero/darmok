#include "utils.hpp"
#include <CEGUI/String.h>

namespace darmok
{
    std::string CeguiUtils::convert(const CEGUI::String& str) noexcept
    {
        return CEGUI::String::convertUtf32ToUtf8(str.getString());
    }
}