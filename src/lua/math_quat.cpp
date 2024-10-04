#include "math.hpp"
#include "glm.hpp"
#include <glm/gtx/rotate_normalized_axis.hpp>

namespace darmok
{
	void LuaMath::bindGlmQuat(sol::state_view& lua) noexcept
	{
		lua.new_usertype<glm::quat>("Quat",
			sol::factories(
				[]() { return glm::quat(1, 0, 0, 0); },
				[](float w, float x, float y, float z) { return glm::quat(w, x, y, z); },
				[](const VarLuaTable<glm::vec3>& vec) { return glm::quat(LuaGlm::tableGet(vec)); }
			),
			sol::meta_function::equal_to, sol::resolve<bool(const glm::quat&, const glm::quat&)>(glm::operator==),
			sol::meta_function::addition, sol::resolve<glm::quat(const glm::quat&, const glm::quat&)>(glm::operator+),
			sol::meta_function::subtraction, sol::resolve<glm::quat(const glm::quat&, const glm::quat&)>(glm::operator-),
			sol::meta_function::unary_minus, sol::resolve<glm::quat(const glm::quat&)>(glm::operator-),
			sol::meta_function::multiplication, sol::overload(
				sol::resolve<glm::quat(const glm::quat&, const glm::quat&)>(glm::operator*),
				sol::resolve<glm::quat(const glm::quat&, const float&)>(glm::operator*),
				sol::resolve<glm::vec3(const glm::quat&, const glm::vec3&)>(glm::operator*),
				sol::resolve<glm::vec4(const glm::quat&, const glm::vec4&)>(glm::operator*),
				[](const glm::quat& quat, const glm::mat4& mat) { return glm::mat4_cast(quat) * mat; }
			),
			sol::meta_function::division, sol::resolve<glm::quat(const glm::quat&, const float&)>(glm::operator/),
			sol::meta_function::to_string, sol::resolve<std::string(const glm::quat&)>(glm::to_string),
			"x", &glm::quat::x,
			"y", &glm::quat::y,
			"z", &glm::quat::z,
			"w", &glm::quat::w,
			"yaw", sol::property([](const glm::quat& v) { return glm::yaw(v); }),
			"pitch", sol::property([](const glm::quat& v) { return glm::pitch(v); }),
			"roll", sol::property([](const glm::quat& v) { return glm::roll(v); }),
			"zero", sol::var(glm::quat()),
			"euler", [](float x, float y, float z){ return glm::quat(glm::vec3(x, y, z)); },
			"euler_deg", [](float x, float y, float z) { return glm::quat(glm::radians(glm::vec3(x, y, z))); },
			"norm", sol::resolve<glm::quat(const glm::quat&)>(glm::normalize),
			"slerp", sol::resolve<glm::quat(const glm::quat&, const glm::quat&, float)>(&glm::slerp),
			"euler_angles", sol::resolve<glm::vec3(const glm::quat&)>(&glm::eulerAngles),
			"angle_axis", [](float angle, const VarLuaTable<glm::vec3>& axis)
			{
				return glm::angleAxis(angle, LuaGlm::tableGet(axis));
			},
			"rotate", sol::resolve<glm::quat(const glm::quat&, const float&, const glm::vec3&)>(&glm::rotateNormalizedAxis),
			"rotate_x", [](const glm::quat& quat, float f) { return glm::rotateNormalizedAxis(quat, f, glm::vec3(1, 0, 0)); },
			"rotate_y", [](const glm::quat& quat, float f) { return glm::rotateNormalizedAxis(quat, f, glm::vec3(0, 1, 0)); },
			"rotate_z", [](const glm::quat& quat, float f) { return glm::rotateNormalizedAxis(quat, f, glm::vec3(0, 0, 1)); },
			"dot", sol::resolve<float(const glm::quat&, const glm::quat&)>(&glm::dot),
			"distance", &Math::distance,
			"rotate_towards", sol::resolve<glm::quat(const glm::quat&, const glm::quat&, float)>(&Math::rotateTowards)
		);
    }
}