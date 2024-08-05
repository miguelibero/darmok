#ifdef _DEBUG

#include "app.hpp"
#include "rmlui_debug.hpp"
#include <darmok/rmlui.hpp>
#include <darmok/rmlui_debug.hpp>

namespace darmok
{
    RmluiDebuggerAppComponent& LuaRmluiDebuggerAppComponent::addAppComponent1(LuaApp& app, RmluiAppComponent& comp) noexcept
    {
        return app.getReal().addComponent<RmluiDebuggerAppComponent>(comp);
    }

    RmluiDebuggerAppComponent& LuaRmluiDebuggerAppComponent::addAppComponent2(LuaApp& app, RmluiAppComponent& comp, const Config& config) noexcept
    {
        return app.getReal().addComponent<RmluiDebuggerAppComponent>(comp, config);
    }

	void LuaRmluiDebuggerAppComponent::bind(sol::state_view& lua) noexcept
	{
        lua.new_usertype<RmluiDebuggerAppComponentConfig>("GuiDebuggerAppComponentConfig",
            sol::default_constructor,
            "enableEvent", &RmluiDebuggerAppComponentConfig::enableEvent
        );
        lua.new_usertype<RmluiDebuggerAppComponent>("GuiDebuggerAppComponent", sol::no_constructor,
            "toggle", &RmluiDebuggerAppComponent::toggle,
            "enabled", sol::property(&RmluiDebuggerAppComponent::isEnabled),
            "add_app_component", sol::overload(
                &LuaRmluiDebuggerAppComponent::addAppComponent1,
                &LuaRmluiDebuggerAppComponent::addAppComponent2
            )
        );
	}
}

#endif