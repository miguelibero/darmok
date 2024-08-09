#include "rmlui.hpp"
#include <RmlUi/Core.h>
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
#include "../rmlui.hpp"
#include <optional>

namespace darmok
{
    LuaRmluiAppComponent::LuaRmluiAppComponent(RmluiAppComponent& comp) noexcept
        : _comp(comp)
    {
    }

    RmluiAppComponent& LuaRmluiAppComponent::getReal() noexcept
    {
        return _comp;
    }

    const RmluiAppComponent& LuaRmluiAppComponent::getReal() const noexcept
    {
        return _comp;
    }

    LuaRmluiView& LuaRmluiAppComponent::getView1() noexcept
    {
        return getView2("");
    }

    LuaRmluiView& LuaRmluiAppComponent::getView2(const std::string& name) noexcept
    {
        auto itr = _views.find(name);
        if (itr == _views.end())
        {
            itr = _views.emplace(name, _comp.getView(name)).first;
        }
        return itr->second;
    }

    bool LuaRmluiAppComponent::hasView(const std::string& name) const noexcept
    {
        return _views.contains(name);
    }

    bool LuaRmluiAppComponent::removeView(const std::string& name) noexcept
    {
        auto itr = _views.find(name);
        if (itr != _views.end())
        {
            _views.erase(itr);
        }
        return _comp.removeView(name);
    }

    LuaRmluiAppComponent& LuaRmluiAppComponent::addAppComponent(LuaApp& app) noexcept
    {
        return app.addComponent<LuaRmluiAppComponent, RmluiAppComponent>();
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

        lua.new_usertype<LuaRmluiAppComponent>("GuiAppComponent", sol::no_constructor,
            "view", sol::property(&LuaRmluiAppComponent::getView1),
            "get_view", &LuaRmluiAppComponent::getView2,
            "has_view", &LuaRmluiAppComponent::hasView,
            "remove_view", &LuaRmluiAppComponent::removeView,
            "add_app_component", &LuaRmluiAppComponent::addAppComponent,
            "load_font", &LuaRmluiAppComponent::loadFont,
            "load_fallback_font", &LuaRmluiAppComponent::loadFallbackFont
        );
    }

    void LuaRmluiView::setRmlVariant(Rml::Variant& variant, sol::object obj) noexcept
    {
        switch (obj.get_type())
        {
            case sol::type::none:
            case sol::type::nil:
                variant.Clear();
                break;
            case sol::type::boolean:
                variant = obj.as<bool>();
                break;
            case sol::type::number:
                variant = obj.as<double>();
                break;
            case sol::type::string:
                variant = obj.as<std::string>();
                break;
            case sol::type::userdata:
            {
                if (obj.is<glm::vec2>())
                {
                    variant = RmluiUtils::convert(obj.as<glm::vec2>());
                }
                if (obj.is<glm::vec3>())
                {
                    variant = RmluiUtils::convert(obj.as<glm::vec3>());
                }
                if (obj.is<glm::vec4>())
                {
                    variant = RmluiUtils::convert(obj.as<glm::vec4>());
                }
                if (obj.is<Color>())
                {
                    variant = RmluiUtils::convert(obj.as<Color>());
                }
                break;
            }
        }
    }

    void LuaRmluiView::getRmlVariant(const Rml::Variant& variant, sol::table table, sol::object key) noexcept
    {
        switch (variant.GetType())
        {
        case Rml::Variant::NONE:
        case Rml::Variant::VOIDPTR:
            table[key] = sol::nil;
            break;
        case Rml::Variant::BOOL:
            table[key] = variant.Get<bool>();
            break;
        case Rml::Variant::BYTE:
            table[key] = variant.Get<uint8_t>();
            break;
        case Rml::Variant::CHAR:
            table[key] = variant.Get<char>();
            break;
        case Rml::Variant::FLOAT:
            table[key] = variant.Get<float>();
            break;
        case Rml::Variant::DOUBLE:
            table[key] = variant.Get<double>();
            break;
        case Rml::Variant::INT:
            table[key] = variant.Get<int>();
            break;
        case Rml::Variant::INT64:
            table[key] = variant.Get<int64_t>();
            break;
        case Rml::Variant::UINT:
            table[key] = variant.Get<unsigned int>();
            break;
        case Rml::Variant::UINT64:
            table[key] = variant.Get<uint64_t>();
            break;
        case Rml::Variant::STRING:
            table[key] = variant.Get<Rml::String>();
            break;
        case Rml::Variant::VECTOR2:
            table[key] = RmluiUtils::convert(variant.Get<Rml::Vector2f>());
            break;
        case Rml::Variant::VECTOR3:
            table[key] = RmluiUtils::convert(variant.Get<Rml::Vector3f>());
            break;
        case Rml::Variant::VECTOR4:
            table[key] = RmluiUtils::convert(variant.Get<Rml::Vector4f>());
            break;
        case Rml::Variant::COLOURF:
            table[key] = RmluiUtils::convert(variant.Get<Rml::Colourf>());
            break;
        case Rml::Variant::COLOURB:
            table[key] = RmluiUtils::convert(variant.Get<Rml::Colourb>());
            break;
        }
        /*
        * TODO: check what to do with these
            SCRIPTINTERFACE = 'p',
            TRANSFORMPTR = 't',
            TRANSITIONLIST = 'T',
            ANIMATIONLIST = 'A',
            DECORATORSPTR = 'D',
            FONTEFFECTSPTR = 'F',
        */
    }

