#ifdef _DEBUG

#include "lua/rmlui.hpp"
#include "lua/rmlui_debug.hpp"

#include <darmok/app.hpp>
#include <darmok/rmlui.hpp>
#include <darmok/rmlui_debug.hpp>
#include <darmok/app.hpp>

namespace darmok
{
    std::reference_wrapper<RmluiDebuggerComponent> LuaRmluiDebuggerComponent::addAppComponent1(App& app) noexcept
    {
        return LuaUtils::unwrapExpected(app.addComponent<RmluiDebuggerComponent>());
    }

    std::reference_wrapper<RmluiDebuggerComponent> LuaRmluiDebuggerComponent::addAppComponent2(App& app, const Config& config) noexcept
    {
        return LuaUtils::unwrapExpected(app.addComponent<RmluiDebuggerComponent>(config));
    }

    OptionalRef<RmluiDebuggerComponent>::std_t LuaRmluiDebuggerComponent::getAppComponent(App& app) noexcept
    {
        return app.getComponent<RmluiDebuggerComponent>();
    }

	void LuaRmluiDebuggerComponent::bind(sol::state_view& lua) noexcept
	{
        lua.new_usertype<Config>("RmluiDebuggerComponentConfig",
            sol::default_constructor,
            "enable_events", &Config::enableEvents
        );
        lua.new_usertype<RmluiDebuggerComponent>("RmluiDebuggerComponent", sol::no_constructor,
            "toggle", &RmluiDebuggerComponent::toggle,
            "enabled", sol::property(&RmluiDebuggerComponent::isEnabled),
            "get_app_component", &LuaRmluiDebuggerComponent::getAppComponent,
            "add_app_component", sol::overload(
                &LuaRmluiDebuggerComponent::addAppComponent1,
                &LuaRmluiDebuggerComponent::addAppComponent2
            )
        );
	}
}

#endif