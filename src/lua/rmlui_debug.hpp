#pragma once

#ifdef _DEBUG

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class App;
    class RmluiDebuggerComponent;

    namespace protobuf
    {
        class RmluiDebuggerComponent;
	}

    class LuaRmluiDebuggerComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        using Definition = protobuf::RmluiDebuggerComponent;

        static std::reference_wrapper<RmluiDebuggerComponent> addAppComponent1(App& app);
        static std::reference_wrapper<RmluiDebuggerComponent> addAppComponent2(App& app, const Definition& def);
        static OptionalRef<RmluiDebuggerComponent>::std_t getAppComponent(App& app) noexcept;
    };
}

#endif