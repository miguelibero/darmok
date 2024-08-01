#pragma once

#include <sol/sol.hpp>
#include <vector>
#include <string>

namespace darmok
{
	void logLuaError(const std::string& desc, const sol::error& err) noexcept;

    bool checkLuaResult(const std::string& desc, const sol::protected_function_result& result) noexcept;

    template<typename Callback>
    bool callLuaTableDelegate(const sol::table& delegate, const std::string& key, const std::string& desc, Callback callback)
    {
        auto elm = delegate[key];
        if (!elm.is<sol::function>())
        {
            return true;
        }
        sol::protected_function func = elm;
        auto result = callback(func);
        return checkLuaResult(desc, result);
    }

    template<typename Callback>
    void callLuaTableListeners(const std::vector<sol::table>& listeners, const std::string& key, const std::string& desc, Callback callback)
    {
        for (auto& listener : listeners)
        {
            callLuaTableDelegate(listener, key, desc, callback);
        }
    }

    template<typename T>
    void newLuaEnumFunc(sol::state_view& lua, std::string_view name, T count, const std::string&(*func)(T), bool string = false)
    {
        std::vector<std::pair<std::string_view, T>> values;
        auto size = to_underlying(count);
        for (int i = 0; i < size; i++)
        {
            auto elm = (T)i;
            values.emplace_back(func(elm), elm);
        }
        newLuaEnumVector(lua, name, values, string);
    }

    int luaDeny(lua_State* L);

    template<typename T>
    void newLuaEnumVector(sol::state_view& lua, std::string_view name, const std::vector<std::pair<std::string_view, T>>& values, bool string)
    {
        if (!string)
        {
            lua.new_enum<T>(name,
                std::initializer_list<std::pair<std::string_view, T>>(
                    &values.front(), &values.front() + values.size()));
            return;
        }
    
        auto metatable = lua.create_table_with();
        auto prefix = std::string(name) + ".";
        for (auto& elm : values)
        {
            std::string valueName(elm.first);
            metatable[valueName] = prefix + valueName;
        }

        auto table = lua.create_named_table(name);
        metatable[sol::meta_function::new_index] = luaDeny;
        metatable[sol::meta_function::index] = metatable;
        table[sol::metatable_key] = metatable;
    }
}