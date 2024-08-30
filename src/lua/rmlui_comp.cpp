#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <darmok/rmlui.hpp>
#include <darmok/string.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>
#include "utils.hpp"

namespace darmok
{
    LuaRmluiComponent::LuaRmluiComponent(RmluiComponent& comp, const sol::state_view& lua) noexcept
        : _comp(comp)
        , _lua(lua)
    {
    }

    void LuaRmluiComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        Rml::Factory::RegisterEventListenerInstancer(this);
    }

    void LuaRmluiComponent::shutdown() noexcept
    {
        Rml::Factory::RegisterEventListenerInstancer(nullptr);
        _cam.reset();
        _scene.reset();
    }

    Rml::EventListener* LuaRmluiComponent::InstanceEventListener(const Rml::String& value, Rml::Element* element)
    {
        return _customEventListeners.emplace_back(std::make_unique<LuaCustomRmluiEventListener>(value, *this)).get();
    }

    void LuaRmluiComponent::processCustomEvent(Rml::Event& event, const std::vector<std::string>& params)
    {
        auto name = params[0];
        auto args = _lua.create_table();
        for (size_t i = 1; i < params.size(); ++i)
        {
            args[i] = params[i];
        }
        for(auto entity : _cam->createEntityView<LuaRmluiCanvas>())
        {
            auto canvas = _scene->getComponent<LuaRmluiCanvas>(entity);
            canvas->processCustomEvent(event, name, args);
        }
    }

    RmluiComponent& LuaRmluiComponent::getReal() noexcept
    {
        return _comp;
    }

    const RmluiComponent& LuaRmluiComponent::getReal() const noexcept
    {
        return _comp;
    }

    LuaRmluiComponent& LuaRmluiComponent::addCameraComponent(Camera& cam, sol::this_state ts) noexcept
    {
        auto& real = cam.addComponent<RmluiComponent>();
        return cam.addComponent<LuaRmluiComponent>(real, ts);
    }

    void LuaRmluiComponent::loadFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path);
    }

    void LuaRmluiComponent::loadFallbackFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path, true);
    }

    void LuaRmluiComponent::bind(sol::state_view& lua) noexcept
    {
        LuaRmluiCanvas::bind(lua);

        lua.new_usertype<LuaRmluiComponent>("RmluiComponent", sol::no_constructor,
            "add_camera_component", &LuaRmluiComponent::addCameraComponent,
            "load_font", &LuaRmluiComponent::loadFont,
            "load_fallback_font", &LuaRmluiComponent::loadFallbackFont
        );
    }

    LuaCustomRmluiEventListener::LuaCustomRmluiEventListener(const std::string& value, LuaRmluiComponent& comp) noexcept
        : _params(StringUtils::split(value, ":"))
        , _comp(comp)
    {
    }

    void LuaCustomRmluiEventListener::ProcessEvent(Rml::Event& event)
    {
        _comp.processCustomEvent(event, _params);
    }
}