#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/glm.hpp>
#include <darmok/rmlui.hpp>
#include <darmok/camera.hpp>
#include <RmlUi/Core/ScrollTypes.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/EventListenerInstancer.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/DataVariable.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <variant>
#include "glm.hpp"
#include "viewport.hpp"
#include "utils.hpp"

namespace Rml
{
    class Variant;
    class ElementDocument;
    class DataModelHandle;
    class DataModelConstructor;
}

namespace darmok
{
    struct LuaRmluiUtils final
    {
        static bool setVariant(Rml::Variant& variant, const sol::object& obj) noexcept;
        static bool getVariant(sol::object& obj, const Rml::Variant& variant) noexcept;
        static void getVariantList(lua_State* lua, std::vector<sol::object>& objs, const Rml::VariantList& variants) noexcept;
        static Rml::DataVariableType getDataVariableType(const sol::object& obj) noexcept;
    };

    class Texture;
    class RmluiRenderer;
    class RmluiCanvas;

    class LuaRmluiEventListener final : public Rml::EventListener
    {
    public:
        LuaRmluiEventListener(const sol::object& obj) noexcept;
        void ProcessEvent(Rml::Event& event) noexcept override;
        void OnDetach(Rml::Element* element) noexcept override;
    private:
        LuaDelegate _delegate;
    };

    class LuaRmluiRenderer;

    class LuaRmluiVariableDefinition final : public Rml::VariableDefinition
    {
    public:
        LuaRmluiVariableDefinition(const sol::table& table) noexcept;
        bool Get(void* ptr, Rml::Variant& variant) noexcept override;
        bool Set(void* ptr, const Rml::Variant& variant) noexcept override;
        int Size(void* ptr) noexcept override;
        Rml::DataVariable Child(void* ptr, const Rml::DataAddressEntry& address) noexcept override;

        using TableKey = std::variant<int, std::string>;
        using AbsTableKey = std::vector<TableKey>;

        void* getKeyPointer(const AbsTableKey& key) noexcept;
    private:
        sol::table _table;

        std::vector<AbsTableKey> _keys;

        sol::object getPointerObject(void* ptr) const noexcept;
        OptionalRef<const AbsTableKey> getPointerKey(void* ptr) const noexcept;
        sol::object getTableValue(const AbsTableKey& key) const noexcept;
    };

