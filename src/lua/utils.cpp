#include "utils.hpp"
#include <sstream>
#include <iostream>
#include <bx/debug.h>
#include <bx/string.h>

namespace darmok
{
    void recoveredLuaError(const std::string& desc, const sol::error& err) noexcept
	{
		std::stringstream ss;
		ss << "recovered error " << desc << ":" << std::endl;
		ss << err.what() << std::endl;

		auto str = ss.str();
		std::cerr << str;
		bx::debugOutput(bx::StringView(str.data(), str.size()));
	}
}