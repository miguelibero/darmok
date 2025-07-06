#include <darmok/string.hpp>
#include <string>
#include <algorithm>
#include <sstream>
#include <charconv>
#include <utf8cpp/utf8.h>

namespace darmok::StringUtils
{
	std::string toLower(std::string_view sv) noexcept
	{
		std::string s{ sv };
		std::transform(s.begin(), s.end(), s.begin(),
			[](auto c) { return std::tolower(c); }
		);
		return s;
	}

	std::string toUpper(std::string_view sv) noexcept
	{
		std::string s{ sv };
		std::transform(s.begin(), s.end(), s.begin(),
			[](auto c) { return std::toupper(c); }
		);
		return s;
	}

	bool startsWith(std::string_view sv, std::string_view start) noexcept
	{
		return sv.find(start) == 0;
	}

	bool endsWith(std::string_view sv, std::string_view end) noexcept
	{
		return sv.rfind(end) == sv.size() - end.size();
	}

	bool contains(std::string_view sv, std::string_view part) noexcept
	{
		return sv.find(part) != std::string::npos;
	}

	bool contains(std::string_view sv, std::string_view::value_type part) noexcept
	{
		return sv.find(part) != std::string::npos;
	}

	std::vector<std::string> splitWords(std::string_view sv) noexcept
	{
		std::vector<std::string> words;
		std::stringstream ss;
		ss << sv;
		std::string word;
		while (ss >> word) {
			words.push_back(word);
		}
		return words;
	}

	std::vector<std::string> split(std::string_view sv, char sep) noexcept
	{
		std::vector<std::string> parts;
		size_t start = 0, end = 0;
		while ((end = sv.find(sep, start)) != std::string::npos)
		{
			parts.emplace_back(sv.substr(start, end - start));
			start = end + 1;
		}
		if (start < sv.size() - 1)
		{
			parts.emplace_back(sv.substr(start));
		}
		return parts;
	}

	std::vector<std::string> split(std::string_view sv, std::string_view sep) noexcept
	{
		std::vector<std::string> parts;
		size_t start = 0, end = 0;
		while ((end = sv.find(sep, start)) != std::string::npos)
		{
			parts.emplace_back(sv.substr(start, end - start));
			start = end + sep.length();
		}
		if (start < sv.size() - sep.length())
		{
			parts.emplace_back(sv.substr(start));
		}
		return parts;
	}

	std::string_view trimLeft(std::string_view str) noexcept
	{
		auto itr = std::find_if(str.begin(), str.end(), [](auto ch) {
			return !std::isspace(ch);
			});
		return { itr, str.end() };
	}

	std::string_view trimRight(std::string_view str) noexcept
	{
		auto itr = std::find_if(str.rbegin(), str.rend(), [](auto ch) {
			return !std::isspace(ch);
			});
		return { str.begin(), itr.base() };
	}

	std::string_view trim(std::string& str) noexcept
	{
		return trimRight(trimLeft(str));
	}

	bool containsGlobPattern(std::string_view sv) noexcept
	{
		return contains(sv, '*') || contains(sv, '?');
	}

	std::string globToRegex(std::string_view glob) noexcept
	{
		std::ostringstream regex("^");
		for (char c : glob) {
			switch (c) {
			case '*': regex << ".*"; break;
			case '?': regex << '.'; break;
			case '.': regex << "\\."; break;
			default: regex << c; break;
			}
		}
		regex << '$';
		return regex.str();
	}

	size_t replace(std::string& str, std::string_view src, std::string_view dst) noexcept
	{
		if (src.empty())
		{
			return 0;
		}
		size_t startPos = 0;
		size_t count = 0;
		while ((startPos = str.find(src, startPos)) != std::string::npos)
		{
			str.replace(startPos, src.length(), dst);
			startPos += dst.length();
			++count;
		}
		return count;
	}

	std::u32string toUtf32(std::string_view str) noexcept
	{
		std::u32string result;
		utf8::utf8to32(str.begin(), str.end(), std::back_inserter(result));
		return result;
	}

	std::string toUtf8(std::u32string_view str) noexcept
	{
		std::string result;
		utf8::utf32to8(str.begin(), str.end(), std::back_inserter(result));
		return result;
	}

	std::string toUtf8(char32_t chr) noexcept
	{
		std::string str;
		utf8::utf32to8(&chr, &chr + 1, std::back_inserter(str));
		return str;
	}

	char32_t toUtf32Char(std::string_view str) noexcept
	{
		if (str.empty())
		{
			return 0;
		}
		auto itr = str.begin();
		return utf8::next(itr, str.end());
	}

	void camelCaseToHumanReadable(std::string& str) noexcept
	{
		std::string result;
		result.reserve(str.size() + 5);
		for (size_t i = 0; i < str.size(); ++i)
		{
			auto chr = str[i];
			if (i > 0 && std::isupper(chr) && std::islower(str[i - 1]))
			{
				result += ' ';
			}
			result += std::tolower(chr);
		}
		str = std::move(result);
	}
}