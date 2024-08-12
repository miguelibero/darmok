#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <darmok/rmlui.hpp>
#include <darmok/string.hpp>
#include "utils.hpp"
#include "app.hpp"

namespace darmok
{
    LuaRmluiAppComponent::LuaRmluiAppComponent(RmluiAppComponent& comp) noexcept
        : _comp(comp)
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

    void LuaRmluiAppComponent::processCustomEvent(const std::vector<std::string>& params, Rml::Event& event)
    {
        auto name = params[0];

        auto arg = StringUtils::join(":", ++params.begin(), params.end());

        {
            auto itr = _funcEventListeners.find(name);
            if (itr != _funcEventListeners.end())
            {
                for (auto& func : itr->second)
                {
                    auto result = func(arg, event);
                    LuaUtils::checkResult("running custom function event", result);
                }
            }
        }
        {
            auto itr = _tabEventListeners.find(name);
            if (itr != _tabEventListeners.end())
            {
                for (auto& tab : itr->second)
                {
                    LuaUtils::callTableDelegate(tab, "on_rmlui_custom_event", "running custom table event",
                        [&arg, &event](auto& func, auto& self) {
                            return func(self, arg, event);
                        });
                }
            }
        }
    }

    LuaRmluiAppComponent& LuaRmluiAppComponent::addCustomEventListener1(const std::string& ev, const sol::table& tab) noexcept
    {
        _tabEventListeners[ev].push_back(tab);
        return *this;
    }

    bool LuaRmluiAppComponent::removeCustomEventListener1(const std::string& ev, const sol::table& tab) noexcept
    {
        auto itr1 = _tabEventListeners.find(ev);
        if (itr1 == _tabEventListeners.end())
        {
            return false;
        }
        auto itr2 = std::find(itr1->second.begin(), itr1->second.end(), tab);
        if (itr2 == itr1->second.end())
        {
            return false;
        }
        itr1->second.erase(itr2);
        return true;
    }

    bool LuaRmluiAppComponent::removeCustomEventListener2(const sol::table& tab) noexcept
    {
        auto found = false;
        for (auto& [ev, listeners] : _tabEventListeners)
        {
            auto itr = std::find(listeners.begin(), listeners.end(), tab);
            if (itr != listeners.end())
            {
                found = true;
                listeners.erase(itr);
            }
        }
        return found;
    }

    LuaRmluiAppComponent& LuaRmluiAppComponent::addCustomEventListener2(const std::string& ev, const sol::protected_function& func) noexcept
    {
        _funcEventListeners[ev].push_back(func);
        return *this;
    }

    bool LuaRmluiAppComponent::removeCustomEventListener4(const std::string& ev, const sol::protected_function& func) noexcept
    {
        auto itr1 = _funcEventListeners.find(ev);
        if (itr1 == _funcEventListeners.end())
        {
            return false;
        }
        auto itr2 = std::find(itr1->second.begin(), itr1->second.end(), func);
        if (itr2 == itr1->second.end())
        {
            return false;
        }
        itr1->second.erase(itr2);
        return true;
    }

    bool LuaRmluiAppComponent::removeCustomEventListener3(const sol::protected_function& func) noexcept
    {
        auto found = false;
        for (auto& [ev, listeners] : _funcEventListeners)
        {
            auto itr = std::find(listeners.begin(), listeners.end(), func);
            if (itr != listeners.end())
            {
                found = true;
                listeners.erase(itr);
            }
        }
        return found;
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
        return app.getReal().addComponent<LuaRmluiAppComponent>(real);
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
            "load_fallback_font", &LuaRmluiAppComponent::loadFallbackFont,
            "add_custom_event_listener", sol::overload(
                &LuaRmluiAppComponent::addCustomEventListener1,
                &LuaRmluiAppComponent::addCustomEventListener2
            ),
            "remove_custom_event_listener", sol::overload(
                &LuaRmluiAppComponent::removeCustomEventListener1,
                &LuaRmluiAppComponent::removeCustomEventListener2,
                &LuaRmluiAppComponent::removeCustomEventListener3,
                &LuaRmluiAppComponent::removeCustomEventListener4
            )
        );
    }

    LuaCustomRmluiEventListener::LuaCustomRmluiEventListener(const std::string& value, LuaRmluiAppComponent& comp) noexcept
        : _params(StringUtils::split(value, ":"))
        , _comp(comp)
    {
    }

    void LuaCustomRmluiEventListener::ProcessEvent(Rml::Event& event)
    {
        _comp.processCustomEvent(_params, event);
    }
}