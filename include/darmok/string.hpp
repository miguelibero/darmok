#pragma once

#include <string_view>
#include <optional>

namespace darmok
{
    struct StringUtils final
    {
        static std::string toLower(std::string_view sv) noexcept;
        static bool startsWith(std::string_view sv, std::string_view start) noexcept;
        static bool endsWith(std::string_view sv, std::string_view end) noexcept;
        static std::optional<int> getIntSuffix(std::string_view name, std::string_view prefix) noexcept;
        static std::string binToHex(uint8_t v) noexcept;
        static uint8_t hexToBin(char chr);
        static uint8_t hexToBin(std::string_view sv);
    };
}