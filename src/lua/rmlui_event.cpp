#include "rmlui.hpp"
#include "../rmlui.hpp"
#include "utils.hpp"

namespace darmok
{
    void LuaRmluiEvent::setRmlVariant(Rml::Variant& variant, sol::object obj) noexcept
    {
        switch (obj.get_type())
        {
            case sol::type::none:
            case sol::type::nil:
                variant.Clear();
                break;
            case sol::type::boolean:
                variant = obj.as<bool>();
                break;
            case sol::type::number:
                variant = obj.as<double>();
                break;
            case sol::type::string:
                variant = obj.as<std::string>();
                break;
            case sol::type::userdata:
            {
                if (obj.is<glm::vec2>())
                {
                    variant = RmluiUtils::convert(obj.as<glm::vec2>());
                }
                if (obj.is<glm::vec3>())
                {
                    variant = RmluiUtils::convert(obj.as<glm::vec3>());
                }
                if (obj.is<glm::vec4>())
                {
                    variant = RmluiUtils::convert(obj.as<glm::vec4>());
                }
                if (obj.is<Color>())
                {
                    variant = RmluiUtils::convert(obj.as<Color>());
                }
                break;
            }
        }
    }

    sol::object LuaRmluiEvent::getRmlVariant(lua_State* lua, const Rml::Variant& variant) noexcept
    {
        switch (variant.GetType())
        {
        case Rml::Variant::BOOL:
            return sol::make_object(lua, variant.Get<bool>());
        case Rml::Variant::BYTE:
            return sol::make_object(lua, variant.Get<uint8_t>());
        case Rml::Variant::CHAR:
            return sol::make_object(lua, variant.Get<char>());
        case Rml::Variant::FLOAT:
            return sol::make_object(lua, variant.Get<float>());
        case Rml::Variant::DOUBLE:
            return sol::make_object(lua, variant.Get<double>());
        case Rml::Variant::INT:
            return sol::make_object(lua, variant.Get<int>());
        case Rml::Variant::INT64:
            return sol::make_object(lua, variant.Get<int64_t>());
        case Rml::Variant::UINT:
            return sol::make_object(lua, variant.Get<unsigned int>());
        case Rml::Variant::UINT64:
            return sol::make_object(lua, variant.Get<uint64_t>());
        case Rml::Variant::STRING:
            return sol::make_object(lua, variant.Get<Rml::String>());
        case Rml::Variant::VECTOR2:
            return sol::make_object(lua, RmluiUtils::convert(variant.Get<Rml::Vector2f>()));
        case Rml::Variant::VECTOR3:
            return sol::make_object(lua, RmluiUtils::convert(variant.Get<Rml::Vector3f>()));
        case Rml::Variant::VECTOR4:
            return sol::make_object(lua, RmluiUtils::convert(variant.Get<Rml::Vector4f>()));
        case Rml::Variant::COLOURF:
            return sol::make_object(lua, RmluiUtils::convert(variant.Get<Rml::Colourf>()));
        case Rml::Variant::COLOURB:
            return sol::make_object(lua, RmluiUtils::convert(variant.Get<Rml::Colourb>()));
        default:
            return sol::nil;
        }
        /*
        * TODO: check what to do with these
            SCRIPTINTERFACE = 'p',
            TRANSFORMPTR = 't',
            TRANSITIONLIST = 'T',
            ANIMATIONLIST = 'A',
            DECORATORSPTR = 'D',
            FONTEFFECTSPTR = 'F',
        */
    }

    glm::vec2 LuaRmluiEvent::getUnprojectedMouseScreenPosition(const Rml::Event& ev) noexcept
    {
        return RmluiUtils::convert(ev.GetUnprojectedMouseScreenPos());
    }

    sol::table LuaRmluiEvent::getParameters(const Rml::Event& ev, sol::this_state ts) noexcept
    {
        auto& params = ev.GetParameters();
        auto tab = sol::state_view(ts).create_table();
        for (auto& elm : params)
        {
            std::string key = elm.first;
            tab[key] = getRmlVariant(ts.lua_state(), elm.second);
        }
        return tab;
    }

    sol::object LuaRmluiEvent::getParameter(const Rml::Event& ev, const std::string& key, sol::this_state ts) noexcept
    {
        auto& params = ev.GetParameters();
        auto itr = params.find(key);
        if (itr == params.end())
        {
            return sol::nil;
        }
        return getRmlVariant(ts.lua_state(), itr->second);
    }

    std::string LuaRmluiEvent::toString(const Rml::Event& ev) noexcept
    {
        return "RmluiEvent(" + ev.GetType() + ")";
    }
    
