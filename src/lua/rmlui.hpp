#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/glm.hpp>
#include <darmok/viewport.hpp>
#include <RmlUi/Core/ScrollTypes.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/EventListenerInstancer.h>
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
        LuaFunctionRmluiEventListener(const std::string& ev, const sol::protected_function& func) noexcept;
        void ProcessEvent(Rml::Event& event) override;
        void processCustomEvent(Rml::Event& event, const std::string& name, const sol::table& args);
        const sol::protected_function& getFunction() const;
        const std::string& getEvent() const;
    private:
        std::string _event;
        sol::protected_function _func;
    };

    class LuaTableRmluiEventListener final : public Rml::EventListener
    {
    public:
        LuaTableRmluiEventListener(const std::string& ev, const sol::table& tab) noexcept;
        void ProcessEvent(Rml::Event& event) override;
        void processCustomEvent(Rml::Event& event, const std::string& name, const sol::table& args);
        const sol::table& getTable() const;
        const std::string& getEvent() const;
    private:
        std::string _event;
        sol::table _tab;
    };

    class LuaRmluiRenderer;

    class LuaCustomRmluiEventListener final : public Rml::EventListener
    {
    public:
        LuaCustomRmluiEventListener(const std::string& value, LuaRmluiRenderer& comp) noexcept;
        void ProcessEvent(Rml::Event& event) override;
    private:
        std::vector<std::string> _params;
        LuaRmluiRenderer& _comp;
    };

    class LuaRmluiEvent final
    {
    public:
        static void setRmlVariant(Rml::Variant& variant, sol::object obj) noexcept;
        static sol::object getRmlVariant(lua_State* lua, const Rml::Variant& variant) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    };

    class LuaEntity;

    class LuaRmluiCanvas final
    {
    public:
        LuaRmluiCanvas(RmluiCanvas& canvas) noexcept;
        ~LuaRmluiCanvas() noexcept;

        LuaRmluiCanvas(const LuaRmluiCanvas& other) = delete;
        LuaRmluiCanvas& operator=(const LuaRmluiCanvas& other) = delete;

        static void bind(sol::state_view& lua) noexcept;

        size_t processCustomEvent(Rml::Event& event, const std::string& name, const sol::table& args);

    private:
        RmluiCanvas& _canvas;
        std::vector<std::unique_ptr<LuaTableRmluiEventListener>> _tabEventListeners;
        std::vector<std::unique_ptr<LuaFunctionRmluiEventListener>> _funcEventListeners;

        static LuaRmluiCanvas& addEntityComponent1(LuaEntity& entity, const std::string& name) noexcept;
        static LuaRmluiCanvas& addEntityComponent2(LuaEntity& entity, const std::string& name, const VarLuaTable<glm::uvec2>& size) noexcept;

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

        std::optional<Viewport> getViewport() const noexcept;
        LuaRmluiCanvas& setViewport(std::optional<VarViewport> vp) noexcept;
        Viewport getCurrentViewport() const noexcept;

        LuaRmluiCanvas& setEnabled(bool enabled) noexcept;
        bool getEnabled() const noexcept;

        LuaRmluiCanvas& setInputActive(bool active) noexcept;
        bool getInputActive() const noexcept;

        LuaRmluiCanvas& setMousePosition(const glm::vec2& position) noexcept;
        const glm::vec2& getMousePosition() const noexcept;

        LuaRmluiCanvas& setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

        Rml::ElementDocument& loadDocument(const std::string& name);
    };

    class LuaRmluiRenderer final : public ICameraComponent, public Rml::EventListenerInstancer
    {
    public:
        LuaRmluiRenderer(RmluiRenderer& comp, const sol::state_view& lua) noexcept;

        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;

        Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override;
        void processCustomEvent(Rml::Event& event, const std::vector<std::string>& params);

        RmluiRenderer& getReal() noexcept;
        const RmluiRenderer& getReal() const noexcept;

        LuaRmluiCanvas& getView2(const std::string& name) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        RmluiRenderer& _comp;
        sol::state_view _lua;
        std::vector<std::unique_ptr<LuaCustomRmluiEventListener>> _customEventListeners;

        static LuaRmluiRenderer& addCameraComponent(Camera& cam, sol::this_state ts) noexcept;

        static void loadFont(const std::string& path) noexcept;
        static void loadFallbackFont(const std::string& path) noexcept;
    };
}