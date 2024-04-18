#include <darmok/utils.hpp>
#include <string>
#include <stdexcept>
#include <algorithm>

namespace darmok
{
	std::string strToLower(std::string_view sv) noexcept
	{
		std::string s(sv);
		std::transform(s.begin(), s.end(), s.begin(),
			[](unsigned char c) { return std::tolower(c); }
		);
		return s;
	}

	void checkError(bx::Error& err)
    {
        if (!err.isOk())
        {
            auto msg = err.getMessage();
            std::string strMsg(msg.getPtr(), msg.getLength());
            strMsg += "( " + err.get().code + std::string(")");
            throw std::runtime_error(strMsg.c_str());
        }
    }

	Utf8Char::Utf8Char(uint32_t pdata, uint8_t plen) noexcept
		: data(pdata)
		, len(pdata)
	{
	}

	std::string_view Utf8Char::string() const noexcept
	{
		return std::string_view((const char*)& data, len);
	}

	// Based on cutef8 by Jeff Bezanson (Public Domain)
	Utf8Char Utf8Char::encode(uint32_t scancode) noexcept
	{
		Utf8Char utf8 = { 0, 0 };

		if (scancode < 0x80)
		{
			utf8.data = scancode;
			utf8.len = 1;
		}
		else if (scancode < 0x800)
		{
			utf8.data = (scancode >> 6) | 0xc0;
			utf8.data += ((scancode & 0x3f) | 0x80) >> 8;
			utf8.len = 2;
		}
		else if (scancode < 0x10000)
		{
			utf8.data = (scancode >> 12) | 0xe0;
			utf8.data += (((scancode >> 6) & 0x3f) | 0x80) >> 8;
			utf8.data += ((scancode & 0x3f) | 0x80) >> 16;
			utf8.len = 3;
		}
		else if (scancode < 0x110000)
		{
			utf8.data = (scancode >> 18) | 0xf0;
			utf8.data += (((scancode >> 12) & 0x3f) | 0x80) >> 8;
			utf8.data += (((scancode >> 6) & 0x3f) | 0x80) >> 16;
			utf8.data += ((scancode & 0x3f) | 0x80) >> 24;
			utf8.len = 4;
		}

		return utf8;
	}
}