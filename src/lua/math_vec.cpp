#include "lua/math.hpp"
#include "lua/glm.hpp"
#include <glm/gtx/rotate_normalized_axis.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

namespace darmok
{
	template<glm::length_t L, glm::qualifier Q = glm::defaultp>
	void configLuaGlmFloatVec(sol::usertype<glm::vec<L, glm::f32, Q>>& usertype) noexcept
	{
		using vec = glm::vec<L, glm::f32, Q>;
		using val = glm::f32;
		LuaGlm::configVec(usertype);
		usertype["norm"] = sol::resolve<vec(const vec&)>(glm::normalize);
		usertype["radians"] = sol::resolve<vec(const vec&)>(glm::radians);
		usertype["degrees"] = sol::resolve<vec(const vec&)>(glm::degrees);
		usertype["clamp"] = sol::overload(
			sol::resolve<vec(const vec&, const vec&, const vec&)>(glm::clamp),
			sol::resolve<vec(const vec&, val, val)>(glm::clamp)
		);
		usertype["almost_zero"] = sol::overload(
			[](const vec& v) { return Math::almostZero(v); },
			[](const vec& v, int factor) { return Math::almostZero(v, factor); }
		);
		usertype["dot"] = &glm::dot<L, val, Q>;
		usertype["lerp"] = &Math::lerp<vec>;
		usertype["length"] = sol::resolve<val(const vec&)>(&glm::length);
		usertype["length2"] = sol::resolve<val(const vec&)>(&glm::length2);
		usertype["abs"] = sol::resolve<vec(const vec&)>(&glm::abs);
	}

