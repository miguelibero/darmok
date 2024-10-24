#include "utils.hpp"
#include <sstream>
#include <iostream>
#include <functional>
#include <bx/debug.h>
#include <bx/string.h>
#include <darmok/stream.hpp>
#include <darmok/utils.hpp>

namespace darmok
{
	bool LuaUtils::isArray(const sol::table& table)  noexcept
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

	entt::id_type ptrTypeId(const void* ptr) noexcept
	{
		auto addr = reinterpret_cast<std::uintptr_t>(ptr);
		auto hash = std::hash<std::uintptr_t>{}(addr);
		return static_cast<entt::id_type>(hash);
	}

	std::optional<entt::id_type> LuaUtils::getTypeId(const sol::object& type) noexcept
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
		, _obj(obj)
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