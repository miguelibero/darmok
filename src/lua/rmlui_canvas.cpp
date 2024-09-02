#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <darmok/rmlui.hpp>
#include <darmok/viewport.hpp>
#include <darmok/texture.hpp>
#include <darmok/string.hpp>
#include "camera.hpp"
#include "utils.hpp"
#include "scene.hpp"
#include "glm.hpp"
#include "viewport.hpp"
#include "../rmlui.hpp"

namespace darmok
{
    std::string LuaRmluiCanvas::getName() const noexcept
    {
        return _canvas.getName();
    }

    std::optional<glm::uvec2> LuaRmluiCanvas::getSize() const noexcept
    {
        return _canvas.getSize();
    }

    LuaRmluiCanvas& LuaRmluiCanvas::setSize(std::optional<VarLuaTable<glm::uvec2>> size) noexcept
    {
        if (size)
        {
            _canvas.setSize(LuaGlm::tableGet(size.value()));
        }
        else
        {
            _canvas.setSize(std::nullopt);
        }
        return *this;
    }

    glm::uvec2 LuaRmluiCanvas::getCurrentSize() const noexcept
    {
        return _canvas.getCurrentSize();
    }

    const glm::vec3& LuaRmluiCanvas::getOffset() const noexcept
    {
        return _canvas.getOffset();
    }

    LuaRmluiCanvas& LuaRmluiCanvas::setOffset(const VarLuaTable<glm::vec3>& offset) noexcept
    {
        _canvas.setOffset(LuaGlm::tableGet(offset));
        return *this;
    }

    LuaRmluiCanvas& LuaRmluiCanvas::setEnabled(bool enabled) noexcept
    {
        _canvas.setEnabled(enabled);
        return *this;
    }

    bool LuaRmluiCanvas::getEnabled() const noexcept
    {
        return _canvas.getEnabled();
    }

    LuaRmluiCanvas& LuaRmluiCanvas::setInputActive(bool active) noexcept
    {
        _canvas.setInputActive(active);
        return *this;
    }

    bool LuaRmluiCanvas::getInputActive() const noexcept
    {
        return _canvas.getInputActive();
    }

    LuaRmluiCanvas& LuaRmluiCanvas::setMousePosition(const glm::vec2& position) noexcept
    {
        _canvas.setMousePosition(position);
        return *this;
    }

    const glm::vec2& LuaRmluiCanvas::getMousePosition() const noexcept
    {
        return _canvas.getMousePosition();
    }

    LuaRmluiCanvas& LuaRmluiCanvas::setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept
    {
        _canvas.setScrollBehavior(behaviour, speedFactor);
        return *this;
    }

    Rml::ElementDocument& LuaRmluiCanvas::loadDocument(const std::string& name)
    {
        return *_canvas.getContext().LoadDocument(name);
    }

    LuaRmluiCanvas::LuaRmluiCanvas(RmluiCanvas& view) noexcept
        : _canvas(view)
    {
    }

    LuaRmluiCanvas::~LuaRmluiCanvas() noexcept
    {
        auto& ctxt = _canvas.getContext();
        for (auto& listener : _tabEventListeners)
        {
            ctxt.RemoveEventListener(listener->getEvent(), listener.get());
        }
        for (auto& listener : _funcEventListeners)
        {
            ctxt.RemoveEventListener(listener->getEvent(), listener.get());
        }
    }

    LuaRmluiCanvas& LuaRmluiCanvas::addEntityComponent1(LuaEntity& entity, const std::string& name) noexcept
    {
        return entity.addWrapperComponent<LuaRmluiCanvas, RmluiCanvas>(name);
    }

    LuaRmluiCanvas& LuaRmluiCanvas::addEntityComponent2(LuaEntity& entity, const std::string& name, const VarLuaTable<glm::uvec2>& size) noexcept
    {
        return entity.addWrapperComponent<LuaRmluiCanvas, RmluiCanvas>(name, LuaGlm::tableGet(size));
    }

