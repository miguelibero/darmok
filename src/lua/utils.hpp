#pragma once

#include "lua.hpp"
#include <darmok/utils.hpp>

#include <vector>
#include <string>
#include <optional>


#include <entt/entt.hpp>
#include <magic_enum/magic_enum.hpp>

namespace darmok
{
    struct LuaUtils final
    {
        static bool isArray(const sol::table& table)  noexcept;

        static void logError(const std::string& desc, const sol::error& err) noexcept;

        static bool checkResult(const std::string& desc, const sol::protected_function_result& result) noexcept;

        static std::optional<entt::id_type> getTypeId(const sol::object& type) noexcept;

        template<typename T>
        static void newEnum(sol::state_view& lua, bool string = false)
        {
            return newEnum(lua, magic_enum::enum_type_name<T>(), string);
        }

        template<typename T>
        static void newEnum(sol::state_view& lua, std::string_view name, bool string = false)
        {
            auto metatable = lua.create_table_with();
            auto prefix = std::string(name) + ".";
            for (auto val : magic_enum::enum_values<T>())
            {
                auto valueName = magic_enum::enum_name(val);
                if (string)
                {
                    metatable.set(valueName, prefix + valueName);
                }
                else
                {
                    metatable.set(valueName, val);
                }
            }
            auto table = lua.create_named_table(name);
            metatable[sol::meta_function::new_index] = LuaUtils::deny;
            metatable[sol::meta_function::index] = metatable;
            table[sol::metatable_key] = metatable;
        }

        static int deny(lua_State* L);

    };

    class LuaTableDelegateDefinition
    {
    public:
        LuaTableDelegateDefinition(const std::string& key, const std::string& desc) noexcept;

        template<typename... Args>
        sol::object operator()(const sol::table& table, Args&&... args) const
        {
            auto elm = table[_key];
            if (elm.get_type() != sol::type::function)
            {
                return sol::object(table.lua_state(), true);
            }
            sol::protected_function func = elm;
            auto result = func(table, std::forward<Args>(args)...);
            LuaUtils::checkResult(_desc, result);
            return result;
        }

        template<typename T, typename... Args>
        void operator()(const std::vector<T>& tables, Args&&... args) const
        {
            for (auto& table : tables)
            {
                operator()(table, std::forward<Args>(args)...);
            }
        }
    private:
        std::string _key;
        std::string _desc;
    };

    class LuaDelegate final
    {
    public:
        LuaDelegate(const sol::object& obj, const std::string& tableKey) noexcept;
        operator bool() const noexcept;

        bool operator==(const sol::object& obj) const noexcept;
        bool operator!=(const sol::object& obj) const noexcept;
        bool operator==(const LuaDelegate& dlg) const noexcept;
        bool operator!=(const LuaDelegate& dlg) const noexcept;

        template<typename... Args>
        sol::protected_function_result operator()(Args&&... args) const noexcept
        {
            auto type = _obj.get_type();
            if (type == sol::type::function)
            {
                sol::protected_function func = _obj;
                return func(std::forward<Args>(args)...);
            }
            else if (type == sol::type::table)
            {
                auto table = _obj.as<sol::table>();
                if (sol::protected_function func = table[_tableKey])
                {
                    return func(table, std::forward<Args>(args)...);
                }
            }
            return sol::protected_function_result(_obj.lua_state(), -1, 0, 0, sol::call_status::runtime);
        }
    private:
        sol::main_object _obj;
        std::string _tableKey;
    };


}