    std::string LuaRmluiView::getName() const noexcept
    {
        return _view.getName();
    }

    LuaRmluiView& LuaRmluiView::setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept
    {
        _view.setTargetTexture(texture);
        return *this;
    }

    std::shared_ptr<Texture> LuaRmluiView::getTargetTexture() noexcept
    {
        return _view.getTargetTexture();
    }

    const Viewport& LuaRmluiView::getViewport() const noexcept
    {
        return _view.getViewport();
    }

    LuaRmluiView& LuaRmluiView::setViewport(const Viewport& vp) noexcept
    {
        _view.setViewport(vp);
        return *this;
    }

    LuaRmluiView& LuaRmluiView::setEnabled(bool enabled) noexcept
    {
        _view.setEnabled(enabled);
        return *this;
    }

    bool LuaRmluiView::getEnabled() const noexcept
    {
        return _view.getEnabled();
    }

    LuaRmluiView& LuaRmluiView::setInputActive(bool active) noexcept
    {
        _view.setInputActive(active);
        return *this;
    }

    bool LuaRmluiView::getInputActive() const noexcept
    {
        return _view.getInputActive();
    }

    LuaRmluiView& LuaRmluiView::setMousePosition(const glm::vec2& position) noexcept
    {
        _view.setMousePosition(position);
        return *this;
    }

    const glm::vec2& LuaRmluiView::getMousePosition() const noexcept
    {
        return _view.getMousePosition();
    }

    LuaRmluiView& LuaRmluiView::setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept
    {
        _view.setScrollBehavior(behaviour, speedFactor);
        return *this;
    }

    Rml::ElementDocument& LuaRmluiView::loadDocument(const std::string& name)
    {
        return *_view.getContext().LoadDocument(name);
    }

    LuaRmluiView::LuaRmluiView(RmluiView& view) noexcept
        : _view(view)
    {
    }

    void LuaRmluiView::createDataModel(const std::string& name, sol::table table) noexcept
    {
        auto constr = _view.getContext().CreateDataModel(name);
        if (!constr)
        {
            return;
        }
        for (auto& [key, val] : table)
        {
            auto str = key.as<std::string>();
            constr.BindFunc(str,
                [table, key](Rml::Variant& var) { setRmlVariant(var, table[key]); },
                [table, key](const Rml::Variant& var) { getRmlVariant(var, table, key); }
            );
        }
        auto handle = constr.GetModelHandle();
        table["is_dirty"] = [handle](const std::string& name) mutable
        {
            return handle.IsVariableDirty(name);
        };
        table["set_dirty"] = sol::overload(
            [handle]() mutable
            {
                return handle.DirtyAllVariables();
            },
            [handle](const std::string& name) mutable
            {
                return handle.DirtyVariable(name);
            }
        );
    }

    Rml::DataModelHandle LuaRmluiView::getDataModel(const std::string& name) const noexcept
    {
        return _view.getContext().GetDataModel(name).GetModelHandle();
    }

    bool LuaRmluiView::removeDataModel(const std::string& name) noexcept
    {
        return _view.getContext().RemoveDataModel(name);
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

        lua.new_enum<Rml::ScrollBehavior>("GuiScrollBehavior", {
            { "Auto", Rml::ScrollBehavior::Auto },
            { "Smooth", Rml::ScrollBehavior::Smooth },
            { "Instant", Rml::ScrollBehavior::Instant }
        });

        lua.new_usertype<Rml::ElementDocument>("GuiDocument", sol::no_constructor,
            "show", sol::overload(
                [](Rml::ElementDocument& doc) { doc.Show(); },
                [](Rml::ElementDocument& doc, Rml::ModalFlag modal) { doc.Show(modal); },
                [](Rml::ElementDocument& doc, Rml::ModalFlag modal, Rml::FocusFlag focus) { doc.Show(modal, focus); }
            )
        );

        lua.new_usertype<Rml::DataModelHandle>("GuiModelHandle", sol::no_constructor,
            "is_dirty", &Rml::DataModelHandle::IsVariableDirty,
            "set_dirty", sol::overload(
                &Rml::DataModelHandle::DirtyVariable,
                &Rml::DataModelHandle::DirtyAllVariables
            )
        );

        lua.new_usertype<LuaRmluiView>("GuiView", sol::no_constructor,
            "name", sol::property(&LuaRmluiView::getName),
            "target_texture", sol::property(&LuaRmluiView::getTargetTexture, &LuaRmluiView::setTargetTexture),
            "viewport", sol::property(&LuaRmluiView::getViewport, &LuaRmluiView::setViewport),
            "input_active", sol::property(&LuaRmluiView::getInputActive, &LuaRmluiView::setInputActive),
            "mouse_position", sol::property(&LuaRmluiView::getMousePosition, &LuaRmluiView::setMousePosition),
            "set_scroll_behavior", &LuaRmluiView::setScrollBehavior,
            "load_document", &LuaRmluiView::loadDocument,
            "create_data_model", &LuaRmluiView::createDataModel,
            "get_data_model", &LuaRmluiView::getDataModel,
            "remove_data_model", &LuaRmluiView::removeDataModel
        );
    }
}