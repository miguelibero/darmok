#include <darmok/utf.hpp>

namespace darmok
{
    UtfChar::UtfChar(uint32_t code) noexcept
		: code(code)
	{
	}

	UtfChar::UtfChar(std::string_view str)
		: code(readCode(str))
	{
	}

	UtfChar::UtfChar(std::u8string_view str)
		: code(readCode(str))
	{
	}

	uint8_t UtfChar::length() const noexcept
	{
		if (code <= 0x7F)
		{
			return 1;
		}
		else if (code <= 0x7FF)
		{
			return 2;
		}
		else if (code <= 0xFFFF)
		{
			return 3;
		}
		else if (code <= 0x10FFFF)
		{
			return 4;
		}
		return 0;
	}

	UtfChar::operator std::string() const
	{
		return toString();
	}

	UtfChar::operator std::u8string() const
	{
		return toUtf8String();
	}

	std::string UtfChar::toString(const UtfVector& chars)
	{
		return vectorToString<char>(chars);
	}

	std::u8string UtfChar::toUtfString(const UtfVector& chars)
	{
		return vectorToString<char8_t>(chars);
	}

	std::string UtfChar::toString() const
	{
		std::string str;
		appendToString(str);
		return str;
	}

	std::u8string UtfChar::toUtf8String() const
	{
		std::u8string str;
		appendToString(str);
		return str;
	}

	const uint32_t UtfChar::invalidCode = 0x110000;

	bool UtfChar::valid() const noexcept
	{
		return code < invalidCode;
	}

	bool UtfChar::empty() const noexcept
	{
		return !valid();
	}

	UtfChar::operator bool() const noexcept
	{
		return valid();
	}

	bool UtfChar::operator==(const UtfChar& other) const noexcept
	{
		return code == other.code;
	}

	bool UtfChar::operator!=(const UtfChar& other) const noexcept
	{
		return !operator==(other);
	}

	bool UtfChar::operator<(const UtfChar& other) const noexcept
	{
		return code < other.code;
	}

	UtfChar UtfChar::read(std::string_view& str)
	{
		return readCode(str);
	}

	size_t UtfChar::read(std::string_view str, UtfVector& chars)
	{
		return doRead(str, chars);
	}

	UtfChar UtfChar::read(std::u8string_view& str)
	{
		return readCode(str);
	}

	size_t UtfChar::read(std::u8string_view str, UtfVector& chars)
	{
		return doRead(str, chars);
	}
}

namespace std
{
	std::size_t std::hash<darmok::UtfChar>::operator()(const darmok::UtfChar& key) const noexcept
	{
		return key.code;
	}
};