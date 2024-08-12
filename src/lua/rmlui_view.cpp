#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <darmok/rmlui.hpp>
#include <darmok/viewport.hpp>
#include <darmok/texture.hpp>
#include <darmok/string.hpp>
#include "camera.hpp"
#include "transform.hpp"
#include "utils.hpp"
#include "app.hpp"
#include "glm.hpp"
#include "viewport.hpp"
#include "../rmlui.hpp"
#include <optional>

namespace darmok
{
    void LuaRmluiView::setRmlVariant(Rml::Variant& variant, sol::object obj) noexcept
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

    sol::object LuaRmluiView::getRmlVariant(lua_State* lua, const Rml::Variant& variant) noexcept
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
    
    std::string LuaRmluiView::getName() const noexcept
    {
        return _view.getName();
    }

    LuaRmluiView& LuaRmluiView::setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept
    {
        _view.setTargetTexture(texture);
        return *this;
    }

    std::shared_ptr<Texture> LuaRmluiView::getTargetTexture() noexcept
    {
        return _view.getTargetTexture();
    }

    const Viewport& LuaRmluiView::getViewport() const noexcept
    {
        return _view.getViewport();
    }

    LuaRmluiView& LuaRmluiView::setViewport(const Viewport& vp) noexcept
    {
        _view.setViewport(vp);
        return *this;
    }

    LuaRmluiView& LuaRmluiView::setEnabled(bool enabled) noexcept
    {
        _view.setEnabled(enabled);
        return *this;
    }

    bool LuaRmluiView::getEnabled() const noexcept
    {
        return _view.getEnabled();
    }

    LuaRmluiView& LuaRmluiView::setInputActive(bool active) noexcept
    {
        _view.setInputActive(active);
        return *this;
    }

    bool LuaRmluiView::getInputActive() const noexcept
    {
        return _view.getInputActive();
    }

    LuaRmluiView& LuaRmluiView::setMousePosition(const glm::vec2& position) noexcept
    {
        _view.setMousePosition(position);
        return *this;
    }

    const glm::vec2& LuaRmluiView::getMousePosition() const noexcept
    {
        return _view.getMousePosition();
    }

    LuaRmluiView& LuaRmluiView::setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept
    {
        _view.setScrollBehavior(behaviour, speedFactor);
        return *this;
    }

    Rml::ElementDocument& LuaRmluiView::loadDocument(const std::string& name)
    {
        return *_view.getContext().LoadDocument(name);
    }

    LuaRmluiView::LuaRmluiView(RmluiView& view) noexcept
        : _view(view)
    {
    }

    LuaRmluiView::~LuaRmluiView() noexcept
    {
        auto& ctxt = _view.getContext();
        for (auto& listener : _tabEventListeners)
        {
            ctxt.RemoveEventListener(listener->getEvent(), listener.get());
        }
        for (auto& listener : _funcEventListeners)
        {
            ctxt.RemoveEventListener(listener->getEvent(), listener.get());
        }
    }

    void LuaRmluiView::createDataModel(const std::string& name, sol::table table) noexcept
    {
        auto constr = _view.getContext().CreateDataModel(name);
        if (!constr)
        {
            return;
        }
        for (auto& [key, val] : table)
        {
            auto str = key.as<std::string>();
            constr.BindFunc(str,
                [table, key](Rml::Variant& var) { setRmlVariant(var, table[key]); },
                [table, key](const Rml::Variant& var) mutable
                    { table[key] = getRmlVariant(table.lua_state(), var); }
            );
        }
        auto handle = constr.GetModelHandle();
        table["is_dirty"] = [handle](const std::string& name) mutable
        {
            return handle.IsVariableDirty(name);
        };
        table["set_dirty"] = sol::overload(
            [handle]() mutable
            {
                return handle.DirtyAllVariables();
            },
            [handle](const std::string& name) mutable
            {
                return handle.DirtyVariable(name);
            }
        );
    }

