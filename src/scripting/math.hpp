#pragma once

#include <string>
#include <variant>

#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include <sol/sol.hpp>
#include <darmok/color.hpp>

namespace darmok
{
    using VarVec2 = std::variant<glm::vec2, sol::table>;
	using VarVec3 = std::variant<glm::vec3, sol::table>;
	using VarVec4 = std::variant<glm::vec4, sol::table>;
	using VarUvec2 = std::variant<glm::uvec2, sol::table>;
	using VarQuat = std::variant<glm::quat, sol::table>;
	using VarColor3 = std::variant<Color3, sol::table>;
	using VarColor = std::variant<Color, sol::table>;

	struct LuaMath final
	{
		// had to split due to Visual Studio error C1126
		static void configure1(sol::state_view& lua) noexcept;
		static void configure2(sol::state_view& lua) noexcept;
		static void configure3(sol::state_view& lua) noexcept;

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

		template<typename T, glm::qualifier Q = glm::defaultp>
		static void loadFromTable(const sol::table& tab, glm::qua<T, Q>& v) noexcept
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

		template<typename T>
		static T lerp(const T& a, const T& b, float p) noexcept
		{
			return a + (b * p);
		}

		// TODO: change all the methods to use the glm:: template vars

		template<typename T>
		static T::template value_type vecMax(const T& v) noexcept
		{
			using vec = T;
			using val = T::value_type;
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

		template<typename T>
		static T::template value_type vecMin(const T& v) noexcept
		{
			using vec = T;
			using val = T::value_type;
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

	private:

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureMat(sol::state_view& lua, std::string_view name) noexcept
		{
			using mat = T;
			using val = T::template value_type;
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
			using val = T::template value_type;
			auto usertype = configureGlmOperators<T, Ctors...>(lua, name);
			usertype["zero"] = sol::var(vec(0));
			usertype["max"] = &vecMax<vec>;
			usertype["min"] = &vecMin<vec>;
			return usertype;
		}

		template<typename T>
		static T::template value_type vecDot(const std::variant<T, sol::table>& a, const std::variant<T, sol::table>& b) noexcept
		{
			return glm::dot(tableToGlm(a), tableToGlm(b));
		}

		template<typename T>
		static T vecClamp(const std::variant<T, sol::table>& v, const std::variant<T, sol::table>& min, const std::variant<T, sol::table>& max) noexcept
		{
			return glm::clamp(tableToGlm(v), tableToGlm(min), tableToGlm(max));
		}

		template<typename T>
		static T vecLerp(const std::variant<T, sol::table>& min, const std::variant<T, sol::table>& max, float p) noexcept
		{
			return lerp(tableToGlm(min), tableToGlm(max), p);
		}

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureVec(sol::state_view& lua, std::string_view name) noexcept
		{
			using vec = T;
			using val = T::template value_type;
			auto usertype = configureGlmOperators<T, Ctors...>(lua, name);
			usertype["zero"] = sol::var(vec(0));
			usertype["norm"] = sol::resolve<vec(const vec&)>(glm::normalize);
			usertype["radians"] = sol::resolve<vec(const vec&)>(glm::radians);
			usertype["degrees"] = sol::resolve<vec(const vec&)>(glm::degrees);
			usertype["clamp"] = sol::overload(&vecClamp<vec>, sol::resolve<vec(const vec&, val, val)>(glm::clamp));
			usertype["dot"] = &vecDot<vec>;
			usertype["lerp"] = &vecLerp<vec>;
			usertype["max"] = &vecMax<vec>;
			usertype["min"] = &vecMin<vec>;
			return usertype;
		}

		template<typename T, typename... Ctors>
		static sol::usertype<T> configureGlmOperators(sol::state_view& lua, std::string_view name) noexcept
		{
			using cls = T;
			using val = T::template value_type;
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