#include "math.hpp"
#include "glm.hpp"
#include <darmok/color.hpp>
#include <glm/gtx/rotate_normalized_axis.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

namespace darmok
{
	template<glm::length_t L, glm::qualifier Q = glm::defaultp>
	void configLuaGlmUintVec(sol::usertype<glm::vec<L, glm::uint, Q>>& usertype) noexcept
	{
		LuaGlm::configVec(usertype);
	}

	template<glm::length_t L, glm::qualifier Q = glm::defaultp>
	void configLuaGlmIntVec(sol::usertype<glm::vec<L, int, Q>>& usertype) noexcept
	{
		LuaGlm::configVec(usertype);
	}

	void LuaMath::bindGlmUvec(sol::state_view& lua) noexcept
	{
		auto uvec4 = lua.new_usertype<glm::uvec4>("Uvec4", sol::constructors<
			glm::uvec4(glm::uint),
			glm::uvec4(glm::uint, glm::uint, glm::uint, glm::uint),
			glm::uvec4(const glm::uvec3&, glm::uint)
		>(),
			"x", &glm::uvec4::x,
			"y", &glm::uvec4::y,
			"z", &glm::uvec4::z,
			"w", &glm::uvec4::w
		);
		configLuaGlmUintVec(uvec4);

		auto uvec3 = lua.new_usertype<glm::uvec3>("Uvec3", sol::constructors<
			glm::uvec3(glm::uint),
			glm::uvec3(glm::uint, glm::uint, glm::uint),
			glm::ivec3(const glm::uvec2&, glm::uint),
			glm::ivec3(const glm::uvec4&)
		>(),
			"x", &glm::uvec3::x,
			"y", &glm::uvec3::y,
			"z", &glm::uvec3::z
		);
		configLuaGlmUintVec(uvec3);

		auto uvec2 = lua.new_usertype<glm::uvec2>("Uvec2", sol::constructors<
			glm::uvec2(glm::uint),
			glm::uvec2(glm::uint, glm::uint),
			glm::ivec3(const glm::uvec3&)
		>(),
			"x", &glm::uvec2::x,
			"y", &glm::uvec2::y
		);
		configLuaGlmUintVec(uvec2);
	}

	void LuaMath::bindGlmIvec(sol::state_view& lua) noexcept
	{
		auto ivec4 = lua.new_usertype<glm::ivec4>("Ivec4", sol::constructors<
			glm::ivec4(int),
			glm::ivec4(int, int, int, int),
			glm::ivec3(const glm::ivec3&, int)
		>(),
			"x", &glm::ivec4::x,
			"y", &glm::ivec4::y,
			"z", &glm::ivec4::z,
			"w", &glm::ivec4::w
		);
		configLuaGlmIntVec(ivec4);

		auto ivec3 = lua.new_usertype<glm::ivec3>("Ivec3", sol::constructors<
			glm::ivec3(int),
			glm::ivec3(int, int, int),
			glm::ivec3(const glm::ivec2&, int),
			glm::ivec3(const glm::ivec4&)
		>(),
			"x", &glm::ivec3::x,
			"y", &glm::ivec3::y,
			"z", &glm::ivec3::z
		);
		configLuaGlmIntVec(ivec3);

		auto ivec2 = lua.new_usertype<glm::ivec2>("Ivec2", sol::constructors<
			glm::ivec2(int),
			glm::ivec2(int, int),
			glm::ivec3(const glm::ivec3&)
		>(),
			"x", &glm::ivec2::x,
			"y", &glm::ivec2::y
		);
		configLuaGlmIntVec(ivec2);
    }
}