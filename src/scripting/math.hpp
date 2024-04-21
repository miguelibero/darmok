#pragma once

#include <string>
#include <variant>

#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include "sol.hpp"

namespace darmok
{
	struct LuaMath final
	{
		// had to split due to Visual Studio error C1126
		static void configure1(sol::state_view& lua) noexcept;
		static void configure2(sol::state_view& lua) noexcept;


		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static void loadFromTable(const sol::table& tab, glm::vec<L, T, Q>& v) noexcept
		{
			glm::length_t i = 0;
			for (auto& elm : tab)
			{
				v[i++] = elm.second.as<T>();
				if (i >= v.length())
				{
					break;
				}
			}
		}

		template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
		static void loadFromTable(const sol::table& tab, glm::mat<L1, L2, T, Q>& v) noexcept
		{
			glm::length_t i = 0;
			for (auto& elm : tab)
			{
				if (elm.second.is<sol::table>())
				{
					loadFromTable(elm.second.as<sol::table>(), v[i++]);
				}
				else if (elm.second.is<glm::vec<L2, T, Q>>())
				{
					v[i++] = elm.second.as<glm::vec<L2, T, Q>>();
				}
				if (i >= v.length())
				{
					break;
				}
			}
		}

		template<typename T>
		static T tableToGlm(const std::variant<T, sol::table>& v) noexcept
		{
			auto ptr = std::get_if<T>(&v);
			if (ptr != nullptr)
			{
				return *ptr;
			}
			return tableToGlm<T>(std::get<sol::table>(v));
		}

		template<typename T>
		static T tableToGlm(const sol::table& tab) noexcept
		{
			T v;
			loadFromTable(tab, v);
			return v;
		}

	private:

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureMat(sol::state_view& lua, std::string_view name) noexcept
		{
			using mat = T;
			using val = T::value_type;
			auto usertype = configureGlmOperators<T, Ctors...>(lua, name);
			usertype["id"] = sol::var(T());
			usertype["inverse"] = sol::resolve<mat(const mat&)>(glm::inverse);
			usertype["transpose"] = sol::resolve<mat(const mat&)>(glm::transpose);
			usertype["det"] = sol::resolve<val(const mat&)>(glm::determinant);
			return usertype;
		}

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureUvec(sol::state_view& lua, std::string_view name) noexcept
		{
			using vec = T;
			using val = T::value_type;
			auto usertype = configureGlmOperators<T, Ctors...>(lua, name);
			usertype["zero"] = sol::var(vec(0));
			return usertype;
		}

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureVec(sol::state_view& lua, std::string_view name) noexcept
		{
			using vec = T;
			using val = T::value_type;
			auto usertype = configureGlmOperators<T, Ctors...>(lua, name);
			usertype["zero"] = sol::var(vec(0));
			usertype["dot"] = sol::resolve<val(const vec&, const vec&)>(glm::dot);
			usertype["norm"] = sol::resolve<vec(const vec&)>(glm::normalize);
			usertype["radians"] = sol::resolve<vec(const vec&)>(glm::radians);
			usertype["degrees"] = sol::resolve<vec(const vec&)>(glm::degrees);
			usertype["clamp"] = sol::overload(sol::resolve<vec(const vec&, const vec&, const vec&)>(glm::clamp), sol::resolve<vec(const vec&, val, val)>(glm::clamp));
			return usertype;
		}

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureGlmOperators(sol::state_view& lua, std::string_view name) noexcept
		{
			using cls = T;
			using val = T::value_type;
			return lua.new_usertype<cls>(name, sol::constructors<Ctors...>(),
				sol::meta_function::equal_to,		sol::resolve<bool(const cls&, const cls&)>(glm::operator==),
				sol::meta_function::addition,		sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator+), sol::resolve<cls(const cls&, val)>(glm::operator+)),
				sol::meta_function::subtraction,	sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator-), sol::resolve<cls(const cls&, val)>(glm::operator-)),
				sol::meta_function::unary_minus,	sol::resolve<cls(const cls&)>(glm::operator-),
				sol::meta_function::multiplication, sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator*), sol::resolve<cls(const cls&, val)>(glm::operator*)),
				sol::meta_function::division,		sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator/), sol::resolve<cls(const cls&, val)>(glm::operator/)),
				sol::meta_function::to_string,		sol::resolve<std::string(const cls&)>(glm::to_string)
			);
		}
	};

}