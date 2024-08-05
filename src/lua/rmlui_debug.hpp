#pragma once

#ifdef _DEBUG

#include <sol/sol.hpp>

namespace darmok
{
    class LuaApp;
    class RmluiAppComponent;
    class RmluiDebuggerAppComponent;
    struct RmluiDebuggerAppComponentConfig;

    class LuaRmluiDebuggerAppComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        using Config = RmluiDebuggerAppComponentConfig;

        static RmluiDebuggerAppComponent& addAppComponent1(LuaApp& app, RmluiAppComponent& comp) noexcept;
        static RmluiDebuggerAppComponent& addAppComponent2(LuaApp& app, RmluiAppComponent& comp, const Config& config) noexcept;
    };
}

#endif