    void LuaRmluiEvent::bind(sol::state_view& lua) noexcept
    {
        lua.new_enum<Rml::EventPhase>("RmluiEventPhase", {
            { "None", Rml::EventPhase::None },
            { "Capture", Rml::EventPhase::Capture },
            { "Target", Rml::EventPhase::Target },
            { "Bubble", Rml::EventPhase::Bubble }
        });

        lua.new_enum<Rml::EventId>("RmluiEventId", {
            { "Invalid", Rml::EventId::Invalid },
            { "Mousedown", Rml::EventId::Mousedown },
            { "Mousescroll", Rml::EventId::Mousescroll },
            { "Mouseover", Rml::EventId::Mouseover },
            { "Mouseout", Rml::EventId::Mouseout },
            { "Focus", Rml::EventId::Focus },
            { "Blur", Rml::EventId::Blur },
            { "Keydown", Rml::EventId::Keydown },
            { "Keyup", Rml::EventId::Keyup },
            { "Textinput", Rml::EventId::Textinput },
            { "Mouseup", Rml::EventId::Mouseup },
            { "Click", Rml::EventId::Click },
            { "Dblclick", Rml::EventId::Dblclick },
            { "Load", Rml::EventId::Load },
            { "Unload", Rml::EventId::Unload },
            { "Show", Rml::EventId::Show },
            { "Hide", Rml::EventId::Hide },
            { "Mousemove", Rml::EventId::Mousemove },
            { "Dragmove", Rml::EventId::Dragmove },
            { "Drag", Rml::EventId::Drag },
            { "Dragstart", Rml::EventId::Dragstart },
            { "Dragover", Rml::EventId::Dragover },
            { "Dragdrop", Rml::EventId::Dragdrop },
            { "Dragout", Rml::EventId::Dragout },
            { "Dragend", Rml::EventId::Dragend },
            { "Handledrag", Rml::EventId::Handledrag },
            { "Resize", Rml::EventId::Resize },
            { "Scroll", Rml::EventId::Scroll },
            { "Animationend", Rml::EventId::Animationend },
            { "Transitionend", Rml::EventId::Transitionend },
            { "Change", Rml::EventId::Change },
            { "Submit", Rml::EventId::Submit },
            { "Tabchange", Rml::EventId::Tabchange },
            { "NumDefinedIds", Rml::EventId::NumDefinedIds },
            { "FirstCustomId", Rml::EventId::FirstCustomId },
            { "MaxNumIds", Rml::EventId::MaxNumIds }
        });

        lua.new_usertype<Rml::Event>("RmluiEvent", sol::no_constructor,
            "phase", sol::property(&Rml::Event::GetPhase, &Rml::Event::SetPhase),
            "current_element", sol::property(&Rml::Event::GetCurrentElement, &Rml::Event::SetCurrentElement),
            "target_element", sol::property(&Rml::Event::GetTargetElement),
            "type", sol::property(&Rml::Event::GetType),
            "id", sol::property(&Rml::Event::GetId),
            "stop_propagation", &Rml::Event::StopPropagation,
            "stop_immediate_propagation", &Rml::Event::StopImmediatePropagation,
            "interruptible", sol::property(&Rml::Event::IsInterruptible),
            "propagating", sol::property(&Rml::Event::IsPropagating),
            "immediate_propagating", sol::property(&Rml::Event::IsImmediatePropagating),
            "unprojected_mouse_screen_position", sol::property(&LuaRmluiEvent::getUnprojectedMouseScreenPosition),
            "parameters", sol::property(&LuaRmluiEvent::getParameters),
            "get_parameter", &LuaRmluiEvent::getParameter,
            sol::meta_function::to_string, &LuaRmluiEvent::toString
        );
    }

    LuaFunctionRmluiEventListener::LuaFunctionRmluiEventListener(const sol::protected_function& func) noexcept
        : _func(func)
    {
    }

    void LuaFunctionRmluiEventListener::OnDetach(Rml::Element*) noexcept
    {
        delete this;
    }

    void LuaFunctionRmluiEventListener::ProcessEvent(Rml::Event& event)
    {
        auto result = _func(event);
        LuaUtils::checkResult("rmlui function event: " + event.GetType(), result);
    }

    LuaTableRmluiEventListener::LuaTableRmluiEventListener(const sol::table& tab) noexcept
        : _tab(tab)
    {
    }

    void LuaTableRmluiEventListener::OnDetach(Rml::Element*) noexcept
    {
        delete this;
    }

    void LuaTableRmluiEventListener::ProcessEvent(Rml::Event& event)
    {
        LuaUtils::callTableDelegate(_tab, "on_rmlui_event", "rmlui table event: " + event.GetType(),
            [&event](auto& func, auto& self) {
                return func(self, event);
            });
    }

    LuaFunctionRmluiCustomEventListener::LuaFunctionRmluiCustomEventListener(const std::string& event, const sol::protected_function& func) noexcept
        : _event(event)
        , _func(func)
    {
    }

    void LuaFunctionRmluiCustomEventListener::onRmluiCustomEvent(Rml::Event& event, const sol::table& args, Rml::Element& element)
    {
        auto result = _func(event, args, element);
        LuaUtils::checkResult("rmlui custom function event: " + event.GetType(), result);
    }

    const sol::protected_function& LuaFunctionRmluiCustomEventListener::getFunction() const
    {
        return _func;
    }

    const std::string& LuaFunctionRmluiCustomEventListener::getEvent() const
    {
        return _event;
    }

    LuaTableRmluiCustomEventListener::LuaTableRmluiCustomEventListener(const std::string& event, const sol::table& tab) noexcept
        : _event(event)
        , _tab(tab)
    {
    }

    const sol::table& LuaTableRmluiCustomEventListener::getTable() const
    {
        return _tab;
    }

    const std::string& LuaTableRmluiCustomEventListener::getEvent() const
    {
        return _event;
    }

    void LuaTableRmluiCustomEventListener::onRmluiCustomEvent(Rml::Event& event, const sol::table& args, Rml::Element& element)
    {
        LuaUtils::callTableDelegate(_tab, "on_rmlui_event", "rmlui table event: " + event.GetType(),
            [&event, &args, &element](auto& func, auto& self) {
                return func(self, event, args, element);
        });
    }
}