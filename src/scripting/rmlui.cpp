#include "rmlui.hpp"
#include <RmlUi/Core.h>
#include <RmlUi/Lua.h>
#include <darmok/rmlui.hpp>
#include "texture.hpp"
#include "camera.hpp"
#include "transform.hpp"
#include "utils.hpp"

namespace darmok
{
    LuaRmluiAppComponent::LuaRmluiAppComponent(OptionalRef<RmluiAppComponent> comp) noexcept
        : _comp(comp)
    {
        comp->setMouseDelegate(*this);
    }

    LuaRmluiAppComponent::~LuaRmluiAppComponent() noexcept
    {
        if (_comp)
        {
            _comp->resetMouseDelegate();
        }
    }

    Rml::Context* LuaRmluiAppComponent::getContext() noexcept
    {
        return _comp->getContext().ptr();
    }

    OptionalRef<RmluiAppComponent> LuaRmluiAppComponent::getReal() noexcept
    {
        return _comp;
    }

    const std::string& LuaRmluiAppComponent::getName() noexcept
    {
        return getContext()->GetName();
    }

    std::optional<LuaTexture> LuaRmluiAppComponent::getTargetTexture() const noexcept
    {
        auto tex = _comp->getTargetTexture();
        if (tex == nullptr)
        {
            return std::nullopt;
        }
        return LuaTexture(tex);
    }

    glm::uvec2 LuaRmluiAppComponent::getCurrentSize() const noexcept
    {
        return _comp->getCurrentSize();
    }

    void LuaRmluiAppComponent::setTargetTexture(const std::optional<LuaTexture>& texture) noexcept
    {
        _comp->setTargetTexture(texture ? texture->getReal() : nullptr);
    }

    std::optional<glm::uvec2> LuaRmluiAppComponent::getFixedSize() const noexcept
    {
        return _comp->getFixedSize();
    }

    void LuaRmluiAppComponent::setFixedSize(const VarLuaTable<std::optional<glm::uvec2>>& size) noexcept
    {
        _comp->setFixedSize(LuaGlm::tableGet(size));
    }

