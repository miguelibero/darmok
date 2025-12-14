#ifdef _DEBUG

#include "lua/rmlui.hpp"
#include "lua/rmlui_debug.hpp"
#include "lua/protobuf.hpp"

#include <darmok/app.hpp>
#include <darmok/rmlui.hpp>
#include <darmok/rmlui_debug.hpp>
#include <darmok/app.hpp>

namespace darmok
{
    std::reference_wrapper<RmluiDebuggerComponent> LuaRmluiDebuggerComponent::addAppComponent1(App& app)
    {
        return LuaUtils::unwrapExpected(app.addComponent<RmluiDebuggerComponent>());
    }

    std::reference_wrapper<RmluiDebuggerComponent> LuaRmluiDebuggerComponent::addAppComponent2(App& app, const Definition& def)
    {
        return LuaUtils::unwrapExpected(app.addComponent<RmluiDebuggerComponent>(def));
    }

    OptionalRef<RmluiDebuggerComponent>::std_t LuaRmluiDebuggerComponent::getAppComponent(App& app) noexcept
    {
        return app.getComponent<RmluiDebuggerComponent>();
    }

	void LuaRmluiDebuggerComponent::bind(sol::state_view& lua) noexcept
	{
		LuaProtobufBinding<RmluiDebuggerComponent::Definition>(lua, "RmluiDebuggerComponentDefinition");

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