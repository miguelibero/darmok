#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <darmok/rmlui.hpp>
#include <darmok/string.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>
#include "utils.hpp"

namespace darmok
{
    LuaRmluiRenderer::LuaRmluiRenderer(RmluiRenderer& comp, const sol::state_view& lua) noexcept
        : _comp(comp)
        , _lua(lua)
    {
    }

    void LuaRmluiRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        Rml::Factory::RegisterEventListenerInstancer(this);
    }

    void LuaRmluiRenderer::shutdown() noexcept
    {
        Rml::Factory::RegisterEventListenerInstancer(nullptr);
        _cam.reset();
        _scene.reset();
    }

    Rml::EventListener* LuaRmluiRenderer::InstanceEventListener(const Rml::String& value, Rml::Element* element)
    {
        return _customEventListeners.emplace_back(std::make_unique<LuaCustomRmluiEventListener>(value, *this)).get();
    }

    void LuaRmluiRenderer::processCustomEvent(Rml::Event& event, const std::vector<std::string>& params)
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

    RmluiRenderer& LuaRmluiRenderer::getReal() noexcept
    {
        return _comp;
    }

    const RmluiRenderer& LuaRmluiRenderer::getReal() const noexcept
    {
        return _comp;
    }

    LuaRmluiRenderer& LuaRmluiRenderer::addCameraComponent(Camera& cam, sol::this_state ts) noexcept
    {
        auto& real = cam.addComponent<RmluiRenderer>();
        return cam.addComponent<LuaRmluiRenderer>(real, ts);
    }

    void LuaRmluiRenderer::loadFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path);
    }

    void LuaRmluiRenderer::loadFallbackFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path, true);
    }

    void LuaRmluiRenderer::bind(sol::state_view& lua) noexcept
    {
        LuaRmluiCanvas::bind(lua);

        lua.new_usertype<LuaRmluiRenderer>("RmluiRenderer", sol::no_constructor,
            "add_camera_component", &LuaRmluiRenderer::addCameraComponent,
            "load_font", &LuaRmluiRenderer::loadFont,
            "load_fallback_font", &LuaRmluiRenderer::loadFallbackFont
        );
    }

    LuaCustomRmluiEventListener::LuaCustomRmluiEventListener(const std::string& value, LuaRmluiRenderer& comp) noexcept
        : _params(StringUtils::split(value, ":"))
        , _comp(comp)
    {
    }

    void LuaCustomRmluiEventListener::ProcessEvent(Rml::Event& event)
    {
        _comp.processCustomEvent(event, _params);
    }
}