    void LuaRmluiAppComponent::loadFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path);
    }

    void LuaRmluiAppComponent::loadFallbackFont(const std::string& path) noexcept
    {
        Rml::LoadFontFace(path, true);
    }

    LuaRmluiAppComponent& LuaRmluiAppComponent::setInputActive(bool active) noexcept
    {
        _comp->setInputActive(active);
        return *this;
    }

    bool LuaRmluiAppComponent::getInputActive() const noexcept
    {
        return _comp->getInputActive();
    }

    glm::ivec2 LuaRmluiAppComponent::onMousePositionChange(const glm::vec2& delta, const glm::vec2& position) noexcept
    {
        if (_mouseDelegate)
        {
            auto result = _mouseDelegate(delta, position);
            if (!result.valid())
            {
                recoveredLuaError("mouse position delegate", result);
                return position;
            }
            sol::object obj = result;
            if (obj.is<glm::ivec2>())
            {
                return obj.as<glm::ivec2>();
            }
            if (obj.is<sol::table>())
            {
                glm::ivec2 v;
                LuaGlm::tableInit(v, obj);
                return v;
            }
        }
        return position;
    }

    LuaRmluiAppComponent& LuaRmluiAppComponent::setMouseDelegate(const sol::protected_function& func) noexcept
    {
        _mouseDelegate = func;
        return *this;
    }

    glm::ivec2 LuaRmluiAppComponent::screenProject1(const glm::vec3& position) const noexcept
    {
        return _comp->screenProject(position);
    }

    glm::ivec2 LuaRmluiAppComponent::screenProject2(const glm::vec3& position, const glm::mat4& model) const noexcept
    {
        return _comp->screenProject(position, model);
    }

    LuaRmluiDocument LuaRmluiAppComponent::loadDocument(const std::string& name)
    {
        return LuaRmluiDocument(getContext()->LoadDocument(name));
    }

    std::optional<LuaRmluiDataModelConstructor> LuaRmluiAppComponent::createDataModel(const std::string& name) noexcept
    {
        auto model = getContext()->CreateDataModel(name);
        if (!model)
        {
            return std::nullopt;
        }
        return LuaRmluiDataModelConstructor(model);
    }

    std::optional<LuaRmluiDataModelConstructor> LuaRmluiAppComponent::getDataModel(const std::string& name) noexcept
    {
        auto model = getContext()->GetDataModel(name);
        if (!model)
        {
            return std::nullopt;
        }
        return LuaRmluiDataModelConstructor(model);
    }

    bool LuaRmluiAppComponent::removeDataModel(const std::string& name) noexcept
    {
        return getContext()->RemoveDataModel(name);
    }

    void LuaRmluiAppComponent::bind(sol::state_view& lua) noexcept
    {
        LuaRmluiDocument::bind(lua);

        Rml::Lua::Initialise(lua.lua_state());

        lua.new_usertype<LuaRmluiAppComponent>("GuiComponent", sol::no_constructor,
            "name", sol::property(&LuaRmluiAppComponent::getName),
            "context", sol::property(&LuaRmluiAppComponent::getContext),
            "target_texture", sol::property(&LuaRmluiAppComponent::getTargetTexture, &LuaRmluiAppComponent::setTargetTexture),
            "fixed_size", sol::property(&LuaRmluiAppComponent::getFixedSize, &LuaRmluiAppComponent::setFixedSize),
            "current_size", sol::property(&LuaRmluiAppComponent::getCurrentSize),
            "input_active", sol::property(&LuaRmluiAppComponent::getInputActive, &LuaRmluiAppComponent::setInputActive),
            "mouse_delegate", sol::property(&LuaRmluiAppComponent::setMouseDelegate),
            "screen_project", sol::overload(&LuaRmluiAppComponent::screenProject1, &LuaRmluiAppComponent::screenProject2),
            "load_document", &LuaRmluiAppComponent::loadDocument,
            "load_font", &LuaRmluiAppComponent::loadFont,
            "load_fallback_font", &LuaRmluiAppComponent::loadFallbackFont,
            "get_data_model", &LuaRmluiAppComponent::getDataModel,
            "create_data_model", &LuaRmluiAppComponent::createDataModel,
            "remove_data_model", &LuaRmluiAppComponent::removeDataModel
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

    LuaRmluiElement::LuaRmluiElement(OptionalRef<Rml::Element> element) noexcept
        : _element(element)
    {
    }

    std::string LuaRmluiElement::getInnerRml() noexcept
    {
        return _element->GetInnerRML();
    }

    void LuaRmluiElement::setInnerRml(const std::string& value) noexcept
    {
        _element->SetInnerRML(value);
    }

    void LuaRmluiElement::setProperty(const std::string& name, const std::string& value) noexcept
    {
        _element->SetProperty(name, value);
    }

    void LuaRmluiElement::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaRmluiElement>("GuiElement", sol::no_constructor,
            "inner_rml", sol::property(&LuaRmluiElement::getInnerRml, &LuaRmluiElement::setInnerRml),
            "set_property", &LuaRmluiElement::setProperty
        );
    }

    LuaRmluiDataModelConstructor::LuaRmluiDataModelConstructor(const Rml::DataModelConstructor& construct) noexcept
        : _construct(construct)
    {
    }

    LuaRmluiDataModelConstructor& LuaRmluiDataModelConstructor::bind(const std::string& name, const sol::object& obj) noexcept
    {
        switch (obj.get_type())
        {
        case sol::type::string:
            obj.as<std::string>();
            //_construct.Bind(name, );
            break;
        }
        return *this;
    }

    void LuaRmluiDataModelConstructor::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaRmluiDataModelConstructor>("GuiDataModelConstructor", sol::no_constructor,
            "bind", sol::overload(
                &LuaRmluiDocument::bind
            )
        );
    }
}