#include "math.hpp"
#include "glm.hpp"
#include <darmok/color.hpp>
#include <glm/gtx/rotate_normalized_axis.hpp>

namespace darmok
{
	template<glm::length_t L, glm::qualifier Q = glm::defaultp>
	void configLuaGlmUvec(sol::usertype<glm::vec<L, glm::uint, Q>>& usertype) noexcept
	{
		using vec = glm::vec<L, glm::uint, Q>;
		LuaGlm::configUsertype(usertype);
		usertype["zero"] = sol::var(vec(0));
		usertype["max"] = &LuaGlm::vecMax<L, glm::uint, Q>;
		usertype["min"] = &LuaGlm::vecMin<L, glm::uint, Q>;
	}

	template<glm::length_t L, glm::qualifier Q = glm::defaultp>
	void configLuaGlmIvec(sol::usertype<glm::vec<L, int, Q>>& usertype) noexcept
	{
		using vec = glm::vec<L, int, Q>;
		LuaGlm::configUsertype(usertype);
		usertype["zero"] = sol::var(vec(0));
		usertype["max"] = &LuaGlm::vecMax<L, int, Q>;
		usertype["min"] = &LuaGlm::vecMin<L, int, Q>;
	}

	void LuaMath::bindGlmUvec(sol::state_view& lua) noexcept
	{
		auto uvec4 = lua.new_usertype<glm::uvec4>("Uvec4", sol::constructors<
			glm::uvec3(glm::uint),
			glm::uvec3(glm::uint, glm::uint, glm::uint, glm::uint)
		>(),
			"x", &glm::uvec4::x,
			"y", &glm::uvec4::y,
			"z", &glm::uvec4::z,
			"w", &glm::uvec4::w
		);
		configLuaGlmUvec(uvec4);

		auto uvec3 = lua.new_usertype<glm::uvec3>("Uvec3", sol::constructors<
			glm::uvec3(glm::uint),
			glm::uvec3(glm::uint, glm::uint, glm::uint)
		>(),
			"x", &glm::uvec3::x,
			"y", &glm::uvec3::y,
			"z", &glm::uvec3::z
		);
		configLuaGlmUvec(uvec3);

		auto uvec2 = lua.new_usertype<glm::uvec2>("Uvec2", sol::constructors<
			glm::uvec2(glm::uint),
			glm::uvec2(glm::uint, glm::uint)
		>(),
			"x", &glm::uvec2::x,
			"y", &glm::uvec2::y
		);
		configLuaGlmUvec(uvec2);
	}

	void LuaMath::bindGlmIvec(sol::state_view& lua) noexcept
	{
		auto ivec4 = lua.new_usertype<glm::ivec4>("Ivec4", sol::constructors<
			glm::ivec4(int),
			glm::ivec4(int, int, int, int)
		>(),
			"x", &glm::ivec4::x,
			"y", &glm::ivec4::y,
			"z", &glm::ivec4::z,
			"w", &glm::ivec4::w
		);
		configLuaGlmIvec(ivec4);

		auto ivec3 = lua.new_usertype<glm::ivec3>("Ivec3", sol::constructors<
			glm::ivec3(int),
			glm::ivec3(int, int, int)
		>(),
			"x", &glm::ivec3::x,
			"y", &glm::ivec3::y,
			"z", &glm::ivec3::z
		);
		configLuaGlmIvec(ivec3);

		auto ivec2 = lua.new_usertype<glm::ivec2>("Ivec2", sol::constructors<
			glm::ivec2(int),
			glm::ivec2(int, int)
		>(),
			"x", &glm::ivec2::x,
			"y", &glm::ivec2::y
		);
		configLuaGlmIvec(ivec2);
    }
}