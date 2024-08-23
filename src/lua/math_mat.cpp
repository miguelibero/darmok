#include "math.hpp"
#include "glm.hpp"
#include <darmok/math.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/transform.hpp>

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
		usertype[sol::meta_function::index] = [](const mat& mat, int idx) { return mat[idx]; };
		usertype[sol::meta_function::new_index] =
			[](mat& mat, int idx, const VarLuaTable<typename mat::col_type>& v) {
				mat[idx] = LuaGlm::tableGet(v);
			};
	}

	void LuaMath::bindGlmMat(sol::state_view& lua) noexcept
	{
		auto mat3 = lua.new_usertype<glm::mat3>("Mat3", sol::constructors<
				glm::mat3(glm::f32),
				glm::mat3(glm::vec3, glm::vec3, glm::vec3)
			>()
		);
		configLuaGlmMat(mat3);
		mat3[sol::meta_function::multiplication] = sol::overload(
			[](const glm::mat3& mat1, const glm::mat3& mat2) { return mat1 * mat2;  },
			[](const glm::mat3& mat, float val) { return mat * val; },
			[](const glm::mat3& mat, const glm::vec3& vec) { return mat * vec;  }
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
			"frustum", sol::overload(
				sol::resolve<glm::mat4(float, float, float, float, float, float)>(&Math::frustum),
				sol::resolve<glm::mat4(const glm::vec2&, const glm::vec2&, float, float)>(&Math::frustum)
			),
			"look_at", sol::resolve<glm::mat4(const glm::vec3&, const glm::vec3&, const glm::vec3&)>(&glm::lookAt),
			"rotate", sol::overload(
				sol::resolve<glm::mat4(const glm::quat&)>(&glm::mat4_cast),
				[](const glm::mat4& mat, const glm::quat& quat) { return mat * glm::mat4_cast(quat); }
			),
			"scale", sol::overload(
				sol::resolve<glm::mat4(const glm::vec3&)>(&glm::scale),
				sol::resolve<glm::mat4(const glm::mat4&, const glm::vec3&)>(&glm::scale)
			),
			"translate", sol::overload(
				sol::resolve<glm::mat4(const glm::vec3&)>(&glm::translate),
				sol::resolve<glm::mat4(const glm::mat4&, const glm::vec3&)>(&glm::translate)
			),
			"transform", &Math::transform,
			"decompose", [](const glm::mat4& mat) {
				glm::vec3 pos;
				glm::quat rot;
				glm::vec3 scale;
				Math::decompose(mat, pos, rot, scale);
				return std::make_tuple( pos, rot, scale );
			},
			"translate_rotate_scale", &Math::transform
		);
		configLuaGlmMat(mat4);
		mat4[sol::meta_function::multiplication] = sol::overload(
			[](const glm::mat4& mat1, const glm::mat4& mat2) { return mat1 * mat2;  },
			[](const glm::mat4& mat, float val) { return mat * val; },
			[](const glm::mat4& mat, const glm::vec3& vec) -> glm::vec3 { return mat * glm::vec4(vec, 1); },
			[](const glm::mat4& mat, const glm::vec4& vec) { return mat * vec; },
			[](const glm::mat4& mat, const glm::quat& quat) { return mat * glm::mat4_cast(quat); }
		);
	}	
}