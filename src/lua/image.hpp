#pragma once

#include "lua/lua.hpp"

namespace darmok
{
	class LuaImage final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	};
}