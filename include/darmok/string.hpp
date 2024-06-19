#pragma once

#include <darmok/export.h>
#include <string>
#include <optional>
#include <vector>

namespace darmok
{
    struct DARMOK_EXPORT StringUtils final
    {
        static [[nodiscard]] std::string toLower(std::string_view sv) noexcept;
        static [[nodiscard]] bool startsWith(std::string_view sv, std::string_view start) noexcept;
        static [[nodiscard]] bool endsWith(std::string_view sv, std::string_view end) noexcept;
        static [[nodiscard]] std::optional<int> getIntSuffix(std::string_view name, std::string_view prefix) noexcept;
        static [[nodiscard]] std::string binToHex(uint8_t v) noexcept;
        static [[nodiscard]] std::vector<std::string> splitWords(std::string_view sv) noexcept;
        static [[nodiscard]] std::vector<std::string> split(std::string_view sv, std::string_view sep) noexcept;
        static [[nodiscard]] uint8_t hexToBin(char chr);
        static [[nodiscard]] uint8_t hexToBin(std::string_view sv);
        static [[nodiscard]] std::string getFileStem(const std::string& filename) noexcept;
        static [[nodiscard]] std::string getFileExt(const std::string& filename) noexcept;
        static void ltrim(std::string& str) noexcept;
        static void rtrim(std::string& str) noexcept;
        static void trim(std::string& str) noexcept;
    };
}