    size_t LuaRmluiView::processCustomEvent(Rml::Event& event, const std::string& name, const sol::table& args)
    {
        size_t count = 0;
        for (auto& listener : _funcEventListeners)
        {
            if (listener->getEvent() == name)
            {
                listener->processCustomEvent(event, name, args);
                ++count;
            }
        }
        for (auto& listener : _tabEventListeners)
        {
            if (listener->getEvent() == name)
            {
                listener->processCustomEvent(event, name, args);
                ++count;
            }
        }
        return count;
    }

    Rml::DataModelHandle LuaRmluiView::getDataModel(const std::string& name) const noexcept
    {
        return _view.getContext().GetDataModel(name).GetModelHandle();
    }

    bool LuaRmluiView::removeDataModel(const std::string& name) noexcept
    {
        return _view.getContext().RemoveDataModel(name);
    }

    LuaRmluiView& LuaRmluiView::addEventListener1(const std::string& ev, const sol::table& tab) noexcept
    {
        auto ptr = std::make_unique<LuaTableRmluiEventListener>(ev, tab);
        _view.getContext().AddEventListener(ev, ptr.get());
        _tabEventListeners.push_back(std::move(ptr));
        return *this;
    }

    bool LuaRmluiView::removeEventListener1(const std::string& ev, const sol::table& tab) noexcept
    {
        auto itr = std::remove_if(_tabEventListeners.begin(), _tabEventListeners.end(), [&ev, &tab](auto& listener) {
            return listener->getEvent() == ev && listener->getTable() == tab;
        });
        if (itr == _tabEventListeners.end())
        {
            return false;
        }
        _tabEventListeners.erase(itr, _tabEventListeners.end());
        return true;
    }

    bool LuaRmluiView::removeEventListener2(const sol::table& tab) noexcept
    {
        auto itr = std::remove_if(_tabEventListeners.begin(), _tabEventListeners.end(), [&tab](auto& listener) {
            return listener->getTable() == tab;
        });
        if (itr == _tabEventListeners.end())
        {
            return false;
        }
        _tabEventListeners.erase(itr, _tabEventListeners.end());
        return true;
    }

    LuaRmluiView& LuaRmluiView::addEventListener2(const std::string& ev, const sol::protected_function& func) noexcept
    {
        auto ptr = std::make_unique<LuaFunctionRmluiEventListener>(ev, func);
        _view.getContext().AddEventListener(ev, ptr.get());
        _funcEventListeners.push_back(std::move(ptr));
        return *this;
    }

    bool LuaRmluiView::removeEventListener4(const std::string& ev, const sol::protected_function& func) noexcept
    {
        auto itr = std::remove_if(_funcEventListeners.begin(), _funcEventListeners.end(), [&ev, &func](auto& listener) {
            return listener->getEvent() == ev && listener->getFunction() == func;
            });
        if (itr == _funcEventListeners.end())
        {
            return false;
        }
        _funcEventListeners.erase(itr, _funcEventListeners.end());
        return true;
    }

    bool LuaRmluiView::removeEventListener3(const sol::protected_function& func) noexcept
    {
        auto itr = std::remove_if(_funcEventListeners.begin(), _funcEventListeners.end(), [&func](auto& listener) {
            return listener->getFunction() == func;
            });
        if (itr == _funcEventListeners.end())
        {
            return false;
        }
        _funcEventListeners.erase(itr, _funcEventListeners.end());
        return true;
    }

