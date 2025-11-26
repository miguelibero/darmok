#pragma once

#ifdef _DEBUG

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class App;
    class RmluiDebuggerComponent;
    struct RmluiDebuggerComponentConfig;

    class LuaRmluiDebuggerComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        using Config = RmluiDebuggerComponentConfig;

        static std::reference_wrapper<RmluiDebuggerComponent> addAppComponent1(App& app) noexcept;
        static std::reference_wrapper<RmluiDebuggerComponent> addAppComponent2(App& app, const Config& config) noexcept;
        static OptionalRef<RmluiDebuggerComponent>::std_t getAppComponent(App& app) noexcept;
    };
}

#endif