#pragma once

#include <darmok/export.h>
#include <darmok/scene_fwd.hpp>
#include <string>
#include <optional>
#include <vector>
#include <cstdarg>
#include <cstdint>
#include <regex>

namespace darmok
{
    struct DARMOK_EXPORT StringUtils final
    {
        [[nodiscard]] static std::string toLower(std::string_view sv) noexcept;
        [[nodiscard]] static std::string toUpper(std::string_view sv) noexcept;
        [[nodiscard]] static bool startsWith(std::string_view sv, std::string_view start) noexcept;
        [[nodiscard]] static bool endsWith(std::string_view sv, std::string_view end) noexcept;
        [[nodiscard]] static std::optional<int> getIntSuffix(std::string_view name, std::string_view prefix) noexcept;

        template<typename T>
        static std::string binToHex(T&& v) noexcept
        {
            return StringUtils::sprintf("%02X", v);
        }

        template<const void*>
        static std::string binToHex(const void* v) noexcept
        {
            return StringUtils::sprintf("%p", v);
        }

        [[nodiscard]] static std::vector<std::string> splitWords(std::string_view sv) noexcept;
        [[nodiscard]] static std::vector<std::string> split(std::string_view sv, char sep) noexcept;
        [[nodiscard]] static std::vector<std::string> split(std::string_view sv, std::string_view sep) noexcept;

        template<typename Iter, typename C>
        [[nodiscard]] static std::string join(std::string_view sep, Iter begin, Iter end, C callback) noexcept
        {
            std::vector<std::string> strs;
            for (auto itr = begin; itr != end; ++ itr)
            {
                strs.emplace_back(callback(*itr));
            }
            return doJoin(sep, strs);
        }

        template<typename T, typename C>
        [[nodiscard]] static std::string join(std::string_view sep, const std::vector<T>& elements, C callback) noexcept
        {
            return join(sep, elements.begin(), elements.end(), callback);
        }

        template<typename T>
        [[nodiscard]] static std::string join(std::string_view sep, const std::vector<T>& elements) noexcept
        {
            return join(sep, elements.begin(), elements.end());
        }

        template<typename Iter>
        [[nodiscard]] static std::string join(std::string_view sep, Iter begin, Iter end) noexcept
        {
            return joinImpl<Iter, typename std::iterator_traits<Iter>::value_type>::run(sep, begin, end);
        }

        [[nodiscard]] static uint8_t hexToBin(char chr);
        [[nodiscard]] static uint8_t hexToBin(std::string_view sv);
        [[nodiscard]] static std::string getFileStem(std::string_view filename, bool lower = true) noexcept;
        [[nodiscard]] static std::string getFileExt(std::string_view filename, bool lower = true) noexcept;
        [[nodiscard]] static std::string escapeArgument(std::string_view arg) noexcept;
        [[nodiscard]] static bool containsGlobPattern(std::string_view glob) noexcept;
        [[nodiscard]] static std::string globToRegex(std::string_view glob) noexcept;
        static size_t replace(std::string& str, std::string_view src, std::string_view dst) noexcept;
        [[nodiscard]] static std::optional<std::string> getEnv(const std::string& name) noexcept;
        [[nodiscard]] static std::u8string utf8Cast(const std::string& str) noexcept;

        static void ltrim(std::string& str) noexcept;
        static void rtrim(std::string& str) noexcept;
        static void trim(std::string& str) noexcept;

        static std::string vsprintf(const std::string& fmt, va_list args);
        static std::string sprintf(const std::string& fmt, ...);

        static std::string getTimeSuffix() noexcept;

        template<typename Callback>
        static bool regexReplace(std::string& str, const std::regex& pattern, Callback&& callback)
        {
            auto strItr = str.cbegin();
            auto changed = false;
            std::smatch match;
            while (std::regex_search(strItr, str.cend(), match, pattern))
            {
                std::string repl;
                if (callback(match, repl))
                {
                    auto pos = match.position(0);
                    str.replace(pos, match.length(0), repl);
                    strItr = str.cbegin() + pos + repl.length();
                    changed = true;
                }
            }
            return changed;
        }

        static std::string toString(Entity entity) noexcept;

    private:
        [[nodiscard]] static std::string doJoin(std::string_view sep, const std::vector<std::string>& strs) noexcept;

        template<typename Iter, typename V>
        struct joinImpl final
        {
            static std::string run(std::string_view sep, Iter begin, Iter end)
            {
                return join(sep, begin, end, [](auto& elm) { return std::to_string(elm);  });
            }
        };

        template<typename Iter>
        struct joinImpl<Iter, std::string> final
        {
            static std::string run(std::string_view sep, Iter begin, Iter end)
            {
                return join(sep, begin, end, [](auto& elm) { return elm;  });
            }
        };

        template<typename Iter>
        struct joinImpl<Iter, std::string_view> final
        {
            static std::string run(std::string_view sep, Iter begin, Iter end)
            {
                return join(sep, begin, end, [](auto& elm) { return elm;  });
            }
        };
    };
}