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
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include "glm.hpp"
#include "viewport.hpp"

namespace Rml
{
    class Variant;
    class ElementDocument;
    class DataModelHandle;
}

namespace darmok
{
    class Texture;
    class RmluiRenderer;
    class RmluiCanvas;

    class LuaFunctionRmluiEventListener final : public Rml::EventListener
    {
    public:
        LuaFunctionRmluiEventListener(const sol::protected_function& func) noexcept;
        void ProcessEvent(Rml::Event& event) override;
        void OnDetach(Rml::Element*) noexcept override;
    private:
        sol::protected_function _func;
    };

    class LuaTableRmluiEventListener final : public Rml::EventListener
    {
    public:
        LuaTableRmluiEventListener(const sol::table& tab) noexcept;
        void ProcessEvent(Rml::Event& event) override;
        void OnDetach(Rml::Element*) noexcept override;
    private:
        sol::table _tab;
    };

    class LuaRmluiRenderer;    

    class LuaRmluiEvent final
    {
    public:
        static void setRmlVariant(Rml::Variant& variant, sol::object obj) noexcept;
        static sol::object getRmlVariant(lua_State* lua, const Rml::Variant& variant) noexcept;

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
        static Rml::Element& addEventListener1(Rml::Element& elm, const std::string& ev, const sol::table& tab) noexcept;
        static Rml::Element& addEventListener2(Rml::Element& elm, const std::string& ev, const sol::protected_function& func) noexcept;
        static bool removeEventListener1(Rml::Element& elm, const std::string& ev, const sol::table& tab) noexcept;
        static bool removeEventListener2(Rml::Element& elm, const std::string& ev, const sol::protected_function& func) noexcept;
        static bool removeEventListener3(Rml::Element& elm, const sol::table& tab) noexcept;
        static bool removeEventListener4(Rml::Element& elm, const sol::protected_function& func) noexcept;
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

    class LuaRmluiCanvas final : IRmluiCustomEventListener, IRmluiScriptRunner
    {
    public:
        LuaRmluiCanvas(RmluiCanvas& canvas, LuaEntity& entity, const sol::state_view& lua) noexcept;
        ~LuaRmluiCanvas() noexcept;

        LuaRmluiCanvas(const LuaRmluiCanvas& other) = delete;
        LuaRmluiCanvas& operator=(const LuaRmluiCanvas& other) = delete;

        RmluiCanvas& getReal() noexcept;
        const RmluiCanvas& getReal() const noexcept;

        void update(float deltaTime);

        static void bind(sol::state_view& lua) noexcept;

    private:
        sol::state_view _lua;
        sol::environment _env;
        RmluiCanvas& _canvas;

        static LuaRmluiCanvas& addEntityComponent1(LuaEntity& entity, const std::string& name, sol::this_state ts) noexcept;
        static LuaRmluiCanvas& addEntityComponent2(LuaEntity& entity, const std::string& name, const VarLuaTable<glm::uvec2>& size, sol::this_state ts) noexcept;
        static OptionalRef<LuaRmluiCanvas>::std_t getEntityComponent(LuaEntity& entity) noexcept;
        std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;

        bool recreateDataModel(const std::string& name, sol::table table) noexcept;
        void createDataModel(const std::string& name, sol::table table) noexcept;
        Rml::DataModelHandle getDataModel(const std::string& name) const noexcept;
        bool removeDataModel(const std::string& name) noexcept;

        std::string getName() const noexcept;

        LuaRmluiCanvas& addEventListener1(const std::string& ev, const sol::table& tab) noexcept;
        bool removeEventListener1(const std::string& ev, const sol::table& tab) noexcept;
        bool removeEventListener2(const sol::table& tab) noexcept;

        LuaRmluiCanvas& addEventListener2(const std::string& ev, const sol::protected_function& func) noexcept;
        bool removeEventListener4(const std::string& ev, const sol::protected_function& func) noexcept;
        bool removeEventListener3(const sol::protected_function& func) noexcept;

        std::optional<glm::uvec2> getSize() const noexcept;
        LuaRmluiCanvas& setSize(std::optional<VarLuaTable<glm::uvec2>> size) noexcept;
        glm::uvec2 getCurrentSize() const noexcept;

        const glm::vec3& getOffset() const noexcept;
        LuaRmluiCanvas& setOffset(const VarLuaTable<glm::vec3>& offset) noexcept;

        LuaRmluiCanvas& setVisible(bool visible) noexcept;
        bool getVisible() const noexcept;

        LuaRmluiCanvas& setInputActive(bool active) noexcept;
        bool getInputActive() const noexcept;

        LuaRmluiCanvas& setMousePosition(const glm::vec2& position) noexcept;
        const glm::vec2& getMousePosition() const noexcept;

        using MousePositionMode = RmluiCanvasMousePositionMode;

        LuaRmluiCanvas& setMousePositionMode(MousePositionMode mode) noexcept;
        MousePositionMode getMousePositionMode() const noexcept;

        LuaRmluiCanvas& setViewportMousePosition(const glm::vec2& position) noexcept;
        glm::vec2 getViewportMousePosition() const noexcept;
        LuaRmluiCanvas& applyViewportMousePositionDelta(const glm::vec2& delta) noexcept;

        LuaRmluiCanvas& setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

        OptionalRef<Rml::ElementDocument>::std_t loadDocument(const std::string& name);
        OptionalRef<Rml::ElementDocument>::std_t getDocument(const std::string& name);
        void unloadAllDocuments();
        size_t getNumDocuments() const;
        void unloadDocument(OptionalRef<Rml::ElementDocument>::std_t doc) const;

        sol::environment& getEnvironment() noexcept;

        void onRmluiCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element) noexcept override;
        bool runRmluiScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine) noexcept override;
    };

    class LuaRmluiRenderer final : public ICameraComponent
    {
    public:
        LuaRmluiRenderer(RmluiRenderer& comp) noexcept;

        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) override;

        RmluiRenderer& getReal() noexcept;
        const RmluiRenderer& getReal() const noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        RmluiRenderer& _comp;

        static LuaRmluiRenderer& addCameraComponent(Camera& cam) noexcept;
        static OptionalRef<LuaRmluiRenderer>::std_t getCameraComponent(Camera& cam) noexcept;

        void loadFont1(const std::string& path) noexcept;
        void loadFont2(const std::string& path, bool fallback) noexcept;
    };
}