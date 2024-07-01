#include "freelook.hpp"
#include "scene.hpp"
#include "transform.hpp"
#include <darmok/freelook.hpp>

namespace darmok
{
    LuaFreelookController::LuaFreelookController(FreelookController& ctrl) noexcept
        : _ctrl(ctrl)
    {
    }

    void LuaFreelookController::setEnabled(bool enabled) noexcept
    {
        _ctrl->setEnabled(enabled);
    }

    bool LuaFreelookController::getEnabled() const noexcept
    {
        return _ctrl->isEnabled();
    }

    LuaFreelookController LuaFreelookController::LuaFreelookController::addSceneComponent1(LuaScene& scene, LuaTransform& trans) noexcept
    {
        return scene.getReal()->addSceneComponent<FreelookController>(trans.getReal());
    }

    LuaFreelookController LuaFreelookController::addSceneComponent2(LuaScene& scene, LuaTransform& trans, const Config& config) noexcept
    {
        return scene.getReal()->addSceneComponent<FreelookController>(trans.getReal(), config);
    }

    void LuaFreelookController::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaFreelookController>("FreelookController", sol::no_constructor,
            "add_scene_component", sol::overload(
                &LuaFreelookController::addSceneComponent1,
                &LuaFreelookController::addSceneComponent2
            ),
            "enabled", sol::property(&LuaFreelookController::getEnabled, &LuaFreelookController::setEnabled)
        );
    }

}