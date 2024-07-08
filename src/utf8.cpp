#include <darmok/utf8.hpp>

namespace darmok
{
    Utf8Char::Utf8Char(uint32_t code) noexcept
		: code(code)
	{
	}

	Utf8Char::Utf8Char(std::string_view str)
		: code(readCode(str))
	{
	}

	Utf8Char::Utf8Char(std::u8string_view str)
		: code(readCode(str))
	{
	}

	uint8_t Utf8Char::length() const noexcept
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

	Utf8Char::operator std::string() const
	{
		return toString();
	}

	Utf8Char::operator std::u8string() const
	{
		return toUtf8String();
	}

	std::string Utf8Char::toString(const Utf8Vector& chars)
	{
		return vectorToString<char>(chars);
	}

	std::u8string Utf8Char::toUtf8String(const Utf8Vector& chars)
	{
		return vectorToString<char8_t>(chars);
	}

	std::string Utf8Char::toString() const
	{
		std::string str;
		appendToString(str);
		return str;
	}

	std::u8string Utf8Char::toUtf8String() const
	{
		std::u8string str;
		appendToString(str);
		return str;
	}

	const uint32_t Utf8Char::invalidCode = 0x110000;

	bool Utf8Char::valid() const noexcept
	{
		return code < invalidCode;
	}

	bool Utf8Char::empty() const noexcept
	{
		return !valid();
	}

	Utf8Char::operator bool() const noexcept
	{
		return valid();
	}

	bool Utf8Char::operator==(const Utf8Char& other) const noexcept
	{
		return code == other.code;
	}

	bool Utf8Char::operator!=(const Utf8Char& other) const noexcept
	{
		return !operator==(other);
	}

	bool Utf8Char::operator<(const Utf8Char& other) const noexcept
	{
		return code < other.code;
	}

	Utf8Char Utf8Char::read(std::string_view& str)
	{
		return readCode(str);
	}

	size_t Utf8Char::read(std::string_view str, Utf8Vector& chars)
	{
		return doRead(str, chars);
	}

	Utf8Char Utf8Char::read(std::u8string_view& str)
	{
		return readCode(str);
	}

	size_t Utf8Char::read(std::u8string_view str, Utf8Vector& chars)
	{
		return doRead(str, chars);
	}
}