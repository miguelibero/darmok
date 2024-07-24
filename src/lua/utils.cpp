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

	bool checkLuaResult(const std::string& desc, const sol::protected_function_result& result) noexcept
	{
		if (!result.valid())
		{
			logLuaError(desc, result);
			return false;
		}
		return true;
	}

	int luaDeny(lua_State* L)
	{
		return luaL_error(L, "operation not allowed");
	}
}