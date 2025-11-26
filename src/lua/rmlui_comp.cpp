#include "lua/rmlui.hpp"
#include "lua/utils.hpp"
#include "lua/camera.hpp"
#include "lua/scene.hpp"

#include <darmok/rmlui.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>

#include <RmlUi/Core.h>

namespace darmok
{
    std::reference_wrapper<RmluiSceneComponent> LuaRmluiSceneComponent::addSceneComponent(Scene& scene) noexcept
    {
        return LuaUtils::unwrapExpected(scene.addSceneComponent<RmluiSceneComponent>());
    }

    OptionalRef<RmluiSceneComponent>::std_t LuaRmluiSceneComponent::getSceneComponent(Scene& scene) noexcept
    {
        return scene.getSceneComponent<RmluiSceneComponent>();
    }

    void LuaRmluiSceneComponent::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<RmluiSceneComponent>("RmluiSceneComponent", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<RmluiSceneComponent>::value),
            "add_scene_component", &LuaRmluiSceneComponent::addSceneComponent,
            "get_scene_component", &LuaRmluiSceneComponent::getSceneComponent
        );
    }

    std::reference_wrapper<RmluiRenderer> LuaRmluiRenderer::addCameraComponent(Camera& cam) noexcept
    {
        return LuaUtils::unwrapExpected(cam.addComponent<RmluiRenderer>());
    }

    OptionalRef<RmluiRenderer>::std_t LuaRmluiRenderer::getCameraComponent(Camera& cam) noexcept
    {
        return cam.getComponent<RmluiRenderer>();
    }

    void LuaRmluiRenderer::loadFont1(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path);
    }

    void LuaRmluiRenderer::loadFont2(const std::string& path, bool fallback) noexcept
    {
        Rml::LoadFontFace(path, fallback);
    }

    void LuaRmluiRenderer::bind(sol::state_view& lua) noexcept
    {
        LuaRmluiCanvas::bind(lua);
        LuaRmluiEvent::bind(lua);
        LuaRmluiElement::bind(lua);
        LuaRmluiElementDocument::bind(lua);
        LuaRmluiSceneComponent::bind(lua);

        lua.new_usertype<LuaRmluiRenderer>("RmluiRenderer", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<LuaRmluiRenderer>::value),
            "add_camera_component", &LuaRmluiRenderer::addCameraComponent,
            "get_camera_component", &LuaRmluiRenderer::getCameraComponent,
            "load_font", sol::overload(
                &LuaRmluiRenderer::loadFont1,
                &LuaRmluiRenderer::loadFont2
            )
        );
    }
}