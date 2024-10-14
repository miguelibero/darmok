#pragma once

#include "lua.hpp"

namespace darmok
{
    class Program;

    class LuaProgram final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	};
}