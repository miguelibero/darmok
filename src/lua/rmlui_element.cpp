#include "rmlui.hpp"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/StyleSheet.h>

namespace darmok
{
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
            "get_property", [](Rml::Element& elm, const std::string& name) -> std::optional<std::string> {
                if (auto prop = elm.GetProperty(name))
                {
                    return prop->ToString();
                }
                return std::nullopt;
            },
            "has_attribute", [](Rml::Element& elm, const std::string& name)
            {
                auto& attrs = elm.GetAttributes();
                auto itr = attrs.find(name);
                return itr != attrs.end();
            },
            "get_attribute", [](Rml::Element& elm, const std::string& name, sol::this_state ts) -> sol::object
            {
                auto& attrs = elm.GetAttributes();
                auto itr = attrs.find(name);
                if (itr == attrs.end())
                {
                    return sol::nil;
                }
                return LuaRmluiEvent::getRmlVariant(ts.lua_state(), itr->second);
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
            },
            "get_element_by_id", [](Rml::Element& elm, const std::string& id) {
                return elm.GetElementById(id);
            },
            "get_elements_by_class_name", [](Rml::Element& elm, const std::string& cls) {
                std::vector<Rml::Element*> elements;
                elm.GetElementsByClassName(elements, cls);
                return elements;
            },
            "get_elements_by_tag_name", [](Rml::Element& elm, const std::string& tag) {
                std::vector<Rml::Element*> elements;
                elm.GetElementsByTagName(elements, tag);
                return elements;
            },
            "query_selector", [](Rml::Element& elm, const std::string& selector) {
                return elm.QuerySelector(selector);
            },
            "query_selector_all", [](Rml::Element& elm, const std::string& selector) {
                std::vector<Rml::Element*> elements;
                elm.QuerySelectorAll(elements, selector);
                return elements;
            }
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

        lua.new_usertype<Rml::ElementDocument>("RmluiDocument", sol::no_constructor,
            "show", sol::overload(
                [](Rml::ElementDocument& doc) { doc.Show(); },
                [](Rml::ElementDocument& doc, Rml::ModalFlag modal) { doc.Show(modal); },
                [](Rml::ElementDocument& doc, Rml::ModalFlag modal, Rml::FocusFlag focus) { doc.Show(modal, focus); }
            ),
            "close", [](Rml::ElementDocument& doc) { doc.Close(); },
            sol::base_classes, sol::bases<Rml::Element>()
        );
    }
}