    class LuaRmluiEvent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static glm::vec2 getUnprojectedMouseScreenPosition(const Rml::Event& ev) noexcept;
        static sol::table getParameters(const Rml::Event& ev, sol::this_state ts) noexcept;
        static sol::object getParameter(const Rml::Event& ev, const std::string& key, sol::this_state ts) noexcept;
        static std::string toString(const Rml::Event& ev) noexcept;
    };

    class LuaRmluiElement
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static std::optional<std::string> getProperty(Rml::Element& elm, const std::string& name) noexcept;
        static bool hasAttribute(const Rml::Element& elm, const std::string& name) noexcept;
        static sol::object getAttribute(const Rml::Element& elm, const std::string& name, sol::this_state ts) noexcept;
        static void setAttribute(Rml::Element& elm, const std::string& name, const sol::object& val);
        static std::vector<Rml::Element*> getElementsByClassName(Rml::Element& elm, const std::string& cls) noexcept;
        static std::vector<Rml::Element*> getElementsByTagName(Rml::Element& elm, const std::string& tag) noexcept;
        static std::vector<Rml::Element*> querySelectorAll(Rml::Element& elm, const std::string& selector) noexcept;
        static int getNumChildren(Rml::Element& elm) noexcept;
        static Rml::Element& addEventListener(Rml::Element& elm, const std::string& ev, const sol::object& obj) noexcept;
        static bool removeEventListener(Rml::Element& elm, const std::string& ev, const sol::object& obj) noexcept;
        static bool dispatchEvent1(Rml::Element& elm, const std::string& type, const sol::table& params);
        static bool dispatchEvent2(Rml::Element& elm, const std::string& type, const sol::table& params, bool interruptible);
        static bool dispatchEvent3(Rml::Element& elm, const std::string& type, const sol::table& params, bool interruptible, bool bubbles);
        static bool focus1(Rml::Element& elm) noexcept;
        static bool focus2(Rml::Element& elm, bool visible);
        static glm::vec2 getOffset(Rml::Element& elm);
        static void setOffset1(Rml::Element& elm, const VarLuaTable<glm::vec2>& offset);
        static void setOffset2(Rml::Element& elm, const VarLuaTable<glm::vec2>& offset, Rml::Element& parent);
        static void setOffset3(Rml::Element& elm, const VarLuaTable<glm::vec2>& offset, Rml::Element& parent, bool fixed);
        static glm::vec2 getSize(Rml::Element& elm);
        static void setSize(Rml::Element& elm, const glm::vec2& size);
    };

    class LuaRmluiStyleSheet
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    };

    class LuaRmluiElementDocument
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static void show1(Rml::ElementDocument& doc) noexcept;
        static void show2(Rml::ElementDocument& doc, Rml::ModalFlag modal) noexcept;
        static void show3(Rml::ElementDocument& doc, Rml::ModalFlag modal, Rml::FocusFlag focus) noexcept;
    };

    class LuaEntity;
    class LuaScene;

    class LuaRmluiCanvasDelegate final : public IRmluiCanvasDelegate
    {
    public:
        LuaRmluiCanvasDelegate(RmluiCanvas& canvas, LuaEntity& entity, const sol::state_view& lua) noexcept;
        ~LuaRmluiCanvasDelegate();

        sol::environment& getEnvironment() noexcept;

        entt::id_type getType() const noexcept override;
        void update(float deltaTime) override;
        void onRmluiCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element) noexcept override;
        bool loadRmluiScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine) noexcept override;
    private:
        sol::state_view _lua;
        sol::environment _env;
        RmluiCanvas& _canvas;
    };

    class LuaRmluiCanvas final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static void configureCanvas(RmluiCanvas& canvas, LuaEntity& entity, const sol::state_view& lua) noexcept;
        static RmluiCanvas& addEntityComponent1(LuaEntity& entity, const std::string& name, sol::this_state ts) noexcept;
        static RmluiCanvas& addEntityComponent2(LuaEntity& entity, const std::string& name, const VarLuaTable<glm::uvec2>& size, sol::this_state ts) noexcept;
        static OptionalRef<RmluiCanvas>::std_t getEntityComponent(LuaEntity& entity) noexcept;
        static std::optional<LuaEntity> getEntity(const RmluiCanvas& canvas, LuaScene& scene) noexcept;

        static sol::environment getEnvironment(const RmluiCanvas& canvas) noexcept;
        static std::optional<glm::uvec2> getSize(const RmluiCanvas& canvas) noexcept;

        static Rml::DataModelConstructor recreateDataModel(RmluiCanvas& canvas, const std::string& name, sol::table table) noexcept;
        static Rml::DataModelConstructor createDataModel(RmluiCanvas& canvas, const std::string& name, sol::table table);
        static Rml::DataModelConstructor getDataModel(RmluiCanvas& canvas, const std::string& name) noexcept;
        static bool removeDataModel(RmluiCanvas& canvas, const std::string& name) noexcept;

        static RmluiCanvas& addEventListener(RmluiCanvas& canvas, const std::string& ev, const sol::object& obj) noexcept;
        static bool removeEventListener(RmluiCanvas& canvas, const std::string& ev, const sol::object& obj) noexcept;

        static void setSize(RmluiCanvas& canvas, std::optional<VarLuaTable<glm::uvec2>> size) noexcept;
        static void setOffset(RmluiCanvas& canvas, const VarLuaTable<glm::vec3>& offset) noexcept;

        static void setMousePosition(RmluiCanvas& canvas, const VarLuaTable<glm::vec2>& position) noexcept;

        static void setViewportMousePosition(RmluiCanvas& canvas, const VarLuaTable<glm::vec2>& position) noexcept;
        static RmluiCanvas& applyViewportMousePositionDelta(RmluiCanvas& canvas, const VarLuaTable<glm::vec2>& delta) noexcept;

        static OptionalRef<Rml::ElementDocument>::std_t loadDocument(RmluiCanvas& canvas, const std::string& name);
        static OptionalRef<Rml::ElementDocument>::std_t getDocument(RmluiCanvas& canvas, const std::string& name);
        static RmluiCanvas& unloadDocument(RmluiCanvas& canvas, OptionalRef<Rml::ElementDocument>::std_t doc);
        static RmluiCanvas& unloadAllDocuments(RmluiCanvas& canvas);
        static size_t getNumDocuments(const RmluiCanvas& canvas);
    };

    class LuaRmluiSceneComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static RmluiSceneComponent& addSceneComponent(LuaScene& scene) noexcept;
        static OptionalRef<RmluiSceneComponent>::std_t getSceneComponent(LuaScene& scene) noexcept;
    };

    class LuaRmluiRenderer final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static RmluiRenderer& addCameraComponent(Camera& cam) noexcept;
        static OptionalRef<RmluiRenderer>::std_t getCameraComponent(Camera& cam) noexcept;

        static void loadFont1(const std::string& path) noexcept;
        static void loadFont2(const std::string& path, bool fallback) noexcept;
    };
}