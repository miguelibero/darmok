#pragma once

#include <sol/sol.hpp>

namespace darmok
{
	struct LuaMath final
	{
		static void bind(sol::state_view& lua) noexcept;

	private:
		
		static void bindGlmMat(sol::state_view& lua) noexcept;
		static void bindGlmVec(sol::state_view& lua) noexcept;
		static void bindGlmUvec(sol::state_view& lua) noexcept;
		static void bindGlmIvec(sol::state_view& lua) noexcept;
		static void bindGlmQuat(sol::state_view& lua) noexcept;
		static void bindColor(sol::state_view& lua) noexcept;
	};
}