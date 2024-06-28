#pragma once

#include <sol/sol.hpp>

namespace darmok
{
	void recoveredLuaError(const std::string& desc, const sol::error& err) noexcept;

    template<typename Callback>
    void callLuaListeners(const std::vector<sol::table>& listeners, const std::string& key, const std::string& desc, Callback callback)
    {
        for (auto& listener : listeners)
        {
            sol::protected_function func = listener[key];
            if (!func)
            {
                continue;
            }
            auto result = callback(func);
            if (!result.valid())
            {
                recoveredLuaError(desc, result);
            }
        }
    }
}