	void LuaMath::bindGlmVec(sol::state_view& lua) noexcept
	{
		auto vec4 = lua.new_usertype<glm::vec4>("Vec4", 
			sol::factories(
				[](float v) { return glm::vec4(v); },
				[](float x, float y, float z, float w) { return glm::vec4(x, y, z, w); },
				[](const glm::ivec2& v, float z, float w) { return glm::vec4(v, z, w); },
				[](const glm::ivec3& v, float w) { return glm::vec4(v, w); },
				[](const glm::ivec4& v) { return glm::vec4(v); },
				[](const glm::uvec2& v, float z, float w) { return glm::vec4(v, z, w); },
				[](const glm::uvec3& v, float w) { return glm::vec4(v, w); },
				[](const glm::uvec4& v) { return glm::ivec4(v); },
				[](const glm::vec2& v, float z, float w) { return glm::vec4(v, z, w); },
				[](const glm::vec3& v, float w) { return glm::vec4(v, w); },
				[](const sol::table& tab) { return LuaGlm::tableGet<glm::vec4>(tab); }
			),
			"x", &glm::vec4::x,
			"y", &glm::vec4::y,
			"z", &glm::vec4::z,
			"w", &glm::vec4::w,
			"rotate", sol::resolve<glm::vec4(const glm::vec4&, const glm::f32&, const glm::vec3&)>(glm::rotate),
			"rotate_x", sol::resolve<glm::vec4(const glm::vec4&, const glm::f32&)>(glm::rotateX),
			"rotate_y", sol::resolve<glm::vec4(const glm::vec4&, const glm::f32&)>(glm::rotateY),
			"rotate_z", sol::resolve<glm::vec4(const glm::vec4&, const glm::f32&)>(glm::rotateZ)
		);
		configLuaGlmFloatVec(vec4);
		vec4[sol::meta_function::multiplication] = sol::overload(
			sol::resolve<glm::vec4(const glm::vec4&, const glm::vec4&)>(glm::operator*),
			sol::resolve<glm::vec4(const glm::vec4&, float)>(glm::operator*),
			sol::resolve<glm::vec4(const glm::vec4&, const glm::quat&)>(glm::operator*)
		);

		auto vec3 = lua.new_usertype<glm::vec3>("Vec3",
			sol::factories(
				[](float v) { return glm::vec3(v); },
				[](float x, float y, float z) { return glm::vec3(x, y, z); },
				[](const glm::ivec2& v, float z) { return glm::vec3(v, z); },
				[](const glm::ivec3& v) { return glm::vec3(v); },
				[](const glm::ivec4& v) { return glm::vec3(v); },
				[](const glm::uvec2& v, float z) { return glm::vec3(v, z); },
				[](const glm::uvec3& v) { return glm::vec3(v); },
				[](const glm::uvec4& v) { return glm::vec3(v); },
				[](const glm::vec2& v, float z) { return glm::vec3(v, z); },
				[](const glm::vec4& v) { return glm::vec3(v); },
				[](const sol::table& tab) { return LuaGlm::tableGet<glm::vec3>(tab); }
			),
			"x", &glm::vec3::x,
			"y", &glm::vec3::y,
			"z", &glm::vec3::z,
			"left", sol::var(glm::vec3(-1, 0, 0)),
			"right", sol::var(glm::vec3(1, 0, 0)),
			"up", sol::var(glm::vec3(0, 1, 0)),
			"down", sol::var(glm::vec3(0, -1, 0)),
			"forward", sol::var(glm::vec3(0, 0, 1)),
			"backward", sol::var(glm::vec3(0, 0, -1)),
			"cross", sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(glm::cross),
			"project", sol::resolve<glm::vec3(const glm::vec3&, const glm::mat4&, const glm::mat4&, const glm::ivec4&)>(glm::project),
			"unproject", sol::resolve<glm::vec3(const glm::vec3&, const glm::mat4&, const glm::mat4&, const glm::ivec4&)>(glm::unProject),
			"rotate", sol::resolve<glm::vec3(const glm::vec3&, const float&, const glm::vec3&)>(glm::rotate),
			"rotate_x", sol::resolve<glm::vec3(const glm::vec3&, const glm::f32&)>(glm::rotateX),
			"rotate_y", sol::resolve<glm::vec3(const glm::vec3&, const glm::f32&)>(glm::rotateY),
			"rotate_z", sol::resolve<glm::vec3(const glm::vec3&, const glm::f32&)>(glm::rotateZ),
			"move_towards", &Math::moveTowards,
			"rotate_towards", sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&, float, float)>(&Math::rotateTowards)
		);
		configLuaGlmFloatVec(vec3);
		vec3[sol::meta_function::multiplication] = sol::overload(
			sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(glm::operator*),
			sol::resolve<glm::vec3(const glm::vec3&, float)>(glm::operator*),
			[](const glm::vec3& vec, const glm::quat& quat) -> glm::vec3 { return vec * quat; }
		);

		auto vec2 = lua.new_usertype<glm::vec2>("Vec2",
			sol::factories(
				[](float v) { return glm::vec2(v); },
				[](float x, float y) { return glm::vec2(x, y); },
				[](const glm::ivec2& v) { return glm::vec2(v); },
				[](const glm::ivec3& v) { return glm::vec2(v); },
				[](const glm::ivec4& v) { return glm::vec2(v); },
				[](const glm::uvec2& v) { return glm::vec2(v); },
				[](const glm::uvec3& v) { return glm::vec2(v); },
				[](const glm::uvec4& v) { return glm::vec2(v); },
				[](const glm::vec3& v) { return glm::vec2(v); },
				[](const glm::vec4& v) { return glm::vec2(v); },
				[](const sol::table& tab) { return LuaGlm::tableGet<glm::vec2>(tab); }
			),
			"x", &glm::vec2::x,
			"y", &glm::vec2::y,
			"rotate", [](const glm::vec2& v, float angle) { return glm::rotate(v, angle); },
			"angle", sol::property([](const glm::vec2& v) { return std::atan2(v.y, v.x); }),
			"vert_angle", sol::property([](const glm::vec2& v) { return std::atan2(v.x, v.y); })
		);
		configLuaGlmFloatVec(vec2);
	}
}