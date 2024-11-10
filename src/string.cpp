#include <darmok/string.hpp>
#include <string>
#include <algorithm>
#include <sstream>

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

	std::string StringUtils::toUpper(std::string_view sv) noexcept
	{
		std::string s(sv);
		std::transform(s.begin(), s.end(), s.begin(),
			[](unsigned char c) { return std::toupper(c); }
		);
		return s;
	}

	std::string StringUtils::toString(Entity entity) noexcept
	{
		return std::to_string(uint32_t(entity));
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

	std::vector<std::string> StringUtils::split(std::string_view sv, char sep) noexcept
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

	std::vector<std::string> StringUtils::split(std::string_view sv, std::string_view sep) noexcept
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

	std::string StringUtils::doJoin(std::string_view sep, const std::vector<std::string>& strs) noexcept
	{
		std::ostringstream ss;
		for (size_t i = 0; i < strs.size(); i++)
		{
			if (i > 0)
			{
				ss << sep;
			}
			ss << strs[i];
		}
		return ss.str();
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

	std::string StringUtils::getFileStem(std::string_view filename, bool lower) noexcept
	{
		auto filenameStr = lower ? toLower(filename) : std::string(filename);
		auto pos = filenameStr.find('.');
		if (pos == std::string::npos)
		{
			return filenameStr;
		}
		return filenameStr.substr(0, pos);
	}

	std::string StringUtils::getFileExt(std::string_view filename, bool lower) noexcept
	{
		auto filenameStr = lower ? toLower(filename) : std::string(filename);
		auto pos = filename.find('.');
		if (pos == std::string::npos)
		{
			return "";
		}
		return filenameStr.substr(pos, filename.size());
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

	std::string StringUtils::vsprintf(const std::string& fmt, va_list args)
	{
		std::string result;
#ifdef _MSC_VER
		auto len = _vscprintf(fmt.c_str(), args);
		result.resize(len);
		vsprintf_s(result.data(), len + 1, fmt.c_str(), args);
#else
		auto len = vsnprintf(nullptr, 0, fmt.c_str(), args);
		result.resize(len);
		vsnprintf(result.data(), len + 1, fmt.c_str(), args);
#endif
		return result;
	}

	std::string StringUtils::sprintf(const std::string& fmt, ...)
	{
		va_list args;
		va_start(args, &fmt);
		auto str = vsprintf(fmt, args);
		va_end(args);
		return str;
	}

	std::string StringUtils::getTimeSuffix() noexcept
	{
		time_t val;
		time(&val);
		return std::to_string(val);
	}

	std::string StringUtils::escapeArgument(std::string_view arg) noexcept
    {
        std::ostringstream oss;

        bool needsQuotes = false;
        for (char c : arg)
        {
            if (c == ' ' || c == '\t' || c == '"' || c == '\\')
            {
                needsQuotes = true;
                break;
            }
        }

        if (needsQuotes)
        {
            oss << '"';
            for (char c : arg)
            {
                if (c == '"' || c == '\\')
                {
                    oss << '\\';
                }
                oss << c;
            }
            oss << '"';
        }
        else
        {
            oss << arg;
        }

        return oss.str();
    }

	bool StringUtils::containsGlobPattern(std::string_view sv) noexcept
	{
		return sv.contains('*') || sv.contains('?');
	}

	std::string StringUtils::globToRegex(std::string_view glob) noexcept
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

	size_t StringUtils::replace(std::string& str, std::string_view src, std::string_view dst) noexcept
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

	std::optional<std::string> StringUtils::getEnv(const std::string& name) noexcept
	{
		char* val = getenv(name.c_str());
		if (val == NULL)
		{
			return std::nullopt;
		}
		return std::string(val);
	}

	std::u8string StringUtils::utf8Cast(const std::string& str) noexcept
	{
		return std::u8string(reinterpret_cast<const char8_t*>(str.data()), str.size());
	}
}