#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/app.hpp>
#include <glm/glm.hpp>
#include <RmlUi/Core/ScrollTypes.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/EventListenerInstancer.h>
#include <unordered_map>
#include "viewport.hpp"
#include "glm.hpp"

namespace Rml
{
    class Variant;
    class ElementDocument;
    class DataModelHandle;
}

namespace darmok
{
    class Texture;
    class RmluiAppComponent;
    class RmluiView;
    struct Viewport;

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

    class LuaRmluiAppComponent;

    class LuaCustomRmluiEventListener final : public Rml::EventListener
    {
    public:
        LuaCustomRmluiEventListener(const std::string& value, LuaRmluiAppComponent& comp) noexcept;
        void ProcessEvent(Rml::Event& event) override;
    private:
        std::vector<std::string> _params;
        LuaRmluiAppComponent& _comp;
    };

    class LuaRmluiView final
    {
    public:
        LuaRmluiView(RmluiView& view) noexcept;
        ~LuaRmluiView() noexcept;
        static void bind(sol::state_view& lua) noexcept;

        size_t processCustomEvent(Rml::Event& event, const std::string& name, const sol::table& args);

    private:
        RmluiView& _view;
        std::vector<std::unique_ptr<LuaTableRmluiEventListener>> _tabEventListeners;
        std::vector<std::unique_ptr<LuaFunctionRmluiEventListener>> _funcEventListeners;

        void createDataModel(const std::string& name, sol::table table) noexcept;
        Rml::DataModelHandle getDataModel(const std::string& name) const noexcept;
        bool removeDataModel(const std::string& name) noexcept;

        std::string getName() const noexcept;

        LuaRmluiView& addEventListener1(const std::string& ev, const sol::table& tab) noexcept;
        bool removeEventListener1(const std::string& ev, const sol::table& tab) noexcept;
        bool removeEventListener2(const sol::table& tab) noexcept;

        LuaRmluiView& addEventListener2(const std::string& ev, const sol::protected_function& func) noexcept;
        bool removeEventListener4(const std::string& ev, const sol::protected_function& func) noexcept;
        bool removeEventListener3(const sol::protected_function& func) noexcept;

        LuaRmluiView& setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;
        std::shared_ptr<Texture> getTargetTexture() noexcept;

        const Viewport& getViewport() const noexcept;
        LuaRmluiView& setViewport(const VarViewport& vp) noexcept;

        glm::uvec2 getSize() const noexcept;
        LuaRmluiView& setSize(const VarLuaTable<glm::uvec2>& size) noexcept;

        LuaRmluiView& setEnabled(bool enabled) noexcept;
        bool getEnabled() const noexcept;

        LuaRmluiView& setInputActive(bool active) noexcept;
        bool getInputActive() const noexcept;

        LuaRmluiView& setMousePosition(const glm::vec2& position) noexcept;
        const glm::vec2& getMousePosition() const noexcept;

        LuaRmluiView& setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

        Rml::ElementDocument& loadDocument(const std::string& name);

        static void setRmlVariant(Rml::Variant& variant, sol::object obj) noexcept;
        static sol::object getRmlVariant(lua_State* lua, const Rml::Variant& variant) noexcept;
    };

    class LuaApp;

    class LuaRmluiAppComponent final : public IAppComponent, public Rml::EventListenerInstancer
    {
    public:
        LuaRmluiAppComponent(RmluiAppComponent& comp, const sol::state_view& lua) noexcept;

        void init(App& app) noexcept override;
        void shutdown() noexcept override;

        Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override;
        void processCustomEvent(Rml::Event& event, const std::vector<std::string>& params);

        RmluiAppComponent& getReal() noexcept;
        const RmluiAppComponent& getReal() const noexcept;

        LuaRmluiView& getView2(const std::string& name) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        RmluiAppComponent& _comp;
        sol::state_view _lua;
        std::unordered_map<std::string, LuaRmluiView> _views;
        std::vector<std::unique_ptr<LuaCustomRmluiEventListener>> _customEventListeners;

        static LuaRmluiAppComponent& addAppComponent(LuaApp& app, sol::this_state ts) noexcept;

        LuaRmluiView& getView1() noexcept;
        bool hasView(const std::string& name) const noexcept;
        bool removeView(const std::string& name) noexcept;

        static void loadFont(const std::string& path) noexcept;
        static void loadFallbackFont(const std::string& path) noexcept;
    };
}