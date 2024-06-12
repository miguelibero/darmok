#pragma once

#include <sol/sol.hpp>

namespace darmok
{
	void recoveredLuaError(const std::string& desc, const sol::error& err) noexcept;
}