#pragma once

#include "lua/lua.hpp"
#include <darmok/utils.hpp>
#include <darmok/string.hpp>
#include <darmok/expected.hpp>

#include <vector>
#include <string>
#include <optional>

#include <entt/entt.hpp>
#include <magic_enum/magic_enum.hpp>

namespace darmok
{
    class App;

    namespace LuaUtils
    {
        App& getApp(const sol::state_view& state);
        bool isArray(const sol::table& table) noexcept;
        void logError(std::string_view desc, const sol::error& err) noexcept;
        void setupErrorHandler(sol::state_view lua, sol::function& func) noexcept;
        std::optional<entt::id_type> getTypeId(const sol::object& type) noexcept;
        int deny(lua_State* L) noexcept;
        std::string prefixError(std::string_view error, std::string_view prefix) noexcept;

        template<typename T>
        T unwrapExpected(expected<T, std::string> v, std::string_view prefix = {})
        {
            if (v)
            {
				return std::move(v).value();
            }
            throw sol::error{ prefixError(v.error(), prefix) };
        }

        template<>
        void unwrapExpected(expected<void, std::string> v, std::string_view prefix);

        template<typename T = sol::object>
        expected<T, std::string> wrapObject(const sol::object& v, std::string_view prefix = {})
        {
            if (!v.is<T>())
            {
                return unexpected{ prefixError("unexpected type", prefix) };
            }
            return v.as<T>();
        }

        template<>
        expected<void, std::string> wrapObject(const sol::object& v, std::string_view prefix);

        template<>
        expected<sol::object, std::string> wrapObject(const sol::object& v, std::string_view prefix);

        template<typename T = sol::object>
        expected<T, std::string> wrapResult(const sol::protected_function_result& v, std::string_view prefix = {})
        {
            if (!v.valid())
            {
                sol::error err = v;
                return unexpected{ prefixError(err.what(), prefix)};
            }
			return wrapObject<T>(v, prefix);
        }

        template<typename T = bool>
        T checkResult(const sol::protected_function_result& v, std::string_view prefix = {}, const T& def = {})
        {
            if (!v.valid())
            {
                sol::error err = v;
                throw sol::error{ prefixError(err.what(), prefix) };
            }
            sol::object obj = v;
            if (obj.is<T>())
            {
                return obj.as<T>();
            }
            return def;
        }

        template<typename T>
        void newEnum(sol::state_view lua, std::string_view name = {}, bool stringValues = false)
        {
            if (name.empty())
            {
                name = magic_enum::enum_type_name<T>();
            }
            auto metatable = lua.create_table_with();
            auto prefix = std::string{ name } + ".";
            for (auto& [val, valueName] : StringUtils::getEnumValues<T>())
            {
                if (stringValues)
                {
                    metatable.set(valueName, prefix + std::string{ valueName });
                }
                else
                {
                    metatable.set(valueName, val);
                }
            }
            auto table = lua.create_named_table(name);
            metatable[sol::meta_function::new_index] = deny;
            metatable[sol::meta_function::index] = metatable;
            table[sol::metatable_key] = metatable;
        }        
    };

    class LuaTableDelegateDefinition final
    {
    public:
        LuaTableDelegateDefinition(std::string key, std::string desc = {}) noexcept;

        template<typename... Args>
        sol::object operator()(const sol::table& table, Args&&... args) const
        {
			auto result = tryGet<sol::object>(table, std::forward<Args>(args)...);
			return LuaUtils::unwrapExpected(std::move(result));
        }

        template<typename T, typename... Args>
        expected<T, std::string> tryGet(const sol::table& table, Args&&... args) const noexcept
        {
            auto elm = table[_key];
            if (elm.get_type() != sol::type::function)
            {
				return LuaUtils::wrapObject<T>(elm, _desc);
            }
            sol::protected_function func = elm;
            auto result = func(table, std::forward<Args>(args)...);
			return LuaUtils::wrapResult<T>(std::move(result), _desc);
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
        LuaDelegate(const sol::object& obj, std::string tableKey, std::string desc = {}) noexcept;
        operator bool() const noexcept;

        bool operator==(const sol::object& obj) const noexcept;
        bool operator!=(const sol::object& obj) const noexcept;
        bool operator==(const LuaDelegate& dlg) const noexcept;
        bool operator!=(const LuaDelegate& dlg) const noexcept;

        template<typename... Args>
        sol::object operator()(Args&&... args) const
        {
            auto result = tryGet<sol::object>(std::forward<Args>(args)...);
            return LuaUtils::unwrapExpected(std::move(result));
        }

        template<typename T, typename... Args>
        expected<T, std::string> tryGet(Args&&... args) const noexcept
        {
            auto type = _obj.get_type();
            if (type == sol::type::function)
            {
                auto func = _obj.as<sol::protected_function>();
                auto result = func(std::forward<Args>(args)...);
                return LuaUtils::wrapResult<T>(std::move(result), _desc);
            }
            if (type == sol::type::table)
            {
                auto table = _obj.as<sol::table>();
                if (sol::protected_function func = table[_tableKey])
                {
                    auto result = func(table, std::forward<Args>(args)...);
                    return LuaUtils::wrapResult<T>(std::move(result), _desc);
                }
				return unexpected<std::string>{ "function not found in table" };
            }

            return LuaUtils::wrapObject<T>(_obj, _desc);
        }
    private:
        sol::main_object _obj;
        std::string _tableKey;
        std::string _desc;
    };


}
