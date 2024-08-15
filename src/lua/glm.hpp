#pragma once

#include <sol/sol.hpp>
#include <darmok/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <variant>
#include <optional>
#include <darmok/color_fwd.hpp>
#include <darmok/math.hpp>
#include <glm/gtx/component_wise.hpp>

namespace darmok
{
	template<typename T>
	using VarLuaTable = std::variant<T, sol::table>;

	template<typename T>
	using VarLuaVecTable = std::variant<typename T::value_type, T, sol::table>;

	struct LuaGlm final
	{
		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static void configVec(sol::usertype<glm::vec<L, T, Q>>& usertype) noexcept
		{
			using vec = glm::vec<L, T, Q>;
			using val = T;
			configUsertype(usertype);
			usertype["zero"] = sol::var(vec(0));
			usertype["one"] = sol::var(vec(1));
			usertype["max"] = sol::resolve<val(const vec&)>(&glm::compMax);
			usertype["min"] = sol::resolve<val(const vec&)>(&glm::compMin);
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
			size_t count = table.size();
			for (glm::length_t i = 0; i < count && i < L; ++i)
			{
				vec[i] = table[i + 1];
			}
		}

		template<typename T, glm::qualifier Q = glm::defaultp>
		static void tableInit(glm::qua<T, Q>& qua, const sol::table& table)
		{
			size_t count = table.size();
			for (glm::length_t i = 0; i < count && i < qua.length(); ++i)
			{
				qua[i] = table[i + 1];
			}
		}

		template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
		static void tableInit(glm::mat<L1, L2, T, Q>& mat, const sol::table& table)
		{
			size_t count = table.size();
			for (glm::length_t i = 0; i < count && i < L1; ++i)
			{
				tableInit(mat[i], table[i + 1]);
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

		template<typename T>
		static typename T tableGet(const VarLuaVecTable<T>& v) noexcept
		{
			using vec = T;
			using val = typename T::value_type;
			auto ptr1 = std::get_if<val>(&v);
			if (ptr1 != nullptr)
			{
				return vec(*ptr1);
			}
			auto ptr2 = std::get_if<vec>(&v);
			if (ptr2 != nullptr)
			{
				return *ptr2;
			}
			vec vecVal;
			tableInit(vecVal, std::get<sol::table>(v));
			return vecVal;
		}
	};
}