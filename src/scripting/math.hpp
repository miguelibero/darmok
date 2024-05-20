#pragma once

#include <string>
#include <variant>

#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include <sol/sol.hpp>
#include <darmok/color.hpp>

namespace darmok
{
	struct LuaMath final
	{
		static void bind(sol::state_view& lua) noexcept;

		template<typename T>
		static T lerp(const T& a, const T& b, float p) noexcept
		{
			return a + (b * p);
		}

	private:
		
		static void bindGlmMat(sol::state_view& lua) noexcept;
		static void bindGlmVec(sol::state_view& lua) noexcept;
		static void bindGlmUvec(sol::state_view& lua) noexcept;
		static void bindGlmIvec(sol::state_view& lua) noexcept;
		static void bindGlmQuat(sol::state_view& lua) noexcept;
		static void bindColor(sol::state_view& lua) noexcept;
	};

	// trying to convert from glm to sol::table implicitly
	// https://sol2.readthedocs.io/en/latest/tutorial/customization.html

	struct LuaGlm final
	{
		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static T vecMax(const glm::vec<L, T, Q>& v) noexcept
		{
			using vec = glm::vec<L, T, Q>;
			using val = T;
			val result = std::numeric_limits<val>::min();
			for (glm::length_t i = 0; i < vec::length(); i++)
			{
				auto j = v[i];
				if (j > result)
				{
					result = j;
				}
			}
			return result;
		}

		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static T vecMin(const  glm::vec<L, T, Q>& v) noexcept
		{
			using vec = glm::vec<L, T, Q>;
			using val = T;
			val result = std::numeric_limits<val>::max();
			for (glm::length_t i = 0; i < vec::length(); i++)
			{
				auto j = v[i];
				if (j < result)
				{
					result = j;
				}
			}
			return result;
		}


		template<typename T>
		static void configUsertype(sol::usertype<T>& usertype) noexcept
		{
			using cls = T;
			using val = T::template value_type;
			usertype[sol::meta_function::equal_to] = sol::resolve<bool(const cls&, const cls&)>(glm::operator==);
			usertype[sol::meta_function::addition] = sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator+), sol::resolve<cls(const cls&, val)>(glm::operator+));
			usertype[sol::meta_function::subtraction] = sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator-), sol::resolve<cls(const cls&, val)>(glm::operator-));
			usertype[sol::meta_function::unary_minus] = sol::resolve<cls(const cls&)>(glm::operator-);
			usertype[sol::meta_function::multiplication] = sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator*), sol::resolve<cls(const cls&, val)>(glm::operator*));
			usertype[sol::meta_function::division] = sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator/), sol::resolve<cls(const cls&, val)>(glm::operator/));
			usertype[sol::meta_function::to_string] = sol::resolve<std::string(const cls&)>(glm::to_string);
		}


		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static void tableInitVec(glm::vec<L, T, Q>& vec, const sol::table& table)
		{
			glm::length_t i = 0;
			for (auto& elm : table)
			{
				vec[i++] = elm.second.as<T>();
				if (i >= L)
				{
					break;
				}
			}
		}

		template<typename Handler, glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static bool vecSolCheck(sol::types<glm::vec<L, T, Q>>, lua_State* lua, int index, Handler&& handler, sol::stack::record& tracking) noexcept
		{
			if (!numberTableSolCheck(L, lua, index, tracking))
			{
				handler(lua, index,
					sol::type_of(lua, index),
					sol::type::table,
					"expected a number table");
				return false;
			}
			return true;
		}

		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static glm::vec<L, T, Q> vecSolGet(sol::types<glm::vec<L, T, Q>>, lua_State* lua, int index, sol::stack::record& tracking)
		{
			glm::vec<L, T, Q> vec;
			int absindex = lua_absindex(lua, index);
			auto table = sol::stack::get<sol::lua_table>(lua, absindex, tracking);
			tableInitVec(vec, table);
			tracking.use(1);
			return vec;
		}

		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static int vecSolPush(lua_State* lua, const glm::vec<L, T, Q>& vec)
		{
			auto table = sol::table::create_with(lua);
			for (glm::length_t i = 0; i < L; i++)
			{
				table[i + 1] = vec[i];
			}
			return sol::stack::push(lua, table);
		}

		template<typename T, glm::qualifier Q = glm::defaultp>
		static void tableInitQuat(glm::qua<T, Q>& qua, const sol::table& table)
		{
			glm::length_t i = 0;
			for (auto& elm : table)
			{
				qua[i++] = elm.second.as<T>();
				if (i >= qua.length())
				{
					break;
				}
			}
		}

		template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
		static void tableInitMat(glm::mat<L1, L2, T, Q>& mat, const sol::table& table)
		{
			glm::length_t i = 0;
			for (auto& elm : table)
			{
				tableInitGlmVec(mat[i++], elm.second);
				if (i >= L1)
				{
					break;
				}
			}
		}

		static bool numberTableSolCheck(size_t size, lua_State* lua, int index, sol::stack::record& tracking) noexcept;		
	};
}

namespace sol {
	template<>
	struct lua_type_of<glm::vec3>
		: std::integral_constant<sol::type, sol::type::table> {};
}

template <typename Handler>
inline bool sol_lua_check(sol::types<glm::vec3> type, lua_State* lua, int index, Handler&& handler, sol::stack::record& tracking)
{
	return darmok::LuaGlm::vecSolCheck(type, lua, index, std::forward<Handler>(handler), tracking);
}

inline glm::vec3 sol_lua_get(sol::types<glm::vec3> type, lua_State* lua, int index, sol::stack::record& tracking)
{
	return darmok::LuaGlm::vecSolGet(type, lua, index, tracking);
}

template <typename Handler>
inline sol::optional<glm::vec3> sol_lua_check_get(sol::types<glm::vec3> type, lua_State* lua, int index, Handler&& handler, sol::stack::record& tracking)
{
	if (sol_lua_check(type, lua, index, std::forward<Handler>(handler), tracking))
	{
		return sol_lua_get(type, lua, index, tracking);
	}
	return sol::nullopt;
}

inline int sol_lua_push(lua_State* lua, const glm::vec3& val)
{
	return darmok::LuaGlm::vecSolPush(lua, val);
}