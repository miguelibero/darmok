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
    class RmluiCameraComponent;
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

    class LuaRmluiCameraComponent;

    class LuaCustomRmluiEventListener final : public Rml::EventListener
    {
    public:
        LuaCustomRmluiEventListener(const std::string& value, LuaRmluiCameraComponent& comp) noexcept;
        void ProcessEvent(Rml::Event& event) override;
    private:
        std::vector<std::string> _params;
        LuaRmluiCameraComponent& _comp;
    };

    class LuaRmluiEvent final
    {
    public:
        static void setRmlVariant(Rml::Variant& variant, sol::object obj) noexcept;
        static sol::object getRmlVariant(lua_State* lua, const Rml::Variant& variant) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    };

    class LuaEntity;
    class LuaScene;

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
        static OptionalRef<LuaRmluiCanvas>::std_t getEntityComponent(LuaEntity& entity) noexcept;
        std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;

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

        LuaRmluiCanvas& setEnabled(bool enabled) noexcept;
        bool getEnabled() const noexcept;

        LuaRmluiCanvas& setInputActive(bool active) noexcept;
        bool getInputActive() const noexcept;

        LuaRmluiCanvas& setMousePosition(const glm::vec2& position) noexcept;
        const glm::vec2& getMousePosition() const noexcept;

        LuaRmluiCanvas& setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

        Rml::ElementDocument& loadDocument(const std::string& name);
    };

    class LuaRmluiCameraComponent final : public ICameraComponent, public Rml::EventListenerInstancer
    {
    public:
        LuaRmluiCameraComponent(RmluiCameraComponent& comp, const sol::state_view& lua) noexcept;

        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;

        Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override;
        void processCustomEvent(Rml::Event& event, const std::vector<std::string>& params);

        RmluiCameraComponent& getReal() noexcept;
        const RmluiCameraComponent& getReal() const noexcept;

        LuaRmluiCanvas& getView2(const std::string& name) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        RmluiCameraComponent& _comp;
        sol::state_view _lua;
        std::vector<std::unique_ptr<LuaCustomRmluiEventListener>> _customEventListeners;

        static LuaRmluiCameraComponent& addCameraComponent(Camera& cam, sol::this_state ts) noexcept;

        static void loadFont(const std::string& path) noexcept;
        static void loadFallbackFont(const std::string& path) noexcept;
    };
}