#pragma once

#include <string>
#include <optional>
#include <vector>

namespace darmok
{
    struct StringUtils final
    {
        DLLEXPORT static [[nodiscard]] std::string toLower(std::string_view sv) noexcept;
        DLLEXPORT static [[nodiscard]] bool startsWith(std::string_view sv, std::string_view start) noexcept;
        DLLEXPORT static [[nodiscard]] bool endsWith(std::string_view sv, std::string_view end) noexcept;
        DLLEXPORT static [[nodiscard]] std::optional<int> getIntSuffix(std::string_view name, std::string_view prefix) noexcept;
        DLLEXPORT static [[nodiscard]] std::string binToHex(uint8_t v) noexcept;
        DLLEXPORT static [[nodiscard]] std::vector<std::string> splitWords(std::string_view sv) noexcept;
        DLLEXPORT static [[nodiscard]] std::vector<std::string> split(std::string_view sv, std::string_view sep) noexcept;
        DLLEXPORT static [[nodiscard]] std::string getPathExtension(std::string_view path) noexcept;
        DLLEXPORT static [[nodiscard]] uint8_t hexToBin(char chr);
        DLLEXPORT static [[nodiscard]] uint8_t hexToBin(std::string_view sv);
        DLLEXPORT static void ltrim(std::string& str) noexcept;
        DLLEXPORT static void rtrim(std::string& str) noexcept;
        DLLEXPORT static void trim(std::string& str) noexcept;
    };
}