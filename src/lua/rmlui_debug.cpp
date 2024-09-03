#ifdef _DEBUG

#include "app.hpp"
#include "rmlui.hpp"
#include "rmlui_debug.hpp"
#include <darmok/rmlui.hpp>
#include <darmok/rmlui_debug.hpp>
#include <darmok/app.hpp>

namespace darmok
{
    RmluiDebuggerComponent& LuaRmluiDebuggerComponent::addAppComponent1(LuaApp& app) noexcept
    {
        return app.getReal().addComponent<RmluiDebuggerComponent>();
    }

    RmluiDebuggerComponent& LuaRmluiDebuggerComponent::addAppComponent2(LuaApp& app, const Config& config) noexcept
    {
        return app.getReal().addComponent<RmluiDebuggerComponent>(config);
    }

    OptionalRef<RmluiDebuggerComponent>::std_t LuaRmluiDebuggerComponent::getAppComponent(LuaApp& app) noexcept
    {
        return app.getReal().getComponent<RmluiDebuggerComponent>();
    }

	void LuaRmluiDebuggerComponent::bind(sol::state_view& lua) noexcept
	{
        lua.new_usertype<Config>("RmluiDebuggerComponentConfig",
            sol::default_constructor,
            "enableEvent", &Config::enableEvent
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