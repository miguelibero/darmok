#pragma once

#include <sol/sol.hpp>
#include <vector>
#include <string>
#include <optional>
#include <entt/entt.hpp>
#include <darmok/utils.hpp>

namespace darmok
{
    struct LuaUtils final
    {
        static void logError(const std::string& desc, const sol::error& err) noexcept;

        static bool checkResult(const std::string& desc, const sol::protected_function_result& result) noexcept;

        static std::optional<entt::id_type> getTypeId(const sol::object& type) noexcept;

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

    class LuaTableDelegateDefinition
    {
    public:
        LuaTableDelegateDefinition(const std::string& key, const std::string& desc) noexcept;

        template<typename... Args>
        bool operator()(const sol::table& table, Args&&... args) const
        {
            auto elm = table[_key];
            if (elm.get_type() != sol::type::function)
            {
                return true;
            }
            sol::protected_function func = elm;
            auto result = func(table, std::forward<Args>(args)...);
            return LuaUtils::checkResult(_desc, result);
        }

        template<typename... Args>
        void operator()(const std::vector<sol::table>& tables, Args&&... args) const
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
        LuaDelegate(const sol::protected_function& func) noexcept;
        LuaDelegate(const sol::table& table, const std::string& key) noexcept;

        bool operator==(const sol::protected_function& func) const noexcept;
        bool operator!=(const sol::protected_function& func) const noexcept;
        bool operator==(const sol::table& table) const noexcept;
        bool operator!=(const sol::table& table) const noexcept;
        bool operator==(const LuaDelegate& dlg) const noexcept;
        bool operator!=(const LuaDelegate& dlg) const noexcept;

        template<typename... Args>
        sol::protected_function_result operator()(Args&&... args) const
        {
            sol::protected_function func;
            if (_table)
            {
                auto elm = _table[_tableKey];
                if (elm.get_type() == sol::type::function)
                {
                    sol::protected_function func = elm;
                    return func(_table, std::forward<Args>(args)...);
                }
            }
            else if (_func)
            {
                return _func(std::forward<Args>(args)...);
            }
            throw std::runtime_error("empty lua delegate");
        }
    private:
        sol::protected_function _func;
        sol::table _table;
        std::string _tableKey;
    };

}