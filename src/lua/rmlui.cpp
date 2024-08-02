#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <RmlUi/Lua.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <darmok/rmlui.hpp>
#include <darmok/viewport.hpp>
#include <darmok/texture.hpp>
#include "camera.hpp"
#include "transform.hpp"
#include "utils.hpp"
#include "app.hpp"
#include "glm.hpp"
#include "viewport.hpp"
#include <optional>

namespace darmok
{
    RmluiAppComponent& LuaRmluiAppComponent::addAppComponent(LuaApp& app) noexcept
    {
        return app.getReal().addComponent<RmluiAppComponent>();
    }

    void LuaRmluiAppComponent::loadFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path);
    }

    void LuaRmluiAppComponent::loadFallbackFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path, true);
    }

    void LuaRmluiAppComponent::bind(sol::state_view& lua) noexcept
    {
        LuaRmluiView::bind(lua);

        // TODO: probably will remove this dependency
        // Rml::Lua::Initialise(lua.lua_state());

        lua.new_usertype<RmluiAppComponent>("GuiAppComponent", sol::no_constructor,
            "view", sol::property([](RmluiAppComponent& comp) -> RmluiView& { return comp.getView(); }),
            "get_view", sol::resolve<RmluiView&(const std::string&)>(&RmluiAppComponent::getView),
            "has_view", &RmluiAppComponent::hasView,
            "remove_view", &RmluiAppComponent::removeView,
            "add_app_component", &LuaRmluiAppComponent::addAppComponent,
            "load_font", &LuaRmluiAppComponent::loadFont,
            "load_fallback_font", &LuaRmluiAppComponent::loadFallbackFont
        );
    }

    void LuaRmluiView::bind(sol::state_view& lua) noexcept
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

        lua.new_usertype<Rml::ElementDocument>("GuiDocument", sol::no_constructor,
            "show", sol::overload(
                [](Rml::ElementDocument& doc) { doc.Show(); },
                [](Rml::ElementDocument& doc, Rml::ModalFlag modal) { doc.Show(modal); },
                [](Rml::ElementDocument& doc, Rml::ModalFlag modal, Rml::FocusFlag focus) { doc.Show(modal, focus); }
            )
        );

        lua.new_usertype<RmluiView>("GuiView", sol::no_constructor,
            "name", sol::property(&RmluiView::getName),
            "context", sol::property(
                sol::resolve<Rml::Context&()>(&RmluiView::getContext)
            ),
            "target_texture", sol::property(&RmluiView::getTargetTexture, &RmluiView::setTargetTexture),
            "viewport", sol::property(&RmluiView::getViewport, &RmluiView::setViewport),
            "input_active", sol::property(&RmluiView::getInputActive, &RmluiView::setInputActive),
            "mouse_position", sol::property(&RmluiView::getMousePosition, &RmluiView::setMousePosition),

            "load_document", [](RmluiView& view, const std::string& name) {
                return view.getContext().LoadDocument(name);
            },
            "get_data_model", [](RmluiView& view, const std::string& name) {
                return view.getContext().GetDataModel(name);
            },
            "create_data_model", [](RmluiView& view, const std::string& name) {
                return view.getContext().CreateDataModel(name);
            },
            "remove_data_model", [](RmluiView& view, const std::string& name) {
                return view.getContext().RemoveDataModel(name);
            }
        );
    }
}