    void LuaRmluiView::bind(sol::state_view& lua) noexcept
    {
        lua.new_enum<Rml::ModalFlag>("RmluiDocumentMode", {
            { "Normal", Rml::ModalFlag::None },
            { "Modal", Rml::ModalFlag::Modal },
            { "Keep", Rml::ModalFlag::Keep },
        });

        lua.new_enum<Rml::FocusFlag>("RmluiDocumentFocus", {
            { "Normal", Rml::FocusFlag::None },
            { "Document", Rml::FocusFlag::Document },
            { "Keep", Rml::FocusFlag::Keep },
            { "Auto", Rml::FocusFlag::Auto },
        });

        lua.new_enum<Rml::ScrollBehavior>("RmluiScrollBehavior", {
            { "Auto", Rml::ScrollBehavior::Auto },
            { "Smooth", Rml::ScrollBehavior::Smooth },
            { "Instant", Rml::ScrollBehavior::Instant }
        });

        lua.new_usertype<Rml::ElementDocument>("RmluiDocument", sol::no_constructor,
            "show", sol::overload(
                [](Rml::ElementDocument& doc) { doc.Show(); },
                [](Rml::ElementDocument& doc, Rml::ModalFlag modal) { doc.Show(modal); },
                [](Rml::ElementDocument& doc, Rml::ModalFlag modal, Rml::FocusFlag focus) { doc.Show(modal, focus); }
            )
        );

        lua.new_usertype<Rml::Element>("RmluiElement", sol::no_constructor,
            "set_class", &Rml::Element::SetClass,
            "has_class", &Rml::Element::IsClassSet,
            "class_names", sol::property(&Rml::Element::GetClassNames, &Rml::Element::SetClassNames),
            "id", sol::property(&Rml::Element::GetId, &Rml::Element::SetId),
            "tag_name", sol::property(&Rml::Element::GetTagName),
            "stylesheet", sol::property(&Rml::Element::GetStyleSheet),
            "address", sol::property(&Rml::Element::GetAddress),
            "visible", sol::property(&Rml::Element::IsVisible),
            "z_index", sol::property(&Rml::Element::GetZIndex),
            "set_property", sol::resolve<bool(const Rml::String&, const Rml::String&)>(&Rml::Element::SetProperty),
            "get_property", [](Rml::Element& elm, const std::string& name) -> std::optional<std::string> {
                if (auto prop = elm.GetProperty(name))
                {
                    return prop->ToString();
                }
                return std::nullopt;
            },
            "get_attribute", [](Rml::Element& elm, const std::string& name, sol::this_state ts) -> sol::object
            {
                auto& attrs = elm.GetAttributes();
                auto itr = attrs.find(name);
                if (itr == attrs.end())
                {
                    return sol::nil;
                }
                return getRmlVariant(ts.lua_state(), itr->second);
            },
            "set_attribute", [](Rml::Element& elm, const std::string& name, const sol::object& val)
            {
                switch (val.get_type())
                {
                case sol::type::string:
                    elm.SetAttribute(name, val.as<std::string>());
                    break;
                case sol::type::number:
                    elm.SetAttribute(name, val.as<float>());
                    break;
                case sol::type::boolean:
                    elm.SetAttribute(name, val.as<bool>());
                    break;
                }
            }
        );

        lua.new_usertype<Rml::DataModelHandle>("RmluiModelHandle", sol::no_constructor,
            "is_dirty", &Rml::DataModelHandle::IsVariableDirty,
            "set_dirty", sol::overload(
                &Rml::DataModelHandle::DirtyVariable,
                &Rml::DataModelHandle::DirtyAllVariables
            )
        );

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
            { "Columnadd", Rml::EventId::Columnadd },
            { "Rowadd", Rml::EventId::Rowadd },
            { "Rowchange", Rml::EventId::Rowchange },
            { "Rowremove", Rml::EventId::Rowremove },
            { "Rowupdate", Rml::EventId::Rowupdate },
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
            "unprojected_mouse_screen_position", sol::property([](const Rml::Event& ev) {
                return RmluiUtils::convert(ev.GetUnprojectedMouseScreenPos());
            }),
            "parameters", sol::property([](const Rml::Event& ev, sol::this_state ts) {
                auto& params = ev.GetParameters();
                auto tab = sol::state_view(ts).create_table();
                for (auto& elm : params)
                {
                    std::string key = elm.first;
                    tab[key] = getRmlVariant(ts.lua_state(), elm.second);
                }
                return tab;
            }),
            "get_parameter", [](const Rml::Event& ev, const std::string& key, sol::this_state ts) -> sol::object {
                auto& params = ev.GetParameters();
                auto itr = params.find(key);
                if (itr == params.end())
                {
                    return sol::nil;
                }
                return getRmlVariant(ts.lua_state(), itr->second);
            },
            sol::meta_function::to_string, [](const Rml::Event& ev) {
                return "RmluiEvent(" + ev.GetType() + ")";
            }
        );

