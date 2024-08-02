#pragma once

#include <sol/sol.hpp>
#include <vector>
#include <string>

namespace darmok
{
    struct LuaUtils final
    {
        static void logError(const std::string& desc, const sol::error& err) noexcept;

        static bool checkResult(const std::string& desc, const sol::protected_function_result& result) noexcept;

        template<typename Callback>
        static bool callTableDelegate(const sol::table& delegate, const std::string& key, const std::string& desc, Callback callback)
        {
            auto elm = delegate[key];
            if (!elm.is<sol::function>())
            {
                return true;
            }
            sol::protected_function func = elm;
            auto result = callback(func);
            return checkResult(desc, result);
        }

        template<typename Callback>
        static void callTableListeners(const std::vector<sol::table>& listeners, const std::string& key, const std::string& desc, Callback callback)
        {
            for (auto& listener : listeners)
            {
                callTableDelegate(listener, key, desc, callback);
            }
        }

        template<typename T>
        static void newEnumFunc(sol::state_view& lua, std::string_view name, T count, const std::string& (*func)(T), bool string = false)
        {
            std::vector<std::pair<std::string_view, T>> values;
            auto size = to_underlying(count);
            for (int i = 0; i < size; i++)
            {
                auto elm = (T)i;
                values.emplace_back(func(elm), elm);
            }
            newEnumVector(lua, name, values, string);
        }

        static int deny(lua_State* L);

        template<typename T>
        static void newEnumVector(sol::state_view& lua, std::string_view name, const std::vector<std::pair<std::string_view, T>>& values, bool string)
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
            metatable[sol::meta_function::new_index] = LuaUtils::deny;
            metatable[sol::meta_function::index] = metatable;
            table[sol::metatable_key] = metatable;
        }
    };
}