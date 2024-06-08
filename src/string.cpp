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
}