#include "rmlui.hpp"
#include "../rmlui.hpp"

namespace darmok
{
    Rml::DataVariableType LuaRmluiUtils::getDataVariableType(const sol::object& obj) noexcept
    {
        if (obj.get_type() != sol::type::table)
        {
            return Rml::DataVariableType::Scalar;
        }
        sol::table table = obj;
        if (LuaUtils::isArray(table))
        {
            return Rml::DataVariableType::Array;
        }
        return Rml::DataVariableType::Struct;
    }

    bool LuaRmluiUtils::setVariant(Rml::Variant& variant, const sol::object& obj) noexcept
    {
        auto type = obj.get_type();
        switch (type)
        {
            case sol::type::none:
            case sol::type::nil:
                variant.Clear();
                return true;
            case sol::type::boolean:
                variant = obj.as<bool>();
                return true;
            case sol::type::number:
                variant = obj.as<double>();
                return true;
            case sol::type::string:
                variant = obj.as<std::string>();
                return true;
            case sol::type::userdata:
            {
                if (obj.is<glm::vec2>())
                {
                    variant = RmluiUtils::convert(obj.as<glm::vec2>());
                    return true;
                }
                if (obj.is<glm::vec3>())
                {
                    variant = RmluiUtils::convert(obj.as<glm::vec3>());
                    return true;
                }
                if (obj.is<glm::vec4>())
                {
                    variant = RmluiUtils::convert(obj.as<glm::vec4>());
                    return true;
                }
                if (obj.is<Color>())
                {
                    variant = RmluiUtils::convert(obj.as<Color>());
                    return true;
                }
                break;
            }
            default:
                break;
        }
        return false;
    }

    bool LuaRmluiUtils::getVariant(sol::object& obj, const Rml::Variant& variant) noexcept
    {
        auto lua = obj.lua_state();
        switch (variant.GetType())
        {
        case Rml::Variant::BOOL:
            obj = sol::object(lua, variant.Get<bool>());
            return true;
        case Rml::Variant::BYTE:
            obj = sol::object(lua, variant.Get<uint8_t>());
            return true;
        case Rml::Variant::CHAR:
            obj = sol::object(lua, variant.Get<char>());
            return true;
        case Rml::Variant::FLOAT:
            obj = sol::object(lua, variant.Get<float>());
            return true;
        case Rml::Variant::DOUBLE:
            obj = sol::object(lua, variant.Get<double>());
            return true;
        case Rml::Variant::INT:
            obj = sol::object(lua, variant.Get<int>());
            return true;
        case Rml::Variant::INT64:
            obj = sol::object(lua, variant.Get<int64_t>());
            return true;
        case Rml::Variant::UINT:
            obj = sol::object(lua, variant.Get<unsigned int>());
            return true;
        case Rml::Variant::UINT64:
            obj = sol::object(lua, variant.Get<uint64_t>());
            return true;
        case Rml::Variant::STRING:
            obj = sol::make_object(lua, variant.Get<Rml::String>());
            return true;
        case Rml::Variant::VECTOR2:
            obj = sol::make_object(lua, RmluiUtils::convert(variant.Get<Rml::Vector2f>()));
            return true;
        case Rml::Variant::VECTOR3:
            obj = sol::make_object(lua, RmluiUtils::convert(variant.Get<Rml::Vector3f>()));
            return true;
        case Rml::Variant::VECTOR4:
            obj = sol::make_object(lua, RmluiUtils::convert(variant.Get<Rml::Vector4f>()));
            return true;
        case Rml::Variant::COLOURF:
            obj = sol::make_object(lua, RmluiUtils::convert(variant.Get<Rml::Colourf>()));
            return true;
        case Rml::Variant::COLOURB:
            obj = sol::make_object(lua, RmluiUtils::convert(variant.Get<Rml::Colourb>()));
            return true;
        default:
            obj.reset();
            return true;
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

    void LuaRmluiUtils::getVariantList(lua_State* lua, std::vector<sol::object>& objs, const Rml::VariantList& variants) noexcept
    {
        for (auto& variant : variants)
        {
            getVariant(objs.emplace_back(lua), variant);
        }
    }

    LuaRmluiVariableDefinition::LuaRmluiVariableDefinition(const sol::table& table, Rml::DataVariableType type) noexcept
        : Rml::VariableDefinition(type)
        , _table(table)
    {
    }

    sol::object LuaRmluiVariableDefinition::nilObject() const noexcept
    {
        return sol::object(_table.lua_state(), sol::nil);
    }

    sol::object LuaRmluiVariableDefinition::getTableValue(const AbsTableKey& key) const noexcept
    {
        sol::table tab(_table);
        for(size_t i = 0; i < key.size(); ++i)
        {
            auto elm = tab[key[i]];
            if (elm.get_type() == sol::type::table)
            {
                tab = elm;
            }
            else if (i == key.size() - 1)
            {
                return elm;
            }
            else
            {
                return nilObject();
            }
        }
        return tab;
    }

    OptionalRef<const LuaRmluiVariableDefinition::AbsTableKey> LuaRmluiVariableDefinition::getPointerKey(void* ptr) const noexcept
    {
        auto index = static_cast<int>(reinterpret_cast<intptr_t>(ptr));
        if (index < 0 || index >= _keys.size())
        {
            return nullptr;
        }
        return _keys.at(index);
    }

    void* LuaRmluiVariableDefinition::getKeyPointer(const AbsTableKey& key) noexcept
    {
        auto itr = std::find(_keys.begin(), _keys.end(), key);
        auto index = 0;
        if (itr != _keys.end())
        {
            index = std::distance(_keys.begin(), itr);
        }
        else
        {
            index = _keys.size();
            _keys.emplace_back(key);
        }
        return reinterpret_cast<void*>(static_cast<intptr_t>(index));
    }

    sol::object LuaRmluiVariableDefinition::getPointerObject(void* ptr) const noexcept
    {
        if (auto key = getPointerKey(ptr))
        {
            return getTableValue(key.value());
        }
        return nilObject();
    }

    bool LuaRmluiVariableDefinition::Get(void* ptr, Rml::Variant& variant) noexcept
    {
        auto obj = getPointerObject(ptr);
        return LuaRmluiUtils::setVariant(variant, obj);
    }

    bool LuaRmluiVariableDefinition::Set(void* ptr, const Rml::Variant& variant) noexcept
    {
        auto obj = getPointerObject(ptr);
        return LuaRmluiUtils::getVariant(obj, variant);
    }

    int LuaRmluiVariableDefinition::Size(void* ptr) noexcept
    {
        auto obj = getPointerObject(ptr);
        if (obj.get_type() != sol::type::table)
        {
            return 0;
        }
        sol::table table = obj;
        return table.size();
    }

    Rml::DataVariable LuaRmluiVariableDefinition::Child(void* ptr, const Rml::DataAddressEntry& address) noexcept
    {
        auto key = getPointerKey(ptr);
        if (!key)
        {
            return {};
        }
        AbsTableKey childKey = key.value();
        if (address.index < 0)
        {
            childKey.push_back(address.name);
        }
        else
        {
            childKey.push_back(address.index + 1);
        }
        auto childPtr = getKeyPointer(childKey);
        return Rml::DataVariable(this, childPtr);
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
            sol::object val = tab[key];
            LuaRmluiUtils::getVariant(val, elm.second);
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

        sol::object obj(ts.lua_state());
        LuaRmluiUtils::getVariant(obj, itr->second);
        return obj;
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