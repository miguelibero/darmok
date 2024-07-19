#pragma once

#include <sol/sol.hpp>

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
}