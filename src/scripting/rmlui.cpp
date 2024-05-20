#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <darmok/rmlui.hpp>
#include "texture.hpp"

namespace darmok
{
    LuaRmluiAppComponent::LuaRmluiAppComponent(OptionalRef<RmluiAppComponent> comp) noexcept
        : _comp(comp)
    {
    }

    Rml::Context& LuaRmluiAppComponent::getContext() noexcept
    {
        return *_comp->getContext();
    }

    OptionalRef<RmluiAppComponent> LuaRmluiAppComponent::getReal() noexcept
    {
        return _comp;
    }

    const std::string& LuaRmluiAppComponent::getName() noexcept
    {
        return getContext().GetName();
    }

    std::optional<LuaTexture> LuaRmluiAppComponent::getTargetTexture() noexcept
    {
        auto tex = _comp->getTargetTexture();
        if (tex == nullptr)
        {
            return std::nullopt;
        }
        return LuaTexture(tex);
    }

    std::optional<glm::uvec2> LuaRmluiAppComponent::getSize() noexcept
    {
        return _comp->getSize();
    }

    void LuaRmluiAppComponent::setTargetTexture(const std::optional<LuaTexture>& texture) noexcept
    {
        _comp->setTargetTexture(texture ? texture->getReal() : nullptr);
    }

    void LuaRmluiAppComponent::setSize(const VarLuaTable<std::optional<glm::uvec2>>& size) noexcept
    {
        _comp->setSize(LuaGlm::tableGet(size));
    }

    void LuaRmluiAppComponent::loadFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path);
    }

    void LuaRmluiAppComponent::loadFallbackFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path, true);
    }

    LuaRmluiDocument LuaRmluiAppComponent::loadDocument(const std::string& name)
    {
        return LuaRmluiDocument(_comp->getContext()->LoadDocument(name));
    }

    void LuaRmluiAppComponent::bind(sol::state_view& lua) noexcept
    {
        LuaRmluiDocument::bind(lua);

        lua.new_usertype<LuaRmluiAppComponent>("GuiComponent", sol::no_constructor,
            "name", sol::property(&LuaRmluiAppComponent::getName),
            "target_texture", sol::property(&LuaRmluiAppComponent::getTargetTexture, &LuaRmluiAppComponent::setTargetTexture),
            "size", sol::property(&LuaRmluiAppComponent::getSize, &LuaRmluiAppComponent::setSize),
            "load_document", &LuaRmluiAppComponent::loadDocument,
            "load_font", &LuaRmluiAppComponent::loadFont,
            "load_fallback_font", &LuaRmluiAppComponent::loadFallbackFont
        );
    }

    LuaRmluiDocument::LuaRmluiDocument(OptionalRef<Rml::ElementDocument> doc) noexcept
        : _doc(doc)
    {
    }

    OptionalRef<Rml::ElementDocument> LuaRmluiDocument::LuaRmluiDocument::getReal() noexcept
    {
        return _doc;
    }

    void LuaRmluiDocument::show1()
    {
        _doc->Show();
    }
    void LuaRmluiDocument::show2(Rml::ModalFlag modal)
    {
        _doc->Show(modal);
    }

    void LuaRmluiDocument::show3(Rml::ModalFlag modal, Rml::FocusFlag focus)
    {
        _doc->Show(modal, focus);
    }

    void LuaRmluiDocument::hide()
    {
        _doc->Hide();
    }

    void LuaRmluiDocument::close()
    {
        _doc->Close();
    }

    void LuaRmluiDocument::bind(sol::state_view& lua) noexcept
    {
        lua.new_enum<Rml::ModalFlag>("GuiDocumentMode", {
            { "Normal", Rml::ModalFlag::None },
            { "Modal", Rml::ModalFlag::Modal },
            { "Keep", Rml::ModalFlag::Keep },
        });

        lua.new_enum<Rml::FocusFlag>("GuiDocumentFocus", {
            { "Normal", Rml::FocusFlag::None },
            { "Document", Rml::FocusFlag::Document },
            { "Keep", Rml::FocusFlag::Keep },
            { "Auto", Rml::FocusFlag::Auto },
        });

        lua.new_usertype<LuaRmluiDocument>("GuiDocument", sol::no_constructor,
            "show", sol::overload(
                &LuaRmluiDocument::show1,
                &LuaRmluiDocument::show2,
                &LuaRmluiDocument::show3
            ),
            "hide", &LuaRmluiDocument::hide,
            "close", &LuaRmluiDocument::close
        );
    }
}