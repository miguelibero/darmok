#pragma once

#include <sol/sol.hpp>

namespace darmok
{
	void logLuaError(const std::string& desc, const sol::error& err) noexcept;

    template<typename Callback>
    sol::protected_function_result callLuaDelegate(const sol::table& delegate, const std::string& key, const std::string& desc, Callback callback)
    {
        sol::protected_function func = delegate[key];
        auto result = callback(func);
        if (!result.valid())
        {
            logLuaError(desc, result);
        }
        return result;
    }

    template<typename Callback>
    void callLuaListeners(const std::vector<sol::table>& listeners, const std::string& key, const std::string& desc, Callback callback)
    {
        for (auto& listener : listeners)
        {
            callLuaDelegate(listener, key, desc, callback);
        }
    }
}