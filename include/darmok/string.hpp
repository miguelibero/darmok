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
        static [[nodiscard]] std::vector<std::string> split(std::string_view sv, char sep) noexcept;
        static [[nodiscard]] std::vector<std::string> split(std::string_view sv, std::string_view sep) noexcept;
        static [[nodiscard]] std::string join(std::vector<std::string> strs, std::string_view sep) noexcept;
        static [[nodiscard]] uint8_t hexToBin(char chr);
        static [[nodiscard]] uint8_t hexToBin(std::string_view sv);
        static [[nodiscard]] std::string getFileStem(std::string_view filename) noexcept;
        static [[nodiscard]] std::string getFileExt(std::string_view filename) noexcept;
        static [[nodiscard]] std::string escapeArgument(std::string_view arg) noexcept;
        static [[nodiscard]] bool containsGlobPattern(std::string_view glob) noexcept;
        static [[nodiscard]] std::string globToRegex(std::string_view glob) noexcept;
        static [[nodiscard]] size_t replace(std::string& str, std::string_view src, std::string_view dst) noexcept;
        static [[nodiscard]] std::optional<std::string> getEnv(const std::string& name) noexcept;

        static void ltrim(std::string& str) noexcept;
        static void rtrim(std::string& str) noexcept;
        static void trim(std::string& str) noexcept;
    };

#pragma region Utf8Char

    struct DARMOK_EXPORT Utf8Char final
    {
        uint32_t data;
        uint8_t len;

        Utf8Char() noexcept;
        Utf8Char(uint32_t data, uint8_t len) noexcept;
        Utf8Char(uint32_t strData) noexcept;
        uint32_t encode() const noexcept;
        [[nodiscard]] static Utf8Char read(std::string_view& str) noexcept;
        [[nodiscard]] static std::vector<Utf8Char> tokenize(std::string_view str) noexcept;
        operator std::string_view() const noexcept;
        operator bool() const noexcept;
        std::string to_string() const noexcept;
        bool operator==(const Utf8Char& other) const noexcept;
        bool operator!=(const Utf8Char& other) const noexcept;
    };

#pragma endregion Utf8Char
}

namespace std
{
    template<typename T>
    struct hash;
}

template<> struct std::hash<darmok::Utf8Char>
{
    std::size_t operator()(const darmok::Utf8Char& key) const noexcept
    {
        std::size_t v = key.encode();
        return v;
    }
};