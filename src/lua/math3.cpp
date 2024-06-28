#include "math.hpp"
#include "glm.hpp"
#include <darmok/color.hpp>
#include <glm/gtx/rotate_normalized_axis.hpp>

namespace darmok
{
	void LuaMath::bindColor(sol::state_view& lua) noexcept
	{
		auto color = lua.new_usertype<Color>("Color",
			sol::factories(
				[](uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return Color(r, g, b, a); },
				[](uint8_t r, uint8_t g, uint8_t b) { return Color(r, g, b, Colors::getMaxValue()); }
			),
			"r", &Color::r, 
			"g", &Color::g, 
			"b", &Color::b, 
			"a", &Color::a, 
			"black", sol::var(Colors::black()), 
			"white", sol::var(Colors::white()), 
			"red", sol::var(Colors::red()), 
			"green", sol::var(Colors::green()), 
			"blue", sol::var(Colors::blue()), 
			"yellow", sol::var(Colors::yellow()), 
			"cyan", sol::var(Colors::cyan()), 
			"magenta", sol::var(Colors::magenta()), 
			"norm", sol::resolve<glm::vec4(const Color&)>(Colors::normalize), 
			"to_num", sol::resolve<uint32_t(const Color&)>(&Colors::toNumber)
		);
		LuaGlm::configUsertype(color);

		auto color3 = lua.new_usertype<Color3>("Color3", sol::constructors<
			Color(uint8_t, uint8_t, uint8_t)
		>(),
			"r", &Color3::r, 
			"g", &Color3::g, 
			"b", &Color3::b, 
			"black", sol::var(Colors::black3()), 
			"white", sol::var(Colors::white3()), 
			"red", sol::var(Colors::red3()), 
			"green", sol::var(Colors::green3()), 
			"blue", sol::var(Colors::blue3()), 
			"yellow", sol::var(Colors::yellow3()), 
			"cyan", sol::var(Colors::cyan3), 
			"magenta", sol::var(Colors::magenta3()), 
			"norm", sol::resolve<glm::vec3(const Color3&)>(Colors::normalize), 
			"to_num", sol::resolve<uint32_t(const Color3&)>(&Colors::toNumber)
		);
		LuaGlm::configUsertype(color3);
	}

	glm::quat quatEuler(float x, float y, float z) noexcept
	{
		return glm::quat(glm::vec3(x, y, z));
	}

	void LuaMath::bindGlmQuat(sol::state_view& lua) noexcept
	{
		lua.new_usertype<glm::quat>("Quat",
			sol::constructors<glm::quat(float, float, float, float), glm::quat(const glm::vec3&)>(),
			sol::meta_function::equal_to, sol::resolve<bool(const glm::quat&, const glm::quat&)>(glm::operator==),
			sol::meta_function::addition, sol::resolve<glm::quat(const glm::quat&, const glm::quat&)>(glm::operator+),
			sol::meta_function::subtraction, sol::resolve<glm::quat(const glm::quat&, const glm::quat&)>(glm::operator-),
			sol::meta_function::unary_minus, sol::resolve<glm::quat(const glm::quat&)>(glm::operator-),
			sol::meta_function::multiplication, sol::overload(sol::resolve<glm::quat(const glm::quat&, const glm::quat&)>(glm::operator*), sol::resolve<glm::quat(const glm::quat&, const float&)>(glm::operator*)),
			sol::meta_function::division, sol::resolve<glm::quat(const glm::quat&, const float&)>(glm::operator/),
			sol::meta_function::to_string, sol::resolve<std::string(const glm::quat&)>(glm::to_string),
			"x", &glm::quat::x,
			"y", &glm::quat::y,
			"z", &glm::quat::z,
			"w", &glm::quat::w,
			"zero", sol::var(glm::quat()),
			"euler", &quatEuler,
			"slerp", sol::resolve<glm::quat(const glm::quat&, const glm::quat&, float)>(&glm::slerp),
			"euler_angles", sol::resolve<glm::vec3(const glm::quat&)>(&glm::eulerAngles),
			"angle_axis", sol::resolve<glm::quat(const float&, const glm::vec3&)>(&glm::angleAxis),
			"rotate_axis", sol::resolve<glm::quat(const glm::quat&, const float&, const glm::vec3&)>(&glm::rotateNormalizedAxis)
		);
    }
}