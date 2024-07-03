#pragma once

#include <memory>
#include <bgfx/bgfx.h>
#include <sol/sol.hpp>

namespace darmok
{
    class Program;

    class LuaProgram final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	};
}