        lua.new_usertype<LuaRmluiView>("RmluiView", sol::no_constructor,
            "name", sol::property(&LuaRmluiView::getName),
            "target_texture", sol::property(&LuaRmluiView::getTargetTexture, &LuaRmluiView::setTargetTexture),
            "viewport", sol::property(&LuaRmluiView::getViewport, &LuaRmluiView::setViewport),
            "input_active", sol::property(&LuaRmluiView::getInputActive, &LuaRmluiView::setInputActive),
            "mouse_position", sol::property(&LuaRmluiView::getMousePosition, &LuaRmluiView::setMousePosition),
            "set_scroll_behavior", &LuaRmluiView::setScrollBehavior,
            "load_document", &LuaRmluiView::loadDocument,
            "create_data_model", &LuaRmluiView::createDataModel,
            "get_data_model", &LuaRmluiView::getDataModel,
            "remove_data_model", &LuaRmluiView::removeDataModel,
            "add_event_listener", sol::overload(&LuaRmluiView::addEventListener1, &LuaRmluiView::addEventListener2),
            "remove_event_listener", sol::overload(
                &LuaRmluiView::removeEventListener1,
                &LuaRmluiView::removeEventListener2,
                &LuaRmluiView::removeEventListener3,
                &LuaRmluiView::removeEventListener4
            )
        );
    }

    LuaFunctionRmluiEventListener::LuaFunctionRmluiEventListener(const std::string& event, const sol::protected_function& func) noexcept
        : _event(event)
        , _func(func)
    {
    }

    void LuaFunctionRmluiEventListener::ProcessEvent(Rml::Event& event)
    {
        auto result = _func(event);
        LuaUtils::checkResult("rmlui function event: " + event.GetType(), result);
    }

    void LuaFunctionRmluiEventListener::processCustomEvent(Rml::Event& event, const std::string& name, const sol::table& args)
    {
        auto result = _func(event, args);
        LuaUtils::checkResult("rmlui custom function event: " + event.GetType(), result);
    }

    const sol::protected_function& LuaFunctionRmluiEventListener::getFunction() const
    {
        return _func;
    }

    const std::string& LuaFunctionRmluiEventListener::getEvent() const
    {
        return _event;
    }

    LuaTableRmluiEventListener::LuaTableRmluiEventListener(const std::string& event, const sol::table& tab) noexcept
        : _event(event)
        , _tab(tab)
    {
    }

    const sol::table& LuaTableRmluiEventListener::getTable() const
    {
        return _tab;
    }

    const std::string& LuaTableRmluiEventListener::getEvent() const
    {
        return _event;
    }

    void LuaTableRmluiEventListener::ProcessEvent(Rml::Event& event)
    {
        LuaUtils::callTableDelegate(_tab, "on_rmlui_event", "rmlui table event: " + event.GetType(),
            [&event](auto& func, auto& self) {
                return func(self, event);
        });
    }

    void LuaTableRmluiEventListener::processCustomEvent(Rml::Event& event, const std::string& name, const sol::table& args)
    {
        LuaUtils::callTableDelegate(_tab, "on_rmlui_event", "rmlui table event: " + event.GetType(),
            [&event, &args](auto& func, auto& self) {
                return func(self, event, args);
        });
    }
}