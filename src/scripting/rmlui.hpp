#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <RmlUi/Core/ElementDocument.h>
#include "glm.hpp"
#include <optional>

namespace Rml
{
    class ElementDocument;
}

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

    class LuaTexture;
    class RmluiAppComponent;

    class LuaRmluiAppComponent final
    {
    public:
        LuaRmluiAppComponent(OptionalRef<RmluiAppComponent> comp) noexcept;

        std::optional<LuaTexture> getTargetTexture() noexcept;
        std::optional<glm::uvec2> getSize() noexcept;

        void setTargetTexture(const std::optional<LuaTexture>& texture) noexcept;
        void setSize(const VarLuaTable<std::optional<glm::uvec2>>& size) noexcept;

        void loadFont(const std::string& path) noexcept;
        void loadFallbackFont(const std::string& path) noexcept;

        OptionalRef<RmluiAppComponent> getReal() noexcept;
        const std::string& getName() noexcept;
        LuaRmluiDocument loadDocument(const std::string& name);

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<RmluiAppComponent> _comp;

        Rml::Context& getContext() noexcept;
    };
}