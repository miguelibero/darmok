#pragma once

#include <sol/sol.hpp>

namespace darmok
{
	class LuaMesh final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	};

}