#pragma once

#include <sol/sol.hpp>

namespace darmok
{
	struct LuaMath final
	{
		static void bind(sol::state_view& lua) noexcept;

		template<typename T>
		static T lerp(const T& a, const T& b, float p) noexcept
		{
			return a + (b * p);
		}

	private:
		
		static void bindGlmMat(sol::state_view& lua) noexcept;
		static void bindGlmVec(sol::state_view& lua) noexcept;
		static void bindGlmUvec(sol::state_view& lua) noexcept;
		static void bindGlmIvec(sol::state_view& lua) noexcept;
		static void bindGlmQuat(sol::state_view& lua) noexcept;
		static void bindColor(sol::state_view& lua) noexcept;
	};
}