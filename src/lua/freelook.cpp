#include "freelook.hpp"
#include "scene.hpp"
#include <darmok/camera.hpp>
#include <darmok/freelook.hpp>

namespace darmok
{
    FreelookController& LuaFreelookController::LuaFreelookController::addSceneComponent1(LuaScene& scene, Camera& cam) noexcept
    {
        return scene.getReal()->addSceneComponent<FreelookController>(cam);
    }

    FreelookController& LuaFreelookController::addSceneComponent2(LuaScene& scene, Camera& cam, const Config& config) noexcept
    {
        return scene.getReal()->addSceneComponent<FreelookController>(cam, config);
    }

    void LuaFreelookController::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<FreelookController>("FreelookController", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<FreelookController>::value),
            "add_scene_component", sol::overload(
                &LuaFreelookController::addSceneComponent1,
                &LuaFreelookController::addSceneComponent2
            ),
            "enabled", sol::property(&FreelookController::isEnabled, &FreelookController::setEnabled)
        );
    }

}