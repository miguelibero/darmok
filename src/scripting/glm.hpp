#pragma once

#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <variant>
#include <optional>
#include <darmok/color_fwd.hpp>

namespace darmok
{
	template<typename T>
	using VarLuaTable = std::variant<T, sol::table>;

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

		template<typename T>
		static void tableInit(std::optional<T>& opt, const sol::table& table)
		{
			if (table.empty())
			{
				opt.reset();
				return;
			}
			if (opt)
			{
				tableInit(opt.value(), table);
			}
			else
			{
				T val;
				tableInit(val, table);
				opt = val;
			}
		}


		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static void tableInit(glm::vec<L, T, Q>& vec, const sol::table& table)
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

		template<typename T, glm::qualifier Q = glm::defaultp>
		static void tableInit(glm::qua<T, Q>& qua, const sol::table& table)
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
		static void tableInit(glm::mat<L1, L2, T, Q>& mat, const sol::table& table)
		{
			glm::length_t i = 0;
			for (auto& elm : table)
			{
				tableInit(mat[i++], elm.second);
				if (i >= L1)
				{
					break;
				}
			}
		}

		template<typename T>
		static T tableGet(const VarLuaTable<T>& v) noexcept
		{
			auto ptr = std::get_if<T>(&v);
			if (ptr != nullptr)
			{
				return *ptr;
			}
			T val;
			tableInit(val, std::get<sol::table>(v));
			return val;
		}
	};
}