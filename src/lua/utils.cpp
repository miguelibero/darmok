#include "utils.hpp"
#include <sstream>
#include <iostream>
#include <bx/debug.h>
#include <bx/string.h>
#include <darmok/stream.hpp>
#include <darmok/utils.hpp>

namespace darmok
{
    void LuaUtils::logError(const std::string& desc, const sol::error& err) noexcept
	{
		std::stringstream ss;
		ss << "recovered lua error " << desc << ":" << std::endl;
		ss << err.what() << std::endl;

		StreamUtils::logDebug(ss.str(), true);
	}

	bool LuaUtils::checkResult(const std::string& desc, const sol::protected_function_result& result) noexcept
	{
		if (!result.valid())
		{
			logError(desc, result);
			return false;
		}
		sol::object obj = result;
		if (obj.get_type() == sol::type::boolean)
		{
			return obj.as<bool>();
		}
		return obj != sol::nil;
	}

	std::optional<entt::id_type> LuaUtils::getTypeId(const sol::object& type) noexcept
	{
		auto luaType = type.get_type();
		if (luaType == sol::type::number)
		{
			return type.template as<entt::id_type>();
		}
		if (luaType == sol::type::table)
		{
			auto entry = type.as<sol::table>()["type_id"];
			if (entry)
			{
				return entry.get<entt::id_type>();
			}
		}
		return std::nullopt;
	}

	int LuaUtils::deny(lua_State* L)
	{
		return luaL_error(L, "operation not allowed");
	}

	LuaTableDelegateDefinition::LuaTableDelegateDefinition(const std::string& key, const std::string& desc) noexcept
		: _key(key)
		, _desc(desc)
	{
	}

	LuaDelegate::LuaDelegate(const sol::object& obj, const std::string& tableKey) noexcept
		: _tableKey(tableKey.empty() ? "__call" : tableKey)
	{
		auto type = obj.get_type();
		if (type == sol::type::function)
		{
			_func = obj;
		}
		else if (type == sol::type::table)
		{
			_table = obj;
		}
	}

	LuaDelegate::operator bool() const noexcept
	{
		return _func || _table;
	}

	bool LuaDelegate::operator==(const sol::object& obj) const noexcept
	{
		return _func == obj || _table == obj;
	}

	bool LuaDelegate::operator!=(const sol::object& obj) const noexcept
	{
		return !operator==(obj);
	}

	bool LuaDelegate::operator==(const LuaDelegate& dlg) const noexcept
	{
		return _func == dlg._func && _table == dlg._table && _tableKey == dlg._tableKey;
	}

	bool LuaDelegate::operator!=(const LuaDelegate& dlg) const noexcept
	{
		return !operator==(dlg);
	}
}