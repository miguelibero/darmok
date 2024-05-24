#include "math.hpp"
#include "glm.hpp"
#include <darmok/color.hpp>
#include <darmok/math.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace darmok
{
	template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
	static void configLuaGlmMat(sol::usertype<glm::mat<L1, L2, T, Q>>& usertype) noexcept
	{
		using mat = glm::mat<L1, L2, T, Q>;
		using val = T;
		LuaGlm::configUsertype(usertype);
		usertype["id"] = sol::var(mat(1));
		usertype["inverse"] = sol::resolve<mat(const mat&)>(glm::inverse);
		usertype["transpose"] = sol::resolve<mat(const mat&)>(glm::transpose);
		usertype["det"] = sol::resolve<val(const mat&)>(glm::determinant);
	}

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
		usertype["dot"] = &glm::dot<L, val, Q>;
		usertype["lerp"] = &Math::lerp<vec>;
	}

	void LuaMath::bindGlmMat(sol::state_view& lua) noexcept
	{
		auto mat3 = lua.new_usertype<glm::mat3>("Mat3", sol::constructors<
			glm::mat3(glm::f32),
			glm::mat3(glm::vec3, glm::vec3, glm::vec3)
		>());
		configLuaGlmMat(mat3);
		mat3[sol::meta_function::multiplication] = sol::overload(
			[](const glm::mat3 mat, const glm::vec3& vec) { return mat * vec;  }
		);
		auto mat4 = lua.new_usertype<glm::mat4>("Mat4", sol::constructors <
			glm::mat4(glm::f32),
			glm::mat4(glm::vec4, glm::vec4, glm::vec4, glm::vec4)
		>(),
			"ortho", sol::overload(
				sol::resolve<glm::mat4(float, float, float, float, float, float)>(&Math::ortho),
				sol::resolve<glm::mat4(const glm::vec2&, const glm::vec2&, float, float)>(&Math::ortho)
			),
			"perspective", sol::overload(
				sol::resolve<glm::mat4(float, float, float, float)>(&Math::perspective),
				sol::resolve<glm::mat4(float, float, float)>(&Math::perspective)
			),
			"frustrum", sol::overload(
				sol::resolve<glm::mat4(float, float, float, float, float, float)>(&Math::frustrum),
				sol::resolve<glm::mat4(const glm::vec2&, const glm::vec2&, float, float)>(&Math::frustrum)
			),
			"look_at", sol::resolve<glm::mat4(const glm::vec3&, const glm::vec3&, const glm::vec3&)>(&glm::lookAt),
			"rotate", sol::resolve<glm::mat4(const glm::quat&)>(&glm::mat4_cast),
			"scale", sol::resolve<glm::mat4(const glm::vec3&)>(&glm::scale),
			"translate", sol::resolve<glm::mat4(const glm::vec3&)>(&glm::translate),
			"transform", sol::overload(&Math::transform, &Math::translateRotateScale),
			"decompose", [](const glm::mat4& mat) {
				glm::vec3 pos;
				glm::quat rot;
				glm::vec3 scale;
				glm::vec3 pivot;
				Math::decompose(mat, pos, rot, scale, pivot);
				return std::make_tuple( pos, rot, scale, pivot );
			},
			"translate_rotate_scale", &Math::translateRotateScale
		);
		configLuaGlmMat(mat4);
		mat4[sol::meta_function::multiplication] = sol::overload(
			[](const glm::mat4 mat, const glm::vec3& vec) -> glm::vec3 { return mat * glm::vec4(vec, 1);  },
			[](const glm::mat4 mat, const glm::vec4& vec) { return mat * vec;  }
		);
	}

	void LuaMath::bindGlmVec(sol::state_view& lua) noexcept
	{
		auto vec4 = lua.new_usertype<glm::vec4>("Vec4", sol::constructors<
				glm::vec4(glm::f32),
				glm::vec4(glm::f32, glm::f32, glm::f32, glm::f32),
				glm::vec4(const glm::ivec4&),
				glm::vec4(const glm::uvec4&),
				glm::vec3(const glm::vec3&, glm::f32)
			>(),
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
		
		auto vec3 = lua.new_usertype<glm::vec3>("Vec3", sol::constructors<
				glm::vec3(glm::f32),
				glm::vec3(glm::f32, glm::f32, glm::f32),
				glm::vec3(const glm::ivec3&),
				glm::vec3(const glm::uvec3&),
				glm::vec3(const glm::vec2&, glm::f32),
				glm::vec3(const glm::vec4&)

			>(),
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
			"rotate_z", sol::resolve<glm::vec3(const glm::vec3&, const glm::f32&)>(glm::rotateZ)
		);
		configLuaGlmFloatVec(vec3);

		auto vec2 = lua.new_usertype<glm::vec2>("Vec2", sol::constructors<
			glm::vec2(glm::f32),
			glm::vec2(glm::f32, glm::f32),
			glm::vec2(const glm::ivec2&),
			glm::vec2(const glm::uvec2&),
			glm::vec2(const glm::vec3&)
		>(),
			"x", &glm::vec2::x,
			"y", &glm::vec2::y
		);
		configLuaGlmFloatVec(vec2);
	}

	void LuaMath::bind(sol::state_view& lua) noexcept
	{
		lua["test_func"] = [](const glm::vec3& vec)
		{
			return vec;
		};
		auto degrees = sol::resolve<float(float)>(&glm::degrees);
		auto radians = sol::resolve<float(float)>(&glm::radians);
		lua.create_named_table("Math",
			"clamp", sol::overload(&glm::clamp<float>, &glm::clamp<int>),
			"lerp", sol::overload(&Math::lerp<float>, &Math::lerp<int>),
			"degrees", degrees,
			"radians", radians,
			"rad2deg", degrees,
			"deg2rad", radians
		);

		bindGlmMat(lua);
		bindGlmVec(lua);
		bindGlmUvec(lua);
		bindGlmIvec(lua);
		bindGlmQuat(lua);
		bindColor(lua);
    }
}