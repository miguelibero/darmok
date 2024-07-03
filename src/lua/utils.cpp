#include "utils.hpp"
#include <sstream>
#include <iostream>
#include <bx/debug.h>
#include <bx/string.h>
#include <darmok/stream.hpp>

namespace darmok
{
    void logLuaError(const std::string& desc, const sol::error& err) noexcept
	{
		std::stringstream ss;
		ss << "recovered lua error " << desc << ":" << std::endl;
		ss << err.what() << std::endl;

		StreamUtils::logDebug(ss.str(), true);
	}
}