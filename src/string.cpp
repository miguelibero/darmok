#include <darmok/string.hpp>
#include <string>
#include <algorithm>
#include <sstream>
#include <charconv>

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
}