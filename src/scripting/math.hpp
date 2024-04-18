#pragma once

#include <string>

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
	private:

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureMat(sol::state_view& lua, std::string_view name) noexcept
		{
			using mat = T;
			using val = T::value_type;
			auto usertype = configureGlmOperators<T, Ctors...>(lua, name);
			usertype["inverse"] = sol::resolve<mat(const mat&)>(glm::inverse);
			usertype["transpose"] = sol::resolve<mat(const mat&)>(glm::transpose);
			usertype["det"] = sol::resolve<val(const mat&)>(glm::determinant);
			return usertype;
		}

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureUvec(sol::state_view& lua, std::string_view name) noexcept
		{
			auto usertype = configureGlmOperators<T, Ctors...>(lua, name);
			usertype["zero"] = sol::var(T());
			return usertype;
		}

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureVec(sol::state_view& lua, std::string_view name) noexcept
		{
			using vec = T;
			using val = T::value_type;
			auto usertype = configureGlmOperators<T, Ctors...>(lua, name);
			usertype["zero"] = sol::var(T());
			usertype["dot"] = sol::resolve<val(const vec&, const vec&)>(glm::dot);
			usertype["norm"] = sol::resolve<vec(const vec&)>(glm::normalize);
			return usertype;
		}

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureGlmOperators(sol::state_view& lua, std::string_view name) noexcept
		{
			using cls = T;
			using val = T::value_type;
			return lua.new_usertype<cls>(name, sol::constructors<Ctors...>(),
				sol::meta_function::addition,		sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator+), sol::resolve<cls(const cls&, val)>(glm::operator+)),
				sol::meta_function::subtraction,	sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator-), sol::resolve<cls(const cls&, val)>(glm::operator-), sol::resolve<cls(const cls&)>(glm::operator-)),
				sol::meta_function::multiplication, sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator*), sol::resolve<cls(const cls&, val)>(glm::operator*)),
				sol::meta_function::division,		sol::overload(sol::resolve<cls(const cls&, const cls&)>(glm::operator/), sol::resolve<cls(const cls&, val)>(glm::operator/)),
				sol::meta_function::to_string,		sol::resolve<std::string(const cls&)>(glm::to_string)
			);
		}
	};

}