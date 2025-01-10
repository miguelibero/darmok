#include "math.hpp"
#include "glm.hpp"
#include <darmok/color.hpp>

namespace darmok
{
	void LuaMath::bindColor(sol::state_view& lua) noexcept
	{
		auto color = lua.new_usertype<Color>("Color",
			sol::factories(
				[](uint8_t r, uint8_t g, uint8_t b, uint8_t a) -> Color { return Color(r, g, b, a); },
				[](uint8_t r, uint8_t g, uint8_t b) -> Color { return Color(r, g, b, Colors::getMaxValue()); }
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
}