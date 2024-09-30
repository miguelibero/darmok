#include "rmlui.hpp"
#include "../rmlui.hpp"

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

        lua.new_enum<Rml::Input::KeyIdentifier>("RmluiKeyId", {
            { "UNKNOWN", Rml::Input::KeyIdentifier::KI_UNKNOWN },
            { "SPACE",Rml::Input::KeyIdentifier::KI_SPACE },

            { "0", Rml::Input::KeyIdentifier::KI_0 },
            { "1", Rml::Input::KeyIdentifier::KI_1 },
            { "2", Rml::Input::KeyIdentifier::KI_2 },
            { "3", Rml::Input::KeyIdentifier::KI_3 },
            { "4", Rml::Input::KeyIdentifier::KI_4 },
            { "5", Rml::Input::KeyIdentifier::KI_5 },
            { "6", Rml::Input::KeyIdentifier::KI_6 },
            { "7", Rml::Input::KeyIdentifier::KI_7 },
            { "8", Rml::Input::KeyIdentifier::KI_8 },
            { "9", Rml::Input::KeyIdentifier::KI_9 },

            { "A", Rml::Input::KeyIdentifier::KI_A },
            { "B", Rml::Input::KeyIdentifier::KI_B },
            { "C", Rml::Input::KeyIdentifier::KI_C },
            { "D", Rml::Input::KeyIdentifier::KI_D },
            { "E", Rml::Input::KeyIdentifier::KI_E },
            { "F", Rml::Input::KeyIdentifier::KI_F },
            { "G", Rml::Input::KeyIdentifier::KI_G },
            { "H", Rml::Input::KeyIdentifier::KI_H },
            { "I", Rml::Input::KeyIdentifier::KI_I },
            { "J", Rml::Input::KeyIdentifier::KI_J },
            { "K", Rml::Input::KeyIdentifier::KI_K },
            { "L", Rml::Input::KeyIdentifier::KI_L },
            { "M", Rml::Input::KeyIdentifier::KI_M },
            { "N", Rml::Input::KeyIdentifier::KI_N },
            { "O", Rml::Input::KeyIdentifier::KI_O },
            { "P", Rml::Input::KeyIdentifier::KI_P },
            { "Q", Rml::Input::KeyIdentifier::KI_Q },
            { "R", Rml::Input::KeyIdentifier::KI_R },
            { "S", Rml::Input::KeyIdentifier::KI_S },
            { "T", Rml::Input::KeyIdentifier::KI_T },
            { "U", Rml::Input::KeyIdentifier::KI_U },
            { "V", Rml::Input::KeyIdentifier::KI_V },
            { "W", Rml::Input::KeyIdentifier::KI_W },
            { "X", Rml::Input::KeyIdentifier::KI_X },
            { "Y", Rml::Input::KeyIdentifier::KI_Y },
            { "Z", Rml::Input::KeyIdentifier::KI_Z },

            { "OEM_1", Rml::Input::KeyIdentifier::KI_OEM_1 },
            { "OEM_PLUS", Rml::Input::KeyIdentifier::KI_OEM_PLUS },
            { "OEM_COMMA", Rml::Input::KeyIdentifier::KI_OEM_COMMA },
            { "OEM_MINUS", Rml::Input::KeyIdentifier::KI_OEM_MINUS },
            { "OEM_PERIOD", Rml::Input::KeyIdentifier::KI_OEM_PERIOD },
            { "OEM_2", Rml::Input::KeyIdentifier::KI_OEM_2 },
            { "OEM_3", Rml::Input::KeyIdentifier::KI_OEM_3 },

            { "OEM_4", Rml::Input::KeyIdentifier::KI_OEM_4 },
            { "OEM_5", Rml::Input::KeyIdentifier::KI_OEM_5 },
            { "OEM_6", Rml::Input::KeyIdentifier::KI_OEM_6 },
            { "OEM_7", Rml::Input::KeyIdentifier::KI_OEM_7 },

            { "OEM_102", Rml::Input::KeyIdentifier::KI_OEM_102 },

            { "NUMPAD0", Rml::Input::KeyIdentifier::KI_NUMPAD0 },
            { "NUMPAD1", Rml::Input::KeyIdentifier::KI_NUMPAD1 },
            { "NUMPAD2", Rml::Input::KeyIdentifier::KI_NUMPAD2 },
            { "NUMPAD3", Rml::Input::KeyIdentifier::KI_NUMPAD3 },
            { "NUMPAD4", Rml::Input::KeyIdentifier::KI_NUMPAD4 },
            { "NUMPAD5", Rml::Input::KeyIdentifier::KI_NUMPAD5 },
            { "NUMPAD6", Rml::Input::KeyIdentifier::KI_NUMPAD6 },
            { "NUMPAD7", Rml::Input::KeyIdentifier::KI_NUMPAD7 },
            { "NUMPAD8", Rml::Input::KeyIdentifier::KI_NUMPAD8 },
            { "NUMPAD9", Rml::Input::KeyIdentifier::KI_NUMPAD9 },
            { "NUMPADENTER", Rml::Input::KeyIdentifier::KI_NUMPADENTER },
            { "MULTIPLY", Rml::Input::KeyIdentifier::KI_MULTIPLY },
            { "ADD", Rml::Input::KeyIdentifier::KI_ADD },
            { "SEPARATOR", Rml::Input::KeyIdentifier::KI_SEPARATOR },
            { "SUBTRACT", Rml::Input::KeyIdentifier::KI_SUBTRACT },
            { "DECIMAL", Rml::Input::KeyIdentifier::KI_DECIMAL },
            { "DIVIDE", Rml::Input::KeyIdentifier::KI_DIVIDE },

            { "OEM_NEC_EQUAL", Rml::Input::KeyIdentifier::KI_OEM_NEC_EQUAL },

            { "BACK", Rml::Input::KeyIdentifier::KI_BACK },
            { "TAB", Rml::Input::KeyIdentifier::KI_TAB },

            { "CLEAR", Rml::Input::KeyIdentifier::KI_CLEAR },
            { "RETURN", Rml::Input::KeyIdentifier::KI_RETURN },

            { "PAUSE", Rml::Input::KeyIdentifier::KI_PAUSE },
            { "CAPITAL", Rml::Input::KeyIdentifier::KI_CAPITAL },

            { "KANA", Rml::Input::KeyIdentifier::KI_KANA },
            { "HANGUL", Rml::Input::KeyIdentifier::KI_HANGUL },
            { "JUNJA", Rml::Input::KeyIdentifier::KI_JUNJA },
            { "FINAL", Rml::Input::KeyIdentifier::KI_FINAL },
            { "HANJA", Rml::Input::KeyIdentifier::KI_HANJA },
            { "KANJI", Rml::Input::KeyIdentifier::KI_KANJI },

            { "ESCAPE", Rml::Input::KeyIdentifier::KI_ESCAPE },

            { "CONVERT", Rml::Input::KeyIdentifier::KI_CONVERT },
            { "NONCONVERT", Rml::Input::KeyIdentifier::KI_NONCONVERT },
            { "ACCEPT", Rml::Input::KeyIdentifier::KI_ACCEPT },
            { "MODECHANGE", Rml::Input::KeyIdentifier::KI_MODECHANGE },

            { "PRIOR", Rml::Input::KeyIdentifier::KI_PRIOR },
            { "NEXT", Rml::Input::KeyIdentifier::KI_NEXT },
            { "END", Rml::Input::KeyIdentifier::KI_END },
            { "HOME", Rml::Input::KeyIdentifier::KI_HOME },
            { "LEFT", Rml::Input::KeyIdentifier::KI_LEFT },
            { "UP", Rml::Input::KeyIdentifier::KI_UP },
            { "RIGHT", Rml::Input::KeyIdentifier::KI_RIGHT },
            { "DOWN", Rml::Input::KeyIdentifier::KI_DOWN },
            { "SELECT", Rml::Input::KeyIdentifier::KI_SELECT },
            { "PRINT", Rml::Input::KeyIdentifier::KI_PRINT },
            { "EXECUTE", Rml::Input::KeyIdentifier::KI_EXECUTE },
            { "SNAPSHOT", Rml::Input::KeyIdentifier::KI_SNAPSHOT },
            { "INSERT", Rml::Input::KeyIdentifier::KI_INSERT },
            { "DELETE", Rml::Input::KeyIdentifier::KI_DELETE },
            { "HELP", Rml::Input::KeyIdentifier::KI_HELP },

            { "LWIN", Rml::Input::KeyIdentifier::KI_LWIN },
            { "RWIN", Rml::Input::KeyIdentifier::KI_RWIN },
            { "APPS", Rml::Input::KeyIdentifier::KI_APPS },

            { "POWER", Rml::Input::KeyIdentifier::KI_POWER },
            { "SLEEP", Rml::Input::KeyIdentifier::KI_SLEEP },
            { "WAKE", Rml::Input::KeyIdentifier::KI_WAKE },

            { "F1", Rml::Input::KeyIdentifier::KI_F1 },
            { "F2", Rml::Input::KeyIdentifier::KI_F2 },
            { "F3", Rml::Input::KeyIdentifier::KI_F3 },
            { "F4", Rml::Input::KeyIdentifier::KI_F4 },
            { "F5", Rml::Input::KeyIdentifier::KI_F5 },
            { "F6", Rml::Input::KeyIdentifier::KI_F6 },
            { "F7", Rml::Input::KeyIdentifier::KI_F7 },
            { "F8", Rml::Input::KeyIdentifier::KI_F8 },
            { "F9", Rml::Input::KeyIdentifier::KI_F9 },
            { "F10", Rml::Input::KeyIdentifier::KI_F10 },
            { "F11", Rml::Input::KeyIdentifier::KI_F11 },
            { "F12", Rml::Input::KeyIdentifier::KI_F12 },
            { "F13", Rml::Input::KeyIdentifier::KI_F13 },
            { "F14", Rml::Input::KeyIdentifier::KI_F14 },
            { "F15", Rml::Input::KeyIdentifier::KI_F15 },
            { "F16", Rml::Input::KeyIdentifier::KI_F16 },
            { "F17", Rml::Input::KeyIdentifier::KI_F17 },
            { "F18", Rml::Input::KeyIdentifier::KI_F18 },
            { "F19", Rml::Input::KeyIdentifier::KI_F19 },
            { "F20", Rml::Input::KeyIdentifier::KI_F20 },
            { "F21", Rml::Input::KeyIdentifier::KI_F21 },
            { "F22", Rml::Input::KeyIdentifier::KI_F22 },
            { "F23", Rml::Input::KeyIdentifier::KI_F23 },
            { "F24", Rml::Input::KeyIdentifier::KI_F24 },

            { "NUMLOCK", Rml::Input::KeyIdentifier::KI_NUMLOCK },
            { "SCROLL", Rml::Input::KeyIdentifier::KI_SCROLL },

            { "OEM_FJ_JISHO", Rml::Input::KeyIdentifier::KI_OEM_FJ_JISHO },
            { "OEM_FJ_MASSHOU", Rml::Input::KeyIdentifier::KI_OEM_FJ_MASSHOU },
            { "OEM_FJ_TOUROKU", Rml::Input::KeyIdentifier::KI_OEM_FJ_TOUROKU },
            { "OEM_FJ_LOYA", Rml::Input::KeyIdentifier::KI_OEM_FJ_LOYA },
            { "OEM_FJ_ROYA", Rml::Input::KeyIdentifier::KI_OEM_FJ_ROYA },

            { "LSHIFT", Rml::Input::KeyIdentifier::KI_LSHIFT },
            { "RSHIFT", Rml::Input::KeyIdentifier::KI_RSHIFT },
            { "LCONTROL", Rml::Input::KeyIdentifier::KI_LCONTROL },
            { "RCONTROL", Rml::Input::KeyIdentifier::KI_RCONTROL },
            { "LMENU", Rml::Input::KeyIdentifier::KI_LMENU },
            { "RMENU", Rml::Input::KeyIdentifier::KI_RMENU },

            { "BROWSER_BACK", Rml::Input::KeyIdentifier::KI_BROWSER_BACK },
            { "BROWSER_FORWARD", Rml::Input::KeyIdentifier::KI_BROWSER_FORWARD },
            { "BROWSER_REFRESH", Rml::Input::KeyIdentifier::KI_BROWSER_REFRESH },
            { "BROWSER_STOP", Rml::Input::KeyIdentifier::KI_BROWSER_STOP },
            { "BROWSER_SEARCH", Rml::Input::KeyIdentifier::KI_BROWSER_SEARCH },
            { "BROWSER_FAVORITES", Rml::Input::KeyIdentifier::KI_BROWSER_FAVORITES },
            { "BROWSER_HOME", Rml::Input::KeyIdentifier::KI_BROWSER_HOME },

            { "VOLUME_MUTE", Rml::Input::KeyIdentifier::KI_VOLUME_MUTE },
            { "VOLUME_DOWN", Rml::Input::KeyIdentifier::KI_VOLUME_DOWN },
            { "VOLUME_UP", Rml::Input::KeyIdentifier::KI_VOLUME_UP },
            { "MEDIA_NEXT_TRACK", Rml::Input::KeyIdentifier::KI_MEDIA_NEXT_TRACK },
            { "MEDIA_PREV_TRACK", Rml::Input::KeyIdentifier::KI_MEDIA_PREV_TRACK },
            { "MEDIA_STOP", Rml::Input::KeyIdentifier::KI_MEDIA_STOP },
            { "MEDIA_PLAY_PAUSE", Rml::Input::KeyIdentifier::KI_MEDIA_PLAY_PAUSE },
            { "LAUNCH_MAIL", Rml::Input::KeyIdentifier::KI_LAUNCH_MAIL },
            { "LAUNCH_MEDIA_SELECT", Rml::Input::KeyIdentifier::KI_LAUNCH_MEDIA_SELECT },
            { "LAUNCH_APP1", Rml::Input::KeyIdentifier::KI_LAUNCH_APP1 },
            { "LAUNCH_APP2", Rml::Input::KeyIdentifier::KI_LAUNCH_APP2 },

            { "OEM_AX", Rml::Input::KeyIdentifier::KI_OEM_AX },
            { "ICO_HELP", Rml::Input::KeyIdentifier::KI_ICO_HELP },
            { "ICO_00", Rml::Input::KeyIdentifier::KI_ICO_00 },

            { "PROCESSKEY", Rml::Input::KeyIdentifier::KI_PROCESSKEY },

            { "ICO_CLEAR", Rml::Input::KeyIdentifier::KI_ICO_CLEAR },

            { "ATTN", Rml::Input::KeyIdentifier::KI_ATTN },
            { "CRSEL", Rml::Input::KeyIdentifier::KI_CRSEL },
            { "EXSEL", Rml::Input::KeyIdentifier::KI_EXSEL },
            { "EREOF", Rml::Input::KeyIdentifier::KI_EREOF },
            { "PLAY", Rml::Input::KeyIdentifier::KI_PLAY },
            { "ZOOM", Rml::Input::KeyIdentifier::KI_ZOOM },
            { "PA1", Rml::Input::KeyIdentifier::KI_PA1 },
            { "OEM_CLEAR", Rml::Input::KeyIdentifier::KI_OEM_CLEAR },

            { "LMETA", Rml::Input::KeyIdentifier::KI_LMETA },
            { "RMETA", Rml::Input::KeyIdentifier::KI_RMETA },
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

    LuaRmluiEventListener::LuaRmluiEventListener(const sol::object& obj) noexcept
        : _delegate(obj, "on_rmlui_event")
    {
    }

    void LuaRmluiEventListener::ProcessEvent(Rml::Event& event) noexcept
    {
        auto result = _delegate(event);
        LuaUtils::checkResult("rmlui event: " + event.GetType(), result);
    }
    void LuaRmluiEventListener::OnDetach(Rml::Element* element) noexcept
    {
        delete this;
    }
}