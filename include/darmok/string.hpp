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

        template<typename T, typename C>
        static [[nodiscard]] std::string join(std::string_view sep, const std::vector<T>& elements,  C callback) noexcept
        {
            std::vector<std::string> strs;
            for (auto& elm : elements)
            {
                strs.emplace_back(callback(elm));
            }
            return join(sep, strs);
        }

        template<typename T>
        static [[nodiscard]] std::string join(std::string_view sep, const std::vector<T>& elements) noexcept
        {
            return join(sep, elements, [](auto& elm) { return std::to_string(elm);  });
        }

        template<>
        static [[nodiscard]] std::string join<std::string>(std::string_view sep, const std::vector<std::string>& elements) noexcept
        {
            return doJoin(sep, elements);
        }

        static [[nodiscard]] uint8_t hexToBin(char chr);
        static [[nodiscard]] uint8_t hexToBin(std::string_view sv);
        static [[nodiscard]] std::string getFileStem(std::string_view filename, bool lower = true) noexcept;
        static [[nodiscard]] std::string getFileExt(std::string_view filename, bool lower = true) noexcept;
        static [[nodiscard]] std::string escapeArgument(std::string_view arg) noexcept;
        static [[nodiscard]] bool containsGlobPattern(std::string_view glob) noexcept;
        static [[nodiscard]] std::string globToRegex(std::string_view glob) noexcept;
        static [[nodiscard]] size_t replace(std::string& str, std::string_view src, std::string_view dst) noexcept;
        static [[nodiscard]] std::optional<std::string> getEnv(const std::string& name) noexcept;
        static [[nodiscard]] std::u8string utf8Cast(const std::string& str) noexcept;

        static void ltrim(std::string& str) noexcept;
        static void rtrim(std::string& str) noexcept;
        static void trim(std::string& str) noexcept;

    private:
        static [[nodiscard]] std::string doJoin(std::string_view sep, const std::vector<std::string>& strs) noexcept;
    };
}