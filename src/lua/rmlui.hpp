#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/rmlui.hpp>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/DataModelHandle.h>
#include "glm.hpp"
#include "viewport.hpp"
#include <optional>

namespace darmok
{
    class LuaRmluiDocument final
    {
    public:
        LuaRmluiDocument(OptionalRef<Rml::ElementDocument> doc) noexcept;

        OptionalRef<Rml::ElementDocument> getReal() noexcept;

        void show1();
        void show2(Rml::ModalFlag modal);
        void show3(Rml::ModalFlag modal, Rml::FocusFlag focus);

        void hide();
        void close();

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<Rml::ElementDocument> _doc;
    };

    class LuaRmluiElement final
    {
    public:
        LuaRmluiElement(OptionalRef<Rml::Element> element) noexcept;

        std::string getInnerRml() noexcept;
        void setInnerRml(const std::string& value) noexcept;
        void setProperty(const std::string& name, const std::string& value) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<Rml::Element> _element;
    };

    class LuaRmluiDataModelConstructor final
    {
    public:
        LuaRmluiDataModelConstructor(const Rml::DataModelConstructor& construct) noexcept;

        LuaRmluiDataModelConstructor& bind(const std::string& name, const sol::object& obj) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        Rml::DataModelConstructor _construct;
    };

    class Texture;
    class RmluiAppComponent;
    class LuaCamera;
    class LuaTransform;
    class LuaApp;

    class LuaRmluiAppComponent final
    {
    public:
        LuaRmluiAppComponent(OptionalRef<RmluiAppComponent> comp) noexcept;

        static LuaRmluiAppComponent addAppComponent1(LuaApp& app, const std::string& name) noexcept;
        static LuaRmluiAppComponent addAppComponent2(LuaApp& app, const std::string& name, const glm::uvec2& size) noexcept;

        Rml::Context* getContext() noexcept;

        std::shared_ptr<Texture> getTargetTexture() const noexcept;
        void setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;

        std::optional<Viewport> getViewport() const noexcept;
        LuaRmluiAppComponent& setViewport(std::optional<VarViewport> viewport) noexcept;
        Viewport getCurrentViewport() const noexcept;

        void loadFont(const std::string& path) noexcept;
        void loadFallbackFont(const std::string& path) noexcept;

        LuaRmluiAppComponent& setInputActive(bool active) noexcept;
        bool getInputActive() const noexcept;

        LuaRmluiAppComponent& setMousePosition(const VarLuaTable<glm::vec2>& position) noexcept;

        OptionalRef<RmluiAppComponent> getReal() noexcept;
        const std::string& getName() noexcept;
        LuaRmluiDocument loadDocument(const std::string& name);

        std::optional<LuaRmluiDataModelConstructor> createDataModel(const std::string& name) noexcept;
        std::optional<LuaRmluiDataModelConstructor> getDataModel(const std::string& name) noexcept;
        bool removeDataModel(const std::string& name) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<RmluiAppComponent> _comp;
    };
}