    OptionalRef<LuaRmluiCanvas>::std_t LuaRmluiCanvas::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<LuaRmluiCanvas>();
    }

    std::optional<LuaEntity> LuaRmluiCanvas::getEntity(LuaScene& scene) noexcept
    {
        return scene.getEntity(_canvas);
    }

    void LuaRmluiCanvas::createDataModel(const std::string& name, sol::table table) noexcept
    {
        auto constr = _canvas.getContext().CreateDataModel(name);
        if (!constr)
        {
            return;
        }
        for (auto& [key, val] : table)
        {
            auto str = key.as<std::string>();
            constr.BindFunc(str,
                [table, key](Rml::Variant& var)
                    { LuaRmluiEvent::setRmlVariant(var, table[key]); },
                [table, key](const Rml::Variant& var) mutable
                    { table[key] = LuaRmluiEvent::getRmlVariant(table.lua_state(), var); }
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

    size_t LuaRmluiCanvas::processCustomEvent(Rml::Event& event, const std::string& name, const sol::table& args)
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

    Rml::DataModelHandle LuaRmluiCanvas::getDataModel(const std::string& name) const noexcept
    {
        return _canvas.getContext().GetDataModel(name).GetModelHandle();
    }

    bool LuaRmluiCanvas::removeDataModel(const std::string& name) noexcept
    {
        return _canvas.getContext().RemoveDataModel(name);
    }

    LuaRmluiCanvas& LuaRmluiCanvas::addEventListener1(const std::string& ev, const sol::table& tab) noexcept
    {
        auto ptr = std::make_unique<LuaTableRmluiEventListener>(ev, tab);
        _canvas.getContext().AddEventListener(ev, ptr.get());
        _tabEventListeners.push_back(std::move(ptr));
        return *this;
    }

    bool LuaRmluiCanvas::removeEventListener1(const std::string& ev, const sol::table& tab) noexcept
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

    bool LuaRmluiCanvas::removeEventListener2(const sol::table& tab) noexcept
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

    LuaRmluiCanvas& LuaRmluiCanvas::addEventListener2(const std::string& ev, const sol::protected_function& func) noexcept
    {
        auto ptr = std::make_unique<LuaFunctionRmluiEventListener>(ev, func);
        _canvas.getContext().AddEventListener(ev, ptr.get());
        _funcEventListeners.push_back(std::move(ptr));
        return *this;
    }

    bool LuaRmluiCanvas::removeEventListener4(const std::string& ev, const sol::protected_function& func) noexcept
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

    bool LuaRmluiCanvas::removeEventListener3(const sol::protected_function& func) noexcept
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

    void LuaRmluiCanvas::bind(sol::state_view& lua) noexcept
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
            }
        );

        lua.new_usertype<Rml::DataModelHandle>("RmluiModelHandle", sol::no_constructor,
            "is_dirty", &Rml::DataModelHandle::IsVariableDirty,
            "set_dirty", sol::overload(
                &Rml::DataModelHandle::DirtyVariable,
                &Rml::DataModelHandle::DirtyAllVariables
            )
        );

        lua.new_usertype<LuaRmluiCanvas>("RmluiCanvas", sol::no_constructor,
            "type_id", &entt::type_hash<RmluiCanvas>::value,
            "add_entity_component", sol::overload(
                &LuaRmluiCanvas::addEntityComponent1,
                &LuaRmluiCanvas::addEntityComponent2
            ),
            "get_entity_component", &LuaRmluiCanvas::getEntityComponent,
            "get_entity", &LuaRmluiCanvas::getEntity,

            "name", sol::property(&LuaRmluiCanvas::getName),
            "size", sol::property(&LuaRmluiCanvas::getSize, &LuaRmluiCanvas::setSize),
            "current_size", sol::property(&LuaRmluiCanvas::getCurrentSize),
            "offset", sol::property(&LuaRmluiCanvas::getOffset, &LuaRmluiCanvas::setOffset),
            "input_active", sol::property(&LuaRmluiCanvas::getInputActive, &LuaRmluiCanvas::setInputActive),
            "mouse_position", sol::property(&LuaRmluiCanvas::getMousePosition, &LuaRmluiCanvas::setMousePosition),
            "set_scroll_behavior", &LuaRmluiCanvas::setScrollBehavior,
            "load_document", &LuaRmluiCanvas::loadDocument,
            "create_data_model", &LuaRmluiCanvas::createDataModel,
            "get_data_model", &LuaRmluiCanvas::getDataModel,
            "remove_data_model", &LuaRmluiCanvas::removeDataModel,
            "add_event_listener", sol::overload(&LuaRmluiCanvas::addEventListener1, &LuaRmluiCanvas::addEventListener2),
            "remove_event_listener", sol::overload(
                &LuaRmluiCanvas::removeEventListener1,
                &LuaRmluiCanvas::removeEventListener2,
                &LuaRmluiCanvas::removeEventListener3,
                &LuaRmluiCanvas::removeEventListener4
            )
        );
    }
}