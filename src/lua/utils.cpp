#include "lua/utils.hpp"
#include <sstream>
#include <iostream>
#include <functional>
#include <bx/debug.h>
#include <bx/string.h>
#include <darmok/stream.hpp>
#include <darmok/utils.hpp>
#include <darmok/app.hpp>

namespace darmok
{
	namespace LuaUtils
	{
		App& getApp(const sol::state_view& state)
		{
			sol::object luaApp = state["app"] ;
			if (!luaApp.is<App>())
			{
				throw sol::error{ "missing app global" };
			}
			return luaApp.as<App>();
		}

		bool isArray(const sol::table& table)  noexcept
		{
			if (table.empty())
			{
				return true;
			}
			auto key = (*table.cbegin()).first;
			if (key.get_type() != sol::type::number)
			{
				return false;
			}
			return key.as<int>() == 1;
		}

		void logError(std::string_view desc, const sol::error& err) noexcept
		{
			std::stringstream ss;
			ss << "recovered lua error " << desc << ":" << std::endl;
			ss << err.what() << std::endl;

			StreamUtils::log(ss.str(), true);
		}

		void setupErrorHandler(sol::state_view lua, sol::function& func) noexcept
		{
			func.set_error_handler(lua.safe_script("return function(err) return debug.traceback(err, 2) end"));
		}

		entt::id_type ptrTypeId(const void* ptr) noexcept
		{
			auto addr = reinterpret_cast<std::uintptr_t>(ptr);
			auto hash = std::hash<std::uintptr_t>{}(addr);
			return static_cast<entt::id_type>(hash);
		}

		std::optional<entt::id_type> getTypeId(const sol::object& type) noexcept
		{
			auto luaType = type.get_type();
			if (luaType == sol::type::number)
			{
				return type.template as<entt::id_type>();
			}
			if (type.is<sol::table>())
			{
				auto table = type.as<sol::table>();
				auto cls = table["class"];
				if (cls.get_type() == sol::type::table)
				{
					table = cls;
				}
				auto entry = table["type_id"];
				auto entryType = entry.get_type();
				if (entryType == sol::type::number)
				{
					return entry.get<entt::id_type>();
				}
				if (entry.is<sol::reference>())
				{
					return ptrTypeId(entry.get<sol::reference>().pointer());
				}
				return ptrTypeId(table.pointer());
			}
			return std::nullopt;
		}

		int deny(lua_State* L) noexcept
		{
			return luaL_error(L, "operation not allowed");
		}

		std::string prefixError(std::string_view error, std::string_view prefix) noexcept
		{
			if (prefix.empty())
			{
				return std::string{ error };
			}
			return std::string{ prefix } + ": " + std::string{ error };
		}

		template<>
		void unwrapExpected(expected<void, std::string> v, std::string_view prefix)
		{
			if (v)
			{
				return;
			}
			if (prefix.empty())
			{
				throw sol::error{ std::move(v).error() };
			}
			throw sol::error{ std::string{prefix} + v.error() };
		}

		template<>
		expected<void, std::string> wrapObject(const sol::object& v, std::string_view prefix)
		{
			return {};
		}

		template<>
		expected<sol::object, std::string> wrapObject(const sol::object& v, std::string_view prefix)
		{
			return v;
		}
	}

	LuaTableDelegateDefinition::LuaTableDelegateDefinition(std::string key, std::string desc) noexcept
		: _key{ std::move(key) }
		, _desc{ std::move(desc) }
	{
	}

	LuaDelegate::LuaDelegate(const sol::object& obj, std::string tableKey, std::string desc) noexcept
		: _obj{ obj }
		, _tableKey{ tableKey.empty() ? "__call" : std::move(tableKey) }
		, _desc{ std::move(desc) }
	{
	}

	LuaDelegate::operator bool() const noexcept
	{
		auto type = _obj.get_type();
		return type == sol::type::function || type == sol::type::table;
	}

	bool LuaDelegate::operator==(const sol::object& obj) const noexcept
	{
		return _obj == obj;
	}

	bool LuaDelegate::operator!=(const sol::object& obj) const noexcept
	{
		return !operator==(obj);
	}

	bool LuaDelegate::operator==(const LuaDelegate& dlg) const noexcept
	{
		return _obj == dlg._obj && _tableKey == dlg._tableKey;
	}

	bool LuaDelegate::operator!=(const LuaDelegate& dlg) const noexcept
	{
		return !operator==(dlg);
	}
}
