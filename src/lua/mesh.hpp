#pragma once

#include "lua/lua.hpp"

namespace darmok
{
	class LuaMesh final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	};

}