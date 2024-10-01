#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <darmok/rmlui.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>
#include "utils.hpp"
#include "camera.hpp"
#include "scene.hpp"

namespace darmok
{
    RmluiSceneComponent& LuaRmluiSceneComponent::addSceneComponent(LuaScene& scene) noexcept
    {
        return scene.getReal()->addSceneComponent<RmluiSceneComponent>();
    }

    OptionalRef<RmluiSceneComponent>::std_t LuaRmluiSceneComponent::getSceneComponent(LuaScene& scene) noexcept
    {
        return scene.getReal()->getSceneComponent<RmluiSceneComponent>();
    }

    void LuaRmluiSceneComponent::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<RmluiSceneComponent>("RmluiSceneComponent", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<RmluiSceneComponent>::value),
            "add_scene_component", &LuaRmluiSceneComponent::addSceneComponent,
            "get_scene_component", &LuaRmluiSceneComponent::getSceneComponent
        );
    }

    RmluiRenderer& LuaRmluiRenderer::addCameraComponent(Camera& cam) noexcept
    {
        return cam.addComponent<RmluiRenderer>();
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