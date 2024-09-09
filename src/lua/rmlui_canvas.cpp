#include "rmlui.hpp"
#include <RmlUi/Core.h>
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
    LuaRmluiCanvas::LuaRmluiCanvas(RmluiCanvas& view, const sol::state_view& lua) noexcept
        : _canvas(view)
        , _lua(lua)
    {
        _canvas.addCustomEventListener(*this);
    }

    LuaRmluiCanvas::~LuaRmluiCanvas() noexcept
    {
        _canvas.removeCustomEventListener(*this);
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
        return _canvas.isEnabled();
    }

    LuaRmluiCanvas& LuaRmluiCanvas::setInputActive(bool active) noexcept
    {
        _canvas.setInputActive(active);
        return *this;
    }

    bool LuaRmluiCanvas::getInputActive() const noexcept
    {
        return _canvas.isInputActive();
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

    LuaRmluiCanvas& LuaRmluiCanvas::setMousePositionMode(MousePositionMode mode) noexcept
    {
        _canvas.setMousePositionMode(mode);
        return *this;
    }

    LuaRmluiCanvas::MousePositionMode LuaRmluiCanvas::getMousePositionMode() const noexcept
    {
        return _canvas.getMousePositionMode();
    }

    LuaRmluiCanvas& LuaRmluiCanvas::setViewportMousePosition(const glm::vec2& position) noexcept
    {
        _canvas.setViewportMousePosition(position);
        return *this;
    }

    glm::vec2 LuaRmluiCanvas::getViewportMousePosition() const noexcept
    {
        return _canvas.getViewportMousePosition();
    }

    LuaRmluiCanvas& LuaRmluiCanvas::applyViewportMousePositionDelta(const glm::vec2& delta) noexcept
    {
        _canvas.applyViewportMousePositionDelta(delta);
        return *this;
    }


    LuaRmluiCanvas& LuaRmluiCanvas::setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept
    {
        _canvas.setScrollBehavior(behaviour, speedFactor);
        return *this;
    }

    OptionalRef<Rml::ElementDocument>::std_t LuaRmluiCanvas::loadDocument(const std::string& name)
    {
        if (auto doc = _canvas.getContext().LoadDocument(name))
        {
            return *doc;
        }
        return std::nullopt;
    }

    OptionalRef<Rml::ElementDocument>::std_t LuaRmluiCanvas::getDocument(const std::string& name)
    {
        if (auto doc = _canvas.getContext().GetDocument(name))
        {
            return *doc;
        }
        return std::nullopt;
    }

    void LuaRmluiCanvas::unloadAllDocuments()
    {
        _canvas.getContext().UnloadAllDocuments();
    }

    size_t LuaRmluiCanvas::getNumDocuments() const
    {
        return _canvas.getContext().GetNumDocuments();
    }

    void LuaRmluiCanvas::unloadDocument(OptionalRef<Rml::ElementDocument>::std_t doc) const
    {
        if (doc)
        {
            _canvas.getContext().UnloadDocument(&doc.value().get());
        }
    }

    RmluiCanvas& LuaRmluiCanvas::getReal() noexcept
    {
        return _canvas;
    }

    const RmluiCanvas& LuaRmluiCanvas::getReal() const noexcept
    {
        return _canvas;
    }

    LuaRmluiCanvas& LuaRmluiCanvas::addEntityComponent1(LuaEntity& entity, const std::string& name, sol::this_state ts) noexcept
    {
        auto& real = entity.addComponent<RmluiCanvas>(name);
        return entity.addComponent<LuaRmluiCanvas>(real, ts);
    }

    LuaRmluiCanvas& LuaRmluiCanvas::addEntityComponent2(LuaEntity& entity, const std::string& name, const VarLuaTable<glm::uvec2>& size, sol::this_state ts) noexcept
    {
        auto& real = entity.addComponent<RmluiCanvas>(name, LuaGlm::tableGet(size));
        return entity.addComponent<LuaRmluiCanvas>(real, ts);
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

    void LuaRmluiCanvas::onRmluiCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element)
    {
        auto params = StringUtils::split(value, ":");
        auto name = params[0];
        auto args = _lua.create_table();
        for (size_t i = 1; i < params.size(); ++i)
        {
            args[i] = params[i];
        }

        for (auto& listener : _funcEventListeners)
        {
            if (listener->getEvent() == name)
            {
                listener->processCustomEvent(event, args, element);
            }
        }
        for (auto& listener : _tabEventListeners)
        {
            if (listener->getEvent() == name)
            {
                listener->processCustomEvent(event, args, element);
            }
        }
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
        lua.new_enum<Rml::ScrollBehavior>("RmluiScrollBehavior", {
            { "Auto", Rml::ScrollBehavior::Auto },
            { "Smooth", Rml::ScrollBehavior::Smooth },
            { "Instant", Rml::ScrollBehavior::Instant }
        });

        lua.new_enum<RmluiCanvasMousePositionMode>("RmluiCanvasMousePositionMode", {
            { "Absolute", RmluiCanvasMousePositionMode::Absolute },
            { "Relative", RmluiCanvasMousePositionMode::Relative },
            { "Disabled", RmluiCanvasMousePositionMode::Disabled }
        });


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
            "enabled", sol::property(&LuaRmluiCanvas::getEnabled, &LuaRmluiCanvas::setEnabled),
            "current_size", sol::property(&LuaRmluiCanvas::getCurrentSize),
            "offset", sol::property(&LuaRmluiCanvas::getOffset, &LuaRmluiCanvas::setOffset),
            "input_active", sol::property(&LuaRmluiCanvas::getInputActive, &LuaRmluiCanvas::setInputActive),
            "mouse_position", sol::property(&LuaRmluiCanvas::getMousePosition, &LuaRmluiCanvas::setMousePosition),
            "mouse_position_mode", sol::property(&LuaRmluiCanvas::getMousePositionMode, &LuaRmluiCanvas::setMousePositionMode),
            "viewport_mouse_position", sol::property(&LuaRmluiCanvas::getViewportMousePosition, &LuaRmluiCanvas::setViewportMousePosition),
            "apply_viewport_mouse_position_delta", &LuaRmluiCanvas::applyViewportMousePositionDelta,
            "set_scroll_behavior", &LuaRmluiCanvas::setScrollBehavior,
            "load_document", &LuaRmluiCanvas::loadDocument,
            "get_document", &LuaRmluiCanvas::getDocument,
            "unload_all_documents", & LuaRmluiCanvas::unloadAllDocuments,
            "unload_document", & LuaRmluiCanvas::unloadDocument,
            "num_documents", sol::property(&LuaRmluiCanvas::getNumDocuments),
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