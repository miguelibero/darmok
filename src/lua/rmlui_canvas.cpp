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
    LuaRmluiCanvasDelegate::LuaRmluiCanvasDelegate(RmluiCanvas& canvas, LuaEntity& entity, const sol::state_view& lua) noexcept
        : _canvas(canvas)
        , _lua(lua)
        , _env(lua, sol::create, _lua.globals())
    {
        _env["canvas"] = this;
        _env["entity"] = entity;
    }

    LuaRmluiCanvasDelegate::~LuaRmluiCanvasDelegate()
    {
        sol::safe_function shutdown = _env["shutdown"];
        if (shutdown)
        {
            auto result = shutdown();
            if (!result.valid())
            {
                LuaUtils::logError("running shutdown", result);
            }
        }
    }

    sol::environment& LuaRmluiCanvasDelegate::getEnvironment() noexcept
    {
        return _env;
    }

    entt::id_type LuaRmluiCanvasDelegate::getType() const noexcept
    {
        return entt::type_hash<LuaRmluiCanvasDelegate>::value();
    }

    void LuaRmluiCanvasDelegate::update(float deltaTime)
    {
        static const std::string updateFunc = "update";
        if (sol::safe_function update = _env[updateFunc])
        {
            auto result = update(deltaTime);
            auto finished = LuaUtils::checkResult("running rmlui canvas update", result);
            if (finished)
            {
                _lua[updateFunc] = nullptr;
            }
        }
    }

    void LuaRmluiCanvasDelegate::onRmluiCustomEvent(Rml::Event& ev, const std::string& value, Rml::Element& element) noexcept
    {
        sol::environment env(_lua, sol::create, _env);
        env["event"] = std::ref(ev);
        env["element"] = std::ref(element);
        env["document"] = element.GetOwnerDocument();

        auto r = _lua.safe_script(value, env);
        auto logDesc = "running rmlui event " + ev.GetType() + " on element " + element.GetAddress();
        LuaUtils::checkResult(logDesc, r);
    }

    bool LuaRmluiCanvasDelegate::loadRmluiScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine) noexcept
    {
        _env["document"] = std::ref(doc);

        std::string buffer = "--";
        buffer += sourcePath;
        auto logDesc = "running rmlui script " + std::string(sourcePath);
        if (sourceLine >= 0)
        {
            auto sourceLineStr = std::to_string(sourceLine);
            buffer += ":" + sourceLineStr;
            logDesc += ":" + sourceLineStr;
        }

        buffer += "\n" + std::string(content);
        auto r = _lua.safe_script(buffer, _env);
        LuaUtils::checkResult(logDesc, r);

        return true;
    }

    sol::environment LuaRmluiCanvas::getEnvironment(const RmluiCanvas& canvas) noexcept
    {
        if (auto dlg = canvas.getDelegate())
        {
            if (dlg->getType() == entt::type_hash<LuaRmluiCanvasDelegate>::value())
            {
                return static_cast<LuaRmluiCanvasDelegate&>(dlg.value()).getEnvironment();
            }
        }
        return {};
    }

    std::optional<glm::uvec2> LuaRmluiCanvas::getSize(const RmluiCanvas& canvas) noexcept
    {
        return canvas.getSize();
    }

    void LuaRmluiCanvas::setSize(RmluiCanvas& canvas, std::optional<VarLuaTable<glm::uvec2>> size) noexcept
    {
        if (size)
        {
            canvas.setSize(LuaGlm::tableGet(size.value()));
        }
        else
        {
            canvas.setSize(std::nullopt);
        }
    }

    void LuaRmluiCanvas::setOffset(RmluiCanvas& canvas, const VarLuaTable<glm::vec3>& offset) noexcept
    {
        canvas.setOffset(LuaGlm::tableGet(offset));
    }

    void LuaRmluiCanvas::setMousePosition(RmluiCanvas& canvas, const VarLuaTable<glm::vec2>& position) noexcept
    {
        canvas.setMousePosition(LuaGlm::tableGet(position));
    }

    void LuaRmluiCanvas::setViewportMousePosition(RmluiCanvas& canvas, const VarLuaTable<glm::vec2>& position) noexcept
    {
        canvas.setViewportMousePosition(LuaGlm::tableGet(position));
    }

    RmluiCanvas& LuaRmluiCanvas::applyViewportMousePositionDelta(RmluiCanvas& canvas, const VarLuaTable<glm::vec2>& delta) noexcept
    {
        return canvas.applyViewportMousePositionDelta(LuaGlm::tableGet(delta));
    }

    OptionalRef<Rml::ElementDocument>::std_t LuaRmluiCanvas::loadDocument(RmluiCanvas& canvas, const std::string& name)
    {
        if (auto doc = canvas.getContext().LoadDocument(name))
        {
            return *doc;
        }
        return std::nullopt;
    }

    OptionalRef<Rml::ElementDocument>::std_t LuaRmluiCanvas::getDocument(RmluiCanvas& canvas, const std::string& name)
    {
        if (auto doc = canvas.getContext().GetDocument(name))
        {
            return *doc;
        }
        return std::nullopt;
    }

    RmluiCanvas& LuaRmluiCanvas::unloadAllDocuments(RmluiCanvas& canvas)
    {
        canvas.getContext().UnloadAllDocuments();
        return canvas;
    }

    size_t LuaRmluiCanvas::getNumDocuments(const RmluiCanvas& canvas)
    {
        return canvas.getContext().GetNumDocuments();
    }

    RmluiCanvas& LuaRmluiCanvas::unloadDocument(RmluiCanvas& canvas, OptionalRef<Rml::ElementDocument>::std_t doc)
    {
        if (doc)
        {
            canvas.getContext().UnloadDocument(&doc.value().get());
        }
        return canvas;
    }

    void LuaRmluiCanvas::configureCanvas(RmluiCanvas& canvas, LuaEntity& entity, const sol::state_view& lua) noexcept
    {
        canvas.setDelegate(std::make_unique<LuaRmluiCanvasDelegate>(canvas, entity, lua));
    }

    RmluiCanvas& LuaRmluiCanvas::addEntityComponent1(LuaEntity& entity, const std::string& name, sol::this_state ts) noexcept
    {
        auto& canvas = entity.addComponent<RmluiCanvas>(name);
        configureCanvas(canvas, entity, ts);
        return canvas;
    }

    RmluiCanvas& LuaRmluiCanvas::addEntityComponent2(LuaEntity& entity, const std::string& name, const VarLuaTable<glm::uvec2>& size, sol::this_state ts) noexcept
    {
        auto& canvas = entity.addComponent<RmluiCanvas>(name, LuaGlm::tableGet(size));
        configureCanvas(canvas, entity, ts);
        return canvas;
    }

    OptionalRef<RmluiCanvas>::std_t LuaRmluiCanvas::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<RmluiCanvas>();
    }

    std::optional<LuaEntity> LuaRmluiCanvas::getEntity(const RmluiCanvas& canvas, LuaScene& scene) noexcept
    {
        return scene.getEntity(canvas);
    }

    bool LuaRmluiCanvas::recreateDataModel(RmluiCanvas& canvas, const std::string& name, sol::table table) noexcept
    {
        auto removed = removeDataModel(canvas, name);
        createDataModel(canvas, name, table);
        return removed;
    }

    void LuaRmluiCanvas::createDataModel(RmluiCanvas& canvas, const std::string& name, sol::table table) noexcept
    {
        auto constr = canvas.getContext().CreateDataModel(name);
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

    Rml::DataModelHandle LuaRmluiCanvas::getDataModel(RmluiCanvas& canvas, const std::string& name) noexcept
    {
        return canvas.getContext().GetDataModel(name).GetModelHandle();
    }

    bool LuaRmluiCanvas::removeDataModel(RmluiCanvas& canvas, const std::string& name) noexcept
    {
        return canvas.getContext().RemoveDataModel(name);
    }

    RmluiCanvas& LuaRmluiCanvas::addEventListener(RmluiCanvas& canvas, const std::string& ev, const sol::object& obj) noexcept
    {
        canvas.getContext().AddEventListener(ev, new LuaRmluiEventListener(obj));
        return canvas;
    }

    bool LuaRmluiCanvas::removeEventListener(RmluiCanvas& canvas, const std::string& ev, const sol::object& obj) noexcept
    {
        // TODO: find a way of doing this
        return false;
    }

    void LuaRmluiCanvas::bind(sol::state_view& lua) noexcept
    {
        Scene::registerComponentDependency<RmluiCanvas, LuaRmluiCanvas>();

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

        lua.new_usertype<RmluiCanvas>("RmluiCanvas", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<RmluiCanvas>::value),
            "add_entity_component", sol::overload(
                &LuaRmluiCanvas::addEntityComponent1,
                &LuaRmluiCanvas::addEntityComponent2
            ),
            "get_entity_component", &LuaRmluiCanvas::getEntityComponent,
            "get_entity", &LuaRmluiCanvas::getEntity,
            "name", sol::property(&RmluiCanvas::getName),
            "environment", sol::property(&LuaRmluiCanvas::getEnvironment),
            "size", sol::property(&LuaRmluiCanvas::getSize, &LuaRmluiCanvas::setSize),
            "visible", sol::property(&RmluiCanvas::isVisible, &RmluiCanvas::setVisible),
            "current_size", sol::property(&RmluiCanvas::getCurrentSize),
            "offset", sol::property(&RmluiCanvas::getOffset, &RmluiCanvas::setOffset),
            "input_active", sol::property(&RmluiCanvas::isInputActive, &RmluiCanvas::setInputActive),
            "mouse_position", sol::property(&RmluiCanvas::getMousePosition, &LuaRmluiCanvas::setMousePosition),
            "mouse_position_mode", sol::property(&RmluiCanvas::getMousePositionMode, &RmluiCanvas::setMousePositionMode),
            "viewport_mouse_position", sol::property(&RmluiCanvas::getViewportMousePosition, &LuaRmluiCanvas::setViewportMousePosition),
            "apply_viewport_mouse_position_delta", &LuaRmluiCanvas::applyViewportMousePositionDelta,
            "set_scroll_behavior", &RmluiCanvas::setScrollBehavior,
            "load_document", &LuaRmluiCanvas::loadDocument,
            "get_document", &LuaRmluiCanvas::getDocument,
            "unload_all_documents", & LuaRmluiCanvas::unloadAllDocuments,
            "unload_document", & LuaRmluiCanvas::unloadDocument,
            "num_documents", sol::property(&LuaRmluiCanvas::getNumDocuments),
            "create_data_model", &LuaRmluiCanvas::createDataModel,
            "recreate_data_model", & LuaRmluiCanvas::recreateDataModel,
            "get_data_model", &LuaRmluiCanvas::getDataModel,
            "remove_data_model", &LuaRmluiCanvas::removeDataModel,
            "add_event_listener", &LuaRmluiCanvas::addEventListener,
            "remove_event_listener", &LuaRmluiCanvas::removeEventListener
        );
    }
}