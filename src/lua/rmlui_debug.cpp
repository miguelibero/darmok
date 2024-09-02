#ifdef _DEBUG

#include "scene.hpp"
#include "rmlui.hpp"
#include "rmlui_debug.hpp"
#include <darmok/rmlui.hpp>
#include <darmok/rmlui_debug.hpp>
#include <darmok/scene.hpp>

namespace darmok
{
    RmluiDebuggerComponent& LuaRmluiDebuggerComponent::addSceneComponent1(LuaScene& scene) noexcept
    {
        return scene.getReal()->addSceneComponent<RmluiDebuggerComponent>();
    }

    RmluiDebuggerComponent& LuaRmluiDebuggerComponent::addSceneComponent2(LuaScene& scene, const Config& config) noexcept
    {
        return scene.getReal()->addSceneComponent<RmluiDebuggerComponent>(config);
    }

    OptionalRef<RmluiDebuggerComponent>::std_t LuaRmluiDebuggerComponent::getSceneComponent(LuaScene& scene) noexcept
    {
        return scene.getReal()->getSceneComponent<RmluiDebuggerComponent>();
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
            "get_scene_component", &LuaRmluiDebuggerComponent::getSceneComponent,
            "add_scene_component", sol::overload(
                &LuaRmluiDebuggerComponent::addSceneComponent1,
                &LuaRmluiDebuggerComponent::addSceneComponent2
            )
        );
	}
}

#endif