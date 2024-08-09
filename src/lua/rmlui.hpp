#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/app.hpp>
#include <glm/glm.hpp>
#include <RmlUi/Core/ScrollTypes.h>
#include <unordered_map>

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

    class LuaRmluiView final
    {
    public:
        LuaRmluiView(RmluiView& view) noexcept;

        static void bind(sol::state_view& lua) noexcept;

    private:
        RmluiView& _view;

        void createDataModel(const std::string& name, sol::table table) noexcept;
        Rml::DataModelHandle getDataModel(const std::string& name) const noexcept;
        bool removeDataModel(const std::string& name) noexcept;

        std::string getName() const noexcept;

        LuaRmluiView& setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;
        std::shared_ptr<Texture> getTargetTexture() noexcept;

        const Viewport& getViewport() const noexcept;
        LuaRmluiView& setViewport(const Viewport& vp) noexcept;

        LuaRmluiView& setEnabled(bool enabled) noexcept;
        bool getEnabled() const noexcept;

        LuaRmluiView& setInputActive(bool active) noexcept;
        bool getInputActive() const noexcept;

        LuaRmluiView& setMousePosition(const glm::vec2& position) noexcept;
        const glm::vec2& getMousePosition() const noexcept;

        LuaRmluiView& setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

        Rml::ElementDocument& loadDocument(const std::string& name);

        static void setRmlVariant(Rml::Variant& variant, sol::object obj) noexcept;
        static void getRmlVariant(const Rml::Variant& variant, sol::table table, sol::object key) noexcept;

    };

    class LuaApp;

    class LuaRmluiAppComponent final : public IAppComponent
    {
    public:
        LuaRmluiAppComponent(RmluiAppComponent& comp) noexcept;

        RmluiAppComponent& getReal() noexcept;
        const RmluiAppComponent& getReal() const noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        RmluiAppComponent& _comp;
        std::unordered_map<std::string, LuaRmluiView> _views;

        static LuaRmluiAppComponent& addAppComponent(LuaApp& app) noexcept;

        LuaRmluiView& getView1() noexcept;
        LuaRmluiView& getView2(const std::string& name) noexcept;
        bool hasView(const std::string& name) const noexcept;
        bool removeView(const std::string& name) noexcept;

        static void loadFont(const std::string& path) noexcept;
        static void loadFallbackFont(const std::string& path) noexcept;
    };
}