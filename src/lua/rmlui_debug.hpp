#pragma once

#ifdef _DEBUG

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class LuaApp;
    class RmluiDebuggerComponent;
    struct RmluiDebuggerComponentConfig;

    class LuaRmluiDebuggerComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        using Config = RmluiDebuggerComponentConfig;

        static RmluiDebuggerComponent& addAppComponent1(LuaApp& app) noexcept;
        static RmluiDebuggerComponent& addAppComponent2(LuaApp& app, const Config& config) noexcept;
        static OptionalRef<RmluiDebuggerComponent>::std_t getAppComponent(LuaApp& app) noexcept;
    };
}

#endif