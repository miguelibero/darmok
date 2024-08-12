#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <darmok/rmlui.hpp>
#include <darmok/string.hpp>
#include "utils.hpp"
#include "app.hpp"

namespace darmok
{
    LuaRmluiAppComponent::LuaRmluiAppComponent(RmluiAppComponent& comp, const sol::state_view& lua) noexcept
        : _comp(comp)
        , _lua(lua)
    {
    }

    void LuaRmluiAppComponent::init(App& app) noexcept
    {
        Rml::Factory::RegisterEventListenerInstancer(this);
    }

    void LuaRmluiAppComponent::shutdown() noexcept
    {
        Rml::Factory::RegisterEventListenerInstancer(nullptr);
        _views.clear();
    }

    Rml::EventListener* LuaRmluiAppComponent::InstanceEventListener(const Rml::String& value, Rml::Element* element)
    {
        return _customEventListeners.emplace_back(std::make_unique<LuaCustomRmluiEventListener>(value, *this)).get();
    }

    void LuaRmluiAppComponent::processCustomEvent(Rml::Event& event, const std::vector<std::string>& params)
    {
        auto name = params[0];
        auto args = _lua.create_table();
        for (size_t i = 1; i < params.size(); ++i)
        {
            args[i] = params[i];
        }
        for (auto& [viewName, view] : _views)
        {
            view.processCustomEvent(event, name, args);
        }
    }

    RmluiAppComponent& LuaRmluiAppComponent::getReal() noexcept
    {
        return _comp;
    }

    const RmluiAppComponent& LuaRmluiAppComponent::getReal() const noexcept
    {
        return _comp;
    }

    LuaRmluiView& LuaRmluiAppComponent::getView1() noexcept
    {
        return getView2("");
    }

    LuaRmluiView& LuaRmluiAppComponent::getView2(const std::string& name) noexcept
    {
        auto itr = _views.find(name);
        if (itr == _views.end())
        {
            itr = _views.emplace(name, _comp.getView(name)).first;
        }
        return itr->second;
    }

    bool LuaRmluiAppComponent::hasView(const std::string& name) const noexcept
    {
        return _views.contains(name);
    }

    bool LuaRmluiAppComponent::removeView(const std::string& name) noexcept
    {
        auto itr = _views.find(name);
        if (itr != _views.end())
        {
            _views.erase(itr);
        }
        return _comp.removeView(name);
    }

    LuaRmluiAppComponent& LuaRmluiAppComponent::addAppComponent(LuaApp& app, sol::this_state ts) noexcept
    {
        auto& real = app.getReal().addComponent<RmluiAppComponent>();
        return app.getReal().addComponent<LuaRmluiAppComponent>(real, ts);
    }

    void LuaRmluiAppComponent::loadFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path);
    }

    void LuaRmluiAppComponent::loadFallbackFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path, true);
    }

    void LuaRmluiAppComponent::bind(sol::state_view& lua) noexcept
    {
        LuaRmluiView::bind(lua);

        // TODO: probably will remove this dependency
        // Rml::Lua::Initialise(lua.lua_state());

        lua.new_usertype<LuaRmluiAppComponent>("RmluiAppComponent", sol::no_constructor,
            "view", sol::property(&LuaRmluiAppComponent::getView1),
            "get_view", &LuaRmluiAppComponent::getView2,
            "has_view", &LuaRmluiAppComponent::hasView,
            "remove_view", &LuaRmluiAppComponent::removeView,
            "add_app_component", &LuaRmluiAppComponent::addAppComponent,
            "load_font", &LuaRmluiAppComponent::loadFont,
            "load_fallback_font", &LuaRmluiAppComponent::loadFallbackFont
        );
    }

    LuaCustomRmluiEventListener::LuaCustomRmluiEventListener(const std::string& value, LuaRmluiAppComponent& comp) noexcept
        : _params(StringUtils::split(value, ":"))
        , _comp(comp)
    {
    }

    void LuaCustomRmluiEventListener::ProcessEvent(Rml::Event& event)
    {
        _comp.processCustomEvent(event, _params);
    }
}