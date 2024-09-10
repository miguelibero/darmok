#include "rmlui.hpp"
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

    void LuaRmluiElement::addEventListener1(Rml::Element& elm, const std::string& ev, const sol::table& tab) noexcept
    {

    }

    void LuaRmluiElement::addEventListener2(Rml::Element& elm, const std::string& ev, const sol::protected_function& func) noexcept
    {

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
            "add_event_listener", sol::overload(
                &LuaRmluiElement::addEventListener1,
                &LuaRmluiElement::addEventListener2
            ),
            "remove_event_listener", sol::overload(
                &LuaRmluiElement::removeEventListener1,
                &LuaRmluiElement::removeEventListener2,
                &LuaRmluiElement::removeEventListener3,
                &LuaRmluiElement::removeEventListener4
            )
        );

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