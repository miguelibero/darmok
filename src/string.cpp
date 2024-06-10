#include <darmok/string.hpp>
#include <string>
#include <algorithm>
#include <sstream>
#include <charconv>
#include <filesystem>

namespace darmok
{
    std::string StringUtils::toLower(std::string_view sv) noexcept
	{
		std::string s(sv);
		std::transform(s.begin(), s.end(), s.begin(),
			[](unsigned char c) { return std::tolower(c); }
		);
		return s;
	}

	bool StringUtils::startsWith(std::string_view sv, std::string_view start) noexcept
	{
		return sv.find(start) == 0;
	}

	bool StringUtils::endsWith(std::string_view sv, std::string_view end) noexcept
	{
		return sv.rfind(end) == sv.size() - end.size();
	}

	std::optional<int> StringUtils::getIntSuffix(std::string_view name, std::string_view prefix) noexcept
	{
		if (!startsWith(name, prefix))
		{
			return std::nullopt;
		}
		int v;
		auto r = std::from_chars(name.data() + prefix.size(), name.data() + name.size(), v);
		if (r.ptr == nullptr)
		{
			return std::nullopt;
		}
		return v;
	}

	std::vector<std::string> StringUtils::splitWords(std::string_view sv) noexcept
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

	std::vector<std::string> StringUtils::split(std::string_view sv, std::string_view sep) noexcept
	{
		std::vector<std::string> parts;
		size_t start = 0;
		size_t end;

		while ((end = sv.find(sep, start)) != std::string::npos)
		{
			parts.emplace_back(sv.substr(start, end - start));
			start = end + sep.length();
		}
		parts.emplace_back(sv.substr(start));
		return parts;
	}

	std::string StringUtils::getPathExtension(std::string_view path) noexcept
	{
		return std::filesystem::path(path).extension().string();
	}

	std::string StringUtils::binToHex(uint8_t v) noexcept
	{
		std::string dest = "  ";
		sprintf(&dest.front(), "%02X", v);
		return dest;
	}

	uint8_t StringUtils::hexToBin(char chr)
	{
		if (chr >= '0' && chr <= '9')
		{
			return chr - '0';
		}
		else if (chr >= 'A' && chr <= 'F')
		{
			return chr - 'A' + 10;
		}
		else if (chr >= 'a' && chr <= 'f')
		{
			return chr - 'a' + 10;
		}
		throw std::invalid_argument("Invalid hex character");
	}

	uint8_t StringUtils::hexToBin(std::string_view sv)
	{
		if (sv.length() != 2)
		{
			throw std::invalid_argument("Hex string must be exactly 2 characters long");
		}

		uint8_t high = hexToBin(sv[0]);
		uint8_t low = hexToBin(sv[1]);
		return (high << 4) | low;
	}

	void StringUtils::ltrim(std::string& str) noexcept
	{
		auto itr = std::find_if(str.begin(), str.end(), [](auto ch) {
			return !std::isspace(ch);
		});
		str.erase(str.begin(), itr);
	}

	void StringUtils::rtrim(std::string& str) noexcept
	{
		auto itr = std::find_if(str.rbegin(), str.rend(), [](auto ch) {
			return !std::isspace(ch);
		});
		str.erase(itr.base(), str.end());
	}

	void StringUtils::trim(std::string& str) noexcept
	{
		ltrim(str);
		rtrim(str);
	}
}