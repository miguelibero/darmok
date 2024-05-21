#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/DataModelHandle.h>
#include "glm.hpp"
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

    class LuaTexture;
    class RmluiAppComponent;
    class LuaCamera;
    class LuaTransform;

    class LuaRmluiAppComponent final
    {
    public:
        LuaRmluiAppComponent(OptionalRef<RmluiAppComponent> comp) noexcept;

        Rml::Context* getContext() noexcept;

        std::optional<LuaTexture> getTargetTexture() noexcept;
        std::optional<glm::uvec2> getSize() noexcept;

        void setTargetTexture(const std::optional<LuaTexture>& texture) noexcept;
        void setSize(const VarLuaTable<std::optional<glm::uvec2>>& size) noexcept;

        void loadFont(const std::string& path) noexcept;
        void loadFallbackFont(const std::string& path) noexcept;

        LuaRmluiAppComponent& setInputActive(bool active) noexcept;
        bool getInputActive() const noexcept;
        LuaRmluiAppComponent& setMouseTransform(const LuaCamera& cam, const LuaTransform& trans) noexcept;
        LuaRmluiAppComponent& resetMouseTransform() noexcept;

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