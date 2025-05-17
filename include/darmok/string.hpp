#pragma once

#include <darmok/export.h>

#include <string>
#include <optional>
#include <vector>
#include <regex>
#include <sstream>

#include <magic_enum/magic_enum.hpp>

namespace darmok
{
    namespace StringUtils
    {
        [[nodiscard]] std::string toLower(std::string_view sv) noexcept;
        [[nodiscard]] std::string toUpper(std::string_view sv) noexcept;
        [[nodiscard]] bool startsWith(std::string_view sv, std::string_view start) noexcept;
        [[nodiscard]] bool endsWith(std::string_view sv, std::string_view end) noexcept;
        [[nodiscard]] bool contains(std::string_view sv, std::string_view part) noexcept;
        [[nodiscard]] bool contains(std::string_view sv, std::string_view::value_type part) noexcept;
        size_t replace(std::string& str, std::string_view src, std::string_view dst) noexcept;

        [[nodiscard]] std::vector<std::string> splitWords(std::string_view sv) noexcept;
        [[nodiscard]] std::vector<std::string> split(std::string_view sv, char sep) noexcept;
        [[nodiscard]] std::vector<std::string> split(std::string_view sv, std::string_view sep) noexcept;

        template<typename Iter, typename Callback>
        [[nodiscard]] std::string join(std::string_view sep, Iter begin, Iter end, Callback&& callback) noexcept
        {
            std::ostringstream out;
            auto itr = begin;
			while (itr != end)
			{
				if (itr != begin)
				{
					out << sep;
				}
				out << callback(*itr);
				++itr;
			}
            return out.str();
        }

        template<typename Iter>
        [[nodiscard]] std::string join(std::string_view sep, Iter begin, Iter end) noexcept
        {
			return join(sep, begin, end, [](auto& elm) { return elm; });
        }

        template<typename Container>
        [[nodiscard]] std::string join(std::string_view sep, const Container& elements) noexcept
        {
            return join(sep, elements.begin(), elements.end());
        }

        [[nodiscard]] bool containsGlobPattern(std::string_view glob) noexcept;
        [[nodiscard]] std::string globToRegex(std::string_view glob) noexcept;
        
        std::string_view trimLeft(std::string_view str) noexcept;
        std::string_view trimRight(std::string_view str) noexcept;
        std::string_view trim(std::string_view str) noexcept;

        template<typename Callback>
        bool regexReplace(std::string& str, const std::regex& pattern, Callback&& callback)
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

        template<typename T>
        std::string_view getEnumName(T val) noexcept
        {
			return magic_enum::enum_name(val);
        }

        template<typename T>
        std::optional<T> readEnum(std::string_view name, std::string_view prefix = {}) noexcept
        {
            if (startsWith(name, prefix))
            {
                name = name.substr(prefix.size());
            }
            return magic_enum::enum_cast<T>(name);
        }        
    };
}