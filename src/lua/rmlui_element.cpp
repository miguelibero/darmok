#include "rmlui.hpp"
#include "../rmlui.hpp"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/StyleSheet.h>

namespace darmok
{

    std::optional<std::string> LuaRmluiElement::getProperty(Rml::Element& elm, const std::string& name) noexcept
    {
        if (auto prop = elm.GetProperty(name))
        {
            return prop->ToString();
        }
        return std::nullopt;
    }

    bool LuaRmluiElement::hasAttribute(const Rml::Element& elm, const std::string& name) noexcept
    {
        auto& attrs = elm.GetAttributes();
        auto itr = attrs.find(name);
        return itr != attrs.end();
    }

    sol::object LuaRmluiElement::getAttribute(const Rml::Element& elm, const std::string& name, sol::this_state ts) noexcept
    {
        auto& attrs = elm.GetAttributes();
        auto itr = attrs.find(name);
        if (itr == attrs.end())
        {
            return sol::nil;
        }
        return LuaRmluiEvent::getRmlVariant(ts.lua_state(), itr->second);
    }
    
    void LuaRmluiElement::setAttribute(Rml::Element& elm, const std::string& name, const sol::object& val)
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
        throw std::invalid_argument("cannot set attribute of that lua type");
    }

    std::vector<Rml::Element*> LuaRmluiElement::getElementsByClassName(Rml::Element& elm, const std::string& cls) noexcept
    {
        std::vector<Rml::Element*> elements;
        elm.GetElementsByClassName(elements, cls);
        return elements;
    }

    std::vector<Rml::Element*> LuaRmluiElement::getElementsByTagName(Rml::Element& elm, const std::string& tag) noexcept
    {
        std::vector<Rml::Element*> elements;
        elm.GetElementsByTagName(elements, tag);
        return elements;
    }

    std::vector<Rml::Element*> LuaRmluiElement::querySelectorAll(Rml::Element& elm, const std::string& selector) noexcept
    {
        std::vector<Rml::Element*> elements;
        elm.QuerySelectorAll(elements, selector);
        return elements;
    }

    int LuaRmluiElement::getNumChildren(Rml::Element& elm) noexcept
    {
        return elm.GetNumChildren();
    }

    Rml::Element& LuaRmluiElement::addEventListener1(Rml::Element& elm, const std::string& ev, const sol::table& tab) noexcept
    {
        elm.AddEventListener(ev, new LuaRmluiEventListener(tab));
        return elm;
    }

    Rml::Element& LuaRmluiElement::addEventListener2(Rml::Element& elm, const std::string& ev, const sol::protected_function& func) noexcept
    {
        elm.AddEventListener(ev, new LuaRmluiEventListener(func));
        return elm;
    }

    bool LuaRmluiElement::removeEventListener1(Rml::Element& elm, const std::string& ev, const sol::table& tab) noexcept
    {
        return false;
    }

    bool LuaRmluiElement::removeEventListener2(Rml::Element& elm, const std::string& ev, const sol::protected_function& func) noexcept
    {
        return false;
    }

    bool LuaRmluiElement::removeEventListener3(Rml::Element& elm, const sol::table& tab) noexcept
    {
        return false;
    }

    bool LuaRmluiElement::removeEventListener4(Rml::Element& elm, const sol::protected_function& func) noexcept
    {
        return false;
    }

    bool LuaRmluiElement::dispatchEvent1(Rml::Element& elm, const std::string& type, const sol::table& params)
    {
        return dispatchEvent3(elm, type, params, false, true);
    }

    bool LuaRmluiElement::dispatchEvent2(Rml::Element& elm, const std::string& type, const sol::table& params, bool interruptible)
    {
        return dispatchEvent3(elm, type, params, interruptible, true);
    }

    bool LuaRmluiElement::dispatchEvent3(Rml::Element& elm, const std::string& type, const sol::table& params, bool interruptible, bool bubbles)
    {
        Rml::Dictionary dict;
        for (auto& [key, val] : params)
        {
            Rml::Variant var;
            LuaRmluiEvent::setRmlVariant(var, val);
            dict.emplace(key.as<std::string>(), var);
        }
        return elm.DispatchEvent(type, dict, interruptible, bubbles);
    }

    bool LuaRmluiElement::focus1(Rml::Element& elm) noexcept
    {
        return elm.Focus();
    }

    bool LuaRmluiElement::focus2(Rml::Element& elm, bool visible)
    {
        return elm.Focus(visible);
    }

    glm::vec2 LuaRmluiElement::getOffset(Rml::Element& elm)
    {
        if (elm.GetPosition() == Rml::Style::Position::Absolute)
        {
            RmluiUtils::convert(elm.GetAbsoluteOffset());
        }
        return RmluiUtils::convert(elm.GetRelativeOffset());
    }

    void LuaRmluiElement::setOffset1(Rml::Element& elm, const VarLuaTable<glm::vec2>& offset)
    {
        auto rmlOffset = RmluiUtils::convert(LuaGlm::tableGet(offset));
        elm.SetOffset(rmlOffset, nullptr);
    }

    void LuaRmluiElement::setOffset2(Rml::Element& elm, const VarLuaTable<glm::vec2>& offset, Rml::Element& parent)
    {
        auto rmlOffset = RmluiUtils::convert(LuaGlm::tableGet(offset));
        elm.SetOffset(rmlOffset, &parent);
    }

    void LuaRmluiElement::setOffset3(Rml::Element& elm, const VarLuaTable<glm::vec2>& offset, Rml::Element& parent, bool fixed)
    {
        auto rmlOffset = RmluiUtils::convert(LuaGlm::tableGet(offset));
        elm.SetOffset(rmlOffset, &parent, fixed);
    }

    glm::vec2 LuaRmluiElement::getSize(Rml::Element& elm)
    {
        return RmluiUtils::convert(elm.GetBox().GetSize());
    }

    void LuaRmluiElement::setSize(Rml::Element& elm, const glm::vec2& size)
    {
        auto box = elm.GetBox();
        box.SetContent(RmluiUtils::convert(size));
        elm.SetBox(box);
    }

    void LuaRmluiElement::bind(sol::state_view& lua) noexcept
    {
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
            "inner_rml", sol::property(
                sol::resolve<Rml::String() const>(&Rml::Element::GetInnerRML),
                &Rml::Element::SetInnerRML),
            "set_property", sol::resolve<bool(const Rml::String&, const Rml::String&)>(&Rml::Element::SetProperty),
            "get_property", &LuaRmluiElement::getProperty,
            "has_attribute", &LuaRmluiElement::hasAttribute,
            "get_attribute", &LuaRmluiElement::getAttribute,
            "set_attribute", &LuaRmluiElement::setAttribute,
            "get_element_by_id", &Rml::Element::GetElementById,
            "get_elements_by_class_name", &LuaRmluiElement::getElementsByClassName,
            "get_elements_by_tag_name", &LuaRmluiElement::getElementsByTagName,
            "query_selector", &Rml::Element::QuerySelector,
            "query_selector_all", &LuaRmluiElement::querySelectorAll,
            "get_child", &Rml::Element::GetChild,
            "num_children", sol::property(&LuaRmluiElement::getNumChildren),
            "add_event_listener", sol::overload(
                &LuaRmluiElement::addEventListener1,
                &LuaRmluiElement::addEventListener2
            ),
            "remove_event_listener", sol::overload(
                &LuaRmluiElement::removeEventListener1,
                &LuaRmluiElement::removeEventListener2,
                &LuaRmluiElement::removeEventListener3,
                &LuaRmluiElement::removeEventListener4
            ),
            "dispatch_event", sol::overload(
                &LuaRmluiElement::dispatchEvent1,
                &LuaRmluiElement::dispatchEvent2,
                &LuaRmluiElement::dispatchEvent3
            ),
            "focus", sol::overload(&LuaRmluiElement::focus1, &LuaRmluiElement::focus2),
            "blur" , &Rml::Element::Blur,
            "click" , &Rml::Element::Click,
            "position", sol::property(&Rml::Element::GetPosition),
            "float", sol::property(&Rml::Element::GetFloat),
            "display", sol::property(&Rml::Element::GetDisplay),
            "box", sol::property(
                sol::resolve<const Rml::Box & ()>(&Rml::Element::GetBox), &Rml::Element::SetBox),
            "size", sol::property(&LuaRmluiElement::getSize, &LuaRmluiElement::setSize),
            "offset", sol::property(&LuaRmluiElement::getOffset, &LuaRmluiElement::setOffset1),
            "set_offset", sol::overload(
                &LuaRmluiElement::setOffset1,
                &LuaRmluiElement::setOffset2,
                &LuaRmluiElement::setOffset3
            )
        );

        lua.new_enum<Rml::Style::Position>("RmluiStylePosition", {
            { "Static", Rml::Style::Position::Static },
            { "Relative", Rml::Style::Position::Relative },
            { "Absolute", Rml::Style::Position::Absolute },
            { "Fixed", Rml::Style::Position::Fixed },
        });

        lua.new_enum<Rml::Style::Float>("RmluiStyleFloat", {
            { "None", Rml::Style::Float::None },
            { "Left", Rml::Style::Float::Left },
            { "Right", Rml::Style::Float::Right },
        });

        lua.new_enum<Rml::Style::Display>("RmluiStyleDisplay", {
            { "None", Rml::Style::Display::None },
            { "Block", Rml::Style::Display::Block },
            { "Inline", Rml::Style::Display::Inline },
            { "InlineBlock", Rml::Style::Display::InlineBlock },
            { "FlowRoot", Rml::Style::Display::FlowRoot },
            { "Flex", Rml::Style::Display::Flex },
            { "InlineFlex", Rml::Style::Display::InlineFlex },
            { "Table", Rml::Style::Display::Table },
            { "InlineTable", Rml::Style::Display::InlineTable },
            { "TableRow", Rml::Style::Display::TableRow },
            { "TableRowGroup", Rml::Style::Display::TableRowGroup },
            { "TableColumn", Rml::Style::Display::TableColumn },
            { "TableColumnGroup", Rml::Style::Display::TableColumnGroup },
            { "TableCell", Rml::Style::Display::TableCell },
        });

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
            { "Instant", Rml::ScrollBehavior::Instant },
        });

        lua.new_enum<Rml::ScrollAlignment>("RmluiScrollAlignment", {
            { "Start", Rml::ScrollAlignment::Start },
            { "Center", Rml::ScrollAlignment::Center },
            { "End", Rml::ScrollAlignment::End },
            { "Nearest", Rml::ScrollAlignment::Nearest },
        });
    }

    void LuaRmluiStyleSheet::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<Rml::StyleSheet>("RmluiStyleSheet", sol::no_constructor
        );
    }

    void LuaRmluiElementDocument::show1(Rml::ElementDocument& doc) noexcept
    {
        doc.Show();
    }

    void LuaRmluiElementDocument::show2(Rml::ElementDocument& doc, Rml::ModalFlag modal) noexcept
    {
        doc.Show(modal);
    }

    void LuaRmluiElementDocument::show3(Rml::ElementDocument& doc, Rml::ModalFlag modal, Rml::FocusFlag focus) noexcept
    {
        doc.Show(modal, focus);
    }

    void LuaRmluiElementDocument::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<Rml::ElementDocument>("RmluiDocument", sol::no_constructor,
            "show", sol::overload(
                &LuaRmluiElementDocument::show1,
                &LuaRmluiElementDocument::show2,
                &LuaRmluiElementDocument::show3
            ),
            "close", &Rml::ElementDocument::Close,
            sol::base_classes, sol::bases<Rml::Element>()
        );
    }
}