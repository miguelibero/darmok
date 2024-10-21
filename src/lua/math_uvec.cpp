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
		auto uvec4 = lua.new_usertype<glm::uvec4>("Uvec4",
			sol::factories(
				[](glm::uint v) { return glm::uvec4(v); },
				[](glm::uint x, glm::uint y, glm::uint z, glm::uint w) { return glm::uvec4(x, y, z, w); },
				[](const glm::ivec2& v, glm::uint z, glm::uint w) { return glm::uvec4(v, z, w); },
				[](const glm::ivec3& v, glm::uint w) { return glm::uvec4(v, w); },
				[](const glm::ivec4& v) { return glm::uvec4(v); },
				[](const glm::uvec2& v, glm::uint z, glm::uint w) { return glm::uvec4(v, z, w); },
				[](const glm::uvec3& v, glm::uint w) { return glm::uvec4(v, w); },
				[](const glm::vec2& v, glm::uint z, glm::uint w) { return glm::uvec4(v, z, w); },
				[](const glm::vec3& v, glm::uint w) { return glm::uvec4(v, w); },
				[](const glm::vec4& v) { return glm::uvec4(v); },
				[](const sol::table& tab) { return LuaGlm::tableGet<glm::uvec4>(tab); }
			),
			"x", &glm::uvec4::x,
			"y", &glm::uvec4::y,
			"z", &glm::uvec4::z,
			"w", &glm::uvec4::w
		);
		configLuaGlmUintVec(uvec4);

		auto uvec3 = lua.new_usertype<glm::uvec3>("Uvec3",
			sol::factories(
				[](glm::uint v) { return glm::uvec3(v); },
				[](glm::uint x, glm::uint y, glm::uint z) { return glm::uvec3(x, y, z); },
				[](const glm::ivec2& v, glm::uint z) { return glm::uvec3(v, z); },
				[](const glm::ivec4& v) { return glm::uvec3(v); },
				[](const glm::uvec2& v, glm::uint z) { return glm::uvec3(v, z); },
				[](const glm::uvec3& v) { return glm::uvec3(v); },
				[](const glm::uvec4& v) { return glm::uvec3(v); },
				[](const glm::vec2& v, glm::uint z) { return glm::uvec3(v, z); },
				[](const glm::vec3& v) { return glm::uvec3(v); },
				[](const glm::vec4& v) { return glm::uvec3(v); },
				[](const sol::table& tab) { return LuaGlm::tableGet<glm::uvec3>(tab); }
			),
			"x", &glm::uvec3::x,
			"y", &glm::uvec3::y,
			"z", &glm::uvec3::z
		);
		configLuaGlmUintVec(uvec3);

		auto uvec2 = lua.new_usertype<glm::uvec2>("Uvec2",
			sol::factories(
				[](glm::uint v) { return glm::uvec2(v); },
				[](glm::uint x, glm::uint y) { return glm::uvec2(x, y); },
				[](const glm::ivec3& v) { return glm::uvec2(v); },
				[](const glm::ivec4& v) { return glm::uvec2(v); },
				[](const glm::uvec2& v) { return glm::uvec2(v); },
				[](const glm::uvec3& v) { return glm::uvec2(v); },
				[](const glm::uvec4& v) { return glm::uvec2(v); },
				[](const glm::vec2& v) { return glm::uvec2(v); },
				[](const glm::vec3& v) { return glm::uvec2(v); },
				[](const glm::vec4& v) { return glm::uvec2(v); },
				[](const sol::table& tab) { return LuaGlm::tableGet<glm::uvec2>(tab); }
			),
			"x", &glm::uvec2::x,
			"y", &glm::uvec2::y
		);
		configLuaGlmUintVec(uvec2);
	}

	void LuaMath::bindGlmIvec(sol::state_view& lua) noexcept
	{
		auto ivec4 = lua.new_usertype<glm::ivec4>("Ivec4",
			sol::factories(
				[](int v) { return glm::ivec4(v); },
				[](int x, int y, int z, int w) { return glm::ivec4(x, y, z, w); },
				[](const glm::ivec2& v, int z, int w) { return glm::ivec4(v, z, w); },
				[](const glm::ivec3& v, int w) { return glm::ivec4(v, w); },
				[](const glm::uvec2& v, int z, int w) { return glm::ivec4(v, z, w); },
				[](const glm::uvec3& v, int w) { return glm::ivec4(v, w); },
				[](const glm::uvec4& v) { return glm::ivec4(v); },
				[](const glm::vec2& v, int z, int w) { return glm::ivec4(v, z, w); },
				[](const glm::vec3& v, int w) { return glm::ivec4(v, w); },
				[](const glm::vec4& v) { return glm::ivec4(v); },
				[](const sol::table& tab) { return LuaGlm::tableGet<glm::ivec4>(tab); }
			),
			"x", &glm::ivec4::x,
			"y", &glm::ivec4::y,
			"z", &glm::ivec4::z,
			"w", &glm::ivec4::w
		);
		configLuaGlmIntVec(ivec4);

		auto ivec3 = lua.new_usertype<glm::ivec3>("Ivec3",
			sol::factories(
				[](int v) { return glm::ivec3(v); },
				[](int x, int y, int z) { return glm::ivec3(x, y, z); },
				[](const glm::ivec2& v, int z) { return glm::ivec3(v, z); },
				[](const glm::ivec4& v) { return glm::ivec3(v); },
				[](const glm::uvec2& v, int z) { return glm::ivec3(v, z); },
				[](const glm::uvec3& v) { return glm::ivec3(v); },
				[](const glm::uvec4& v) { return glm::ivec3(v); },
				[](const glm::vec2& v, int z) { return glm::ivec3(v, z); },
				[](const glm::vec3& v) { return glm::ivec3(v); },
				[](const glm::vec4& v) { return glm::ivec3(v); },
				[](const sol::table& tab) { return LuaGlm::tableGet<glm::ivec3>(tab); }
			),
			"x", &glm::ivec3::x,
			"y", &glm::ivec3::y,
			"z", &glm::ivec3::z
		);
		configLuaGlmIntVec(ivec3);

		auto ivec2 = lua.new_usertype<glm::ivec2>("Ivec2",
			sol::factories(
				[](int v) { return glm::ivec2(v); },
				[](int x, int y) { return glm::ivec2(x, y); },
				[](const glm::ivec3& v) { return glm::ivec2(v); },
				[](const glm::ivec4& v) { return glm::ivec2(v); },
				[](const glm::uvec2& v) { return glm::ivec2(v); },
				[](const glm::uvec3& v) { return glm::ivec2(v); },
				[](const glm::uvec4& v) { return glm::ivec2(v); },
				[](const glm::vec2& v) { return glm::ivec2(v); },
				[](const glm::vec3& v) { return glm::ivec2(v); },
				[](const glm::vec4& v) { return glm::ivec2(v); },
				[](const sol::table& tab) { return LuaGlm::tableGet<glm::ivec2>(tab); }
			),
			"x", &glm::ivec2::x,
			"y", &glm::ivec2::y
		);
		configLuaGlmIntVec(ivec2);
    }
}