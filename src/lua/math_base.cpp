#include "math.hpp"
#include "glm.hpp"
#include <darmok/math.hpp>

namespace darmok
{
	void LuaMath::bind(sol::state_view& lua) noexcept
	{
		auto degrees = sol::resolve<float(float)>(&glm::degrees);
		auto radians = sol::resolve<float(float)>(&glm::radians);
		lua.create_named_table("Math",
			"clamp", sol::overload(
				sol::resolve<float(float, float, float)>(&glm::clamp<float>),
				sol::resolve<int(int, int, int)>(&glm::clamp<int>)
			),
			"lerp", sol::overload(&Math::lerp<float>, &Math::lerp<int>),
			"abs", sol::overload(
				sol::resolve<float(float)>(&glm::abs),
				sol::resolve<int(int)>(&glm::abs)
			),
			"pi", glm::pi<float>(),
			"half_pi", glm::half_pi<float>(),
			"quarter_pi", glm::quarter_pi<float>(),
			"two_pi", glm::two_pi<float>(),
			"two_over_pi", glm::two_over_pi<float>(),
			"two_over_root_pi", glm::two_over_root_pi<float>(),
			"degrees", degrees,
			"radians", radians,
			"rad2deg", degrees,
			"deg2rad", radians,
			"ortho", sol::overload(
				sol::resolve<glm::mat4(float, float, float, float, float, float)>(&Math::ortho),
				[](const VarLuaTable<glm::vec2>& bottomLeft, const VarLuaTable<glm::vec2>& rightTop, float near, float far)
				{
					return Math::ortho(LuaGlm::tableGet(bottomLeft), LuaGlm::tableGet(rightTop), near, far);
				}
			) 
		);

		bindGlmMat(lua);
		bindGlmVec(lua);
		bindGlmUvec(lua);
		bindGlmIvec(lua);
		bindGlmQuat(lua);
		bindColor(lua);
    }
}