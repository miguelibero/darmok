#pragma once

#include <sol/sol.hpp>
#include <vector>
#include <string>

namespace darmok
{
	void logLuaError(const std::string& desc, const sol::error& err) noexcept;

    bool checkLuaResult(const std::string& desc, const sol::protected_function_result& result) noexcept;

    template<typename Callback>
    sol::protected_function_result callLuaTableDelegate(const sol::table& delegate, const std::string& key, const std::string& desc, Callback callback)
    {
        sol::protected_function func = delegate[key];
        auto result = callback(func);
        checkLuaResult(desc, result);
        return result;
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
    void newLuaEnumFunc(sol::state_view& lua, std::string_view name, T count, const std::string&(*func)(T))
    {
        std::vector<std::pair<std::string_view, T>> values;
        auto size = to_underlying(count);
        for (int i = 0; i < size; i++)
        {
            auto elm = (T)i;
            values.emplace_back(func(elm), elm);
        }
        newLuaEnumVector(lua, name, values);
    }

    template<typename T>
    void newLuaEnumVector(sol::state_view& lua, std::string_view name, const std::vector<std::pair<std::string_view, T>>& values)
    {
        lua.new_enum<T>(name,
            std::initializer_list<std::pair<std::string_view, T>>(&values.front(), &values.front() + values.size()));
    }
}