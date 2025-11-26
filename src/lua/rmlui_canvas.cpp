#include "lua/rmlui.hpp"
#include "lua/camera.hpp"
#include "lua/utils.hpp"
#include "lua/scene.hpp"
#include "lua/glm.hpp"
#include "lua/viewport.hpp"
#include "detail/rmlui.hpp"

#include <darmok/rmlui.hpp>
#include <darmok/viewport.hpp>
#include <darmok/texture.hpp>
#include <darmok/string.hpp>

#include <RmlUi/Core.h>
#include <RmlUi/Core/DataModelHandle.h>


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

    void LuaRmluiCanvasDelegate::update(float deltaTime)
    {
        static const std::string updateFunc = "update";
        if (sol::safe_function update = _env[updateFunc])
        {
            auto result = update(deltaTime);
            if (LuaUtils::checkResult<bool>(result, "running rmlui canvas update"))
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

        auto result = _lua.safe_script(value, env);
        auto logDesc = "running rmlui event " + ev.GetType() + " on element " + element.GetAddress();
        LuaUtils::checkResult(result, logDesc);
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
        auto result = _lua.safe_script(buffer, _env);
        LuaUtils::checkResult(result, logDesc);

        return true;
    }

    OptionalRef<Camera>::std_t LuaRmluiCanvas::getCamera(const RmluiCanvas& canvas) noexcept
    {
        return canvas.getCamera();
    }

    void LuaRmluiCanvas::setCamera(RmluiCanvas& canvas, OptionalRef<Camera>::std_t cam) noexcept
    {
        canvas.setCamera(cam ? &cam->get() : nullptr);
    }

    OptionalRef<Camera>::std_t LuaRmluiCanvas::getCurrentCamera(const RmluiCanvas& canvas) noexcept
    {
        return canvas.getCurrentCamera();
    }

    sol::environment LuaRmluiCanvas::getEnvironment(const RmluiCanvas& canvas) noexcept
    {
        static const entt::id_type type = entt::type_hash<LuaRmluiCanvasDelegate>::value();
        if (auto dlg = canvas.getDelegate())
        {
            if (dlg->getRmluiCanvasDelegateType() == type)
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

    bool LuaRmluiCanvas::update(RmluiCanvas& canvas)
    {
        return canvas.getContext().Update();
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

    std::optional<LuaEntity> LuaRmluiCanvas::getEntity(const RmluiCanvas& canvas, std::shared_ptr<Scene>& scene) noexcept
    {
        return LuaScene::getEntity(scene, canvas);
    }

    Rml::DataModelConstructor LuaRmluiCanvas::recreateDataModel(RmluiCanvas& canvas, const std::string& name, sol::table table) noexcept
    {
        removeDataModel(canvas, name);
        return createDataModel(canvas, name, table);
    }

    Rml::DataModelConstructor LuaRmluiCanvas::createDataModel(RmluiCanvas& canvas, const std::string& name, sol::table table)
    {
        auto lua = table.lua_state();

        auto construct = canvas.createDataModel(name);
        auto scalarDefPtr = Rml::MakeUnique<LuaRmluiVariableDefinition>(table, Rml::DataVariableType::Scalar);
        auto structDefPtr = Rml::MakeUnique<LuaRmluiVariableDefinition>(table, Rml::DataVariableType::Struct);
        auto arrayDefPtr = Rml::MakeUnique<LuaRmluiVariableDefinition>(table, Rml::DataVariableType::Array);
        auto scalarDef = scalarDefPtr.get();
        auto structDef = structDefPtr.get();
        auto arrayDef = arrayDefPtr.get();
        auto typeReg = construct.GetDataTypeRegister();
        typeReg->RegisterDefinition(Rml::FamilyId(1), std::move(scalarDefPtr));
        typeReg->RegisterDefinition(Rml::FamilyId(2), std::move(structDefPtr));
        typeReg->RegisterDefinition(Rml::FamilyId(3), std::move(arrayDefPtr));

        for (auto& [key, val] : table)
        {
            auto strkey = key.as<std::string>();
            auto type = val.get_type();
            if (type == sol::type::function)
            {
                auto func = val.as<sol::protected_function>();

                construct.BindEventCallback(strkey, [func, lua](Rml::DataModelHandle handle, Rml::Event& ev, const Rml::VariantList& variants)
                {
                    std::vector<sol::object> args{
                        sol::make_object(lua, handle),
                        sol::make_object(lua, std::ref(ev))
                    };
                    LuaRmluiUtils::getVariantList(lua, args, variants);
                    func(sol::as_args(args));
                });
            }
            else
            {
                OptionalRef<LuaRmluiVariableDefinition> def = scalarDef;
                if (type == sol::type::table)
                {
                    def = LuaUtils::isArray(val) ? arrayDef : structDef;
                }
                auto ptr = def->getKeyPointer({ strkey });
                construct.BindCustomDataVariable(strkey, Rml::DataVariable(def.ptr(), ptr));
            }
        }

        table["handle"] = construct.GetModelHandle();
        return construct;
    }

    Rml::DataModelConstructor LuaRmluiCanvas::getDataModel(RmluiCanvas& canvas, const std::string& name) noexcept
    {
        return canvas.getDataModel(name);
    }

    bool LuaRmluiCanvas::removeDataModel(RmluiCanvas& canvas, const std::string& name) noexcept
    {
        return canvas.removeDataModel(name);
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

        lua.new_usertype<Rml::DataModelHandle>("RmluiDataModelHandle", sol::no_constructor,
            "is_dirty", &Rml::DataModelHandle::IsVariableDirty,
            "set_dirty", sol::overload(
                &Rml::DataModelHandle::DirtyVariable,
                &Rml::DataModelHandle::DirtyAllVariables
            )
        );

        lua.new_usertype<Rml::DataModelConstructor>("RmluiDataModelConstructor", sol::no_constructor,
            "register_transform", [](Rml::DataModelConstructor& constuctor, const std::string& name, const sol::protected_function& func)
            {
                constuctor.RegisterTransformFunc(name, [func](const Rml::VariantList& variants)
                {
                    std::vector<sol::object> args;
                    LuaRmluiUtils::getVariantList(func.lua_state(), args, variants);
                    sol::object obj = func(sol::as_args(args));
                    Rml::Variant r;
                    LuaRmluiUtils::setVariant(r, obj);
                    return r;
                });
                return constuctor;
            }
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
            "camera", sol::property(&LuaRmluiCanvas::getCamera, &LuaRmluiCanvas::setCamera),
            "current_camera", sol::property(&LuaRmluiCanvas::getCurrentCamera),
            "size", sol::property(&LuaRmluiCanvas::getSize, &RmluiCanvas::setSize),
            "visible", sol::property(&RmluiCanvas::isVisible, &RmluiCanvas::setVisible),
            "current_size", sol::property(&RmluiCanvas::getCurrentSize),
            "offset", sol::property(&RmluiCanvas::getOffset, &LuaRmluiCanvas::setOffset),
            "input_enabled", sol::property(&RmluiCanvas::isInputEnabled, &RmluiCanvas::setInputEnabled),
            "default_texture_flags", sol::property(&RmluiCanvas::getDefaultTextureFlags, &RmluiCanvas::setDefaultTextureFlags),
            "get_texture_flags", &RmluiCanvas::getTextureFlags,
            "set_texture_flags", &RmluiCanvas::setTextureFlags,
            "mouse_position", sol::property(&RmluiCanvas::getMousePosition, &LuaRmluiCanvas::setMousePosition),
            "mouse_position_mode", sol::property(&RmluiCanvas::getMousePositionMode, &RmluiCanvas::setMousePositionMode),
            "viewport_mouse_position", sol::property(&RmluiCanvas::getViewportMousePosition, &LuaRmluiCanvas::setViewportMousePosition),
            "apply_viewport_mouse_position_delta", &LuaRmluiCanvas::applyViewportMousePositionDelta,
            "set_scroll_behavior", &RmluiCanvas::setScrollBehavior,
            "update", &LuaRmluiCanvas::update,
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