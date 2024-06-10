#pragma once

#include <string>
#include <optional>
#include <vector>

namespace darmok
{
    struct StringUtils final
    {
        static std::string toLower(std::string_view sv) noexcept;
        static bool startsWith(std::string_view sv, std::string_view start) noexcept;
        static bool endsWith(std::string_view sv, std::string_view end) noexcept;
        static std::optional<int> getIntSuffix(std::string_view name, std::string_view prefix) noexcept;
        static std::string binToHex(uint8_t v) noexcept;
        static std::vector<std::string> splitWords(std::string_view sv) noexcept;
        static std::vector<std::string> split(std::string_view sv, std::string_view sep) noexcept;
        static std::string getPathExtension(std::string_view path) noexcept;
        static uint8_t hexToBin(char chr);
        static uint8_t hexToBin(std::string_view sv);
        static void ltrim(std::string& str) noexcept;
        static void rtrim(std::string& str) noexcept;
        static void trim(std::string& str) noexcept;
    };
}