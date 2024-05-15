#include <darmok/rmlui.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/texture.hpp>
#include "rmlui.hpp"
#include <glm/glm.hpp>

#include "generated/rmlui/shaders/basic.vertex.h"
#include "generated/rmlui/shaders/basic.fragment.h"
#include "generated/rmlui/shaders/basic.vlayout.h"

namespace darmok
{
    struct RmluiUtils final
    {
        static Rml::Vector2i convert(const glm::ivec2& v)
        {
            return { v.x, v.y };
        }
    };

    Rml::CompiledGeometryHandle RmluiRender::CompileGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture) noexcept
    {

    }

    void RmluiRender::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) noexcept
    {

    }

    void RmluiRender::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) noexcept
    {

    }


    bool RmluiRender::LoadTexture(Rml::TextureHandle& handle, Rml::Vector2i& textureDimensions, const Rml::String& source) noexcept
    {

    }

    bool RmluiRender::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& sourceDimensions) noexcept
    {

    }

    void RmluiRender::ReleaseTexture(Rml::TextureHandle texture) noexcept
    {

    }

    void RmluiRender::SetTransform(const Rml::Matrix4f* transform) noexcept
    {

    }

    void RmluiRender::RenderGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept
    {
        if (!_viewId || _program == nullptr)
        {
            return;
        }

        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;
        bgfx::allocTransientVertexBuffer(&tvb, numVertices, _layout);
        bgfx::allocTransientIndexBuffer(&tib, numIndices, true);
        bx::memCopy(tvb.data, vertices, numVertices * sizeof(Rml::Vertex));
        bx::memCopy(tib.data, indices, numIndices * sizeof(int));

        auto encoder = bgfx::begin();

        encoder->setVertexBuffer(0, &tvb);
        encoder->setIndexBuffer(&tib);

        uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_ALPHA
            ;

        encoder->setState(state);
        encoder->submit(_viewId.value(), _program->getHandle());

        bgfx::end(encoder);
    }

    void RmluiRender::EnableScissorRegion(bool enable) noexcept
    {
        SetScissorRegion(0, 0, 0, 0);
    }

    void RmluiRender::SetScissorRegion(int x, int y, int width, int height) noexcept
    {
        if (!_viewId)
        {
            return;
        }
        bgfx::setViewScissor(_viewId.value(), x, y, width, height);
    }

    const bgfx::EmbeddedShader RmluiRender::_embeddedShaders[] =
    {
        BGFX_EMBEDDED_SHADER(rmlui_basic_vertex),
        BGFX_EMBEDDED_SHADER(rmlui_basic_fragment),
        BGFX_EMBEDDED_SHADER_END()
    };

    void RmluiRender::init(App& app)
    {
        _program = std::make_unique<Program>("rmlui_basic", _embeddedShaders, rmlui_basic_vlayout);
        _layout = _program->getVertexLayout();
        _textureUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
    }

    void RmluiRender::shutdown()
    {
        _program.reset();
        if (isValid(_textureUniform))
        {
            bgfx::destroy(_textureUniform);
        }
    }

    void RmluiRender::beforeRender(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
    }

    void RmluiRender::afterRender() noexcept
    {
        _viewId.reset();
    }

    RmluiSystem::RmluiSystem() noexcept
        : _elapsedTime(0)
    {
    }

    void RmluiSystem::init(App& app)
    {
        _elapsedTime = 0;
    }

    void RmluiSystem::update(float dt) noexcept
    {
        _elapsedTime += dt;
    }

    double RmluiSystem::GetElapsedTime()
    {
        return _elapsedTime;
    }

    void RmluiDataFile::init(App& app)
    {
        _dataLoader = app.getAssets().getDataLoader();
    }

    Rml::FileHandle RmluiDataFile::Open(const Rml::String& path) noexcept
    {
        try
        {
            auto data = _dataLoader.value()(path);
            auto handle = (Rml::FileHandle)data.ptr();
            _elements.emplace(handle, Element{ std::move(data), 0 });
            return handle;
        }
        catch(...)
        {
            return (Rml::FileHandle)nullptr;
        }
    }

    OptionalRef<RmluiDataFile::Element> RmluiDataFile::find(Rml::FileHandle file) noexcept
    {
        auto itr = _elements.find(file);
        if (itr == _elements.end())
        {
            return nullptr;
        }
        return itr->second;
    }

    void RmluiDataFile::Close(Rml::FileHandle file) noexcept
    {
        _elements.erase(file);
    }

    size_t RmluiDataFile::Read(void* buffer, size_t size, Rml::FileHandle file) noexcept
    {
        auto elm = find(file);
        if (!elm)
        {
            return 0;
        }
        auto view = elm->data.view(elm->position, size);
        if (size > view.size())
        {
            size = view.size();
        }
        bx::memCopy(buffer, view.ptr(), size);
        elm->position += size;
        return size;
    }

    bool RmluiDataFile::Seek(Rml::FileHandle file, long offset, int origin) noexcept
    {
        auto elm = find(file);
        if (!elm)
        {
            return false;
        }
        auto size = elm->data.size();
        if (origin == SEEK_END)
        {
            offset += size;
        }
        else if (origin == SEEK_CUR)
        {
            offset += elm->position;
        }
        if (offset >= size)
        {
            offset = size - 1;
        }
        if (offset < 0)
        {
            offset = 0;
        }
        elm->position = offset;
        return true;
    }

    size_t RmluiDataFile::Tell(Rml::FileHandle file) noexcept
    {
        auto elm = find(file);
        if (!elm)
        {
            return 0;
        }
        return elm->position;
    }

    size_t RmluiDataFile::Length(Rml::FileHandle file) noexcept
    {
        auto elm = find(file);
        if (!elm)
        {
            return 0;
        }
        return elm->data.size();
    }

    bool RmluiDataFile::LoadFile(const Rml::String& path, Rml::String& out_data) noexcept
    {
        try
        {
            out_data = _dataLoader.value()(path).stringView();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    OptionalRef<Rml::Context> RmluiAppComponentImpl::getContext() const noexcept
    {
        return _context;
    }

    void RmluiAppComponentImpl::init(App& app)
    {
        _app = app;
        _system.init(app);
        _render.init(app);
        _file.init(app);

        Rml::SetRenderInterface(&_render);
        Rml::SetSystemInterface(&_system);
        Rml::SetFileInterface(&_file);

        // Now we can initialize RmlUi.
        Rml::Initialise();

        auto& winSize = app.getWindow().getPixelSize();
        _context = Rml::CreateContext("main", RmluiUtils::convert(winSize));

        app.getWindow().addListener(*this);
        app.getInput().getKeyboard().addListener(*this);
        app.getInput().getMouse().addListener(*this);
    }

    void RmluiAppComponentImpl::shutdown() noexcept
    {
        if (_app)
        {
            _app->getWindow().removeListener(*this);
            _app->getInput().getKeyboard().removeListener(*this);
            _app->getInput().getMouse().removeListener(*this);
        }

        _context.reset();
        Rml::Shutdown();
        _app.reset();
    }

    bool RmluiAppComponentImpl::render(bgfx::ViewId viewId) const noexcept
    {
        _render.beforeRender(viewId);
        auto result = _context->Render();
        _render.afterRender();
        return result;
    }

    bool RmluiAppComponentImpl::update(float dt) noexcept
    {
        _system.update(dt);
        return _context->Update();
    }

    void RmluiAppComponentImpl::onWindowPixelSize(const glm::uvec2& size) noexcept
    {
        if (_context == nullptr)
        {
            return;
        }
    }

    const RmluiAppComponentImpl::KeyboardMap& RmluiAppComponentImpl::getKeyboardMap() noexcept
    {
        static KeyboardMap map
        {
            { KeyboardKey::Esc, Rml::Input::KI_ESCAPE },
            { KeyboardKey::Return, Rml::Input::KI_RETURN },
            { KeyboardKey::Tab, Rml::Input::KI_TAB },
            { KeyboardKey::Space, Rml::Input::KI_SPACE },
            { KeyboardKey::Backspace, Rml::Input::KI_BACK },
            { KeyboardKey::Up, Rml::Input::KI_UP },
            { KeyboardKey::Down, Rml::Input::KI_DOWN },
            { KeyboardKey::Left, Rml::Input::KI_LEFT },
            { KeyboardKey::Right, Rml::Input::KI_RIGHT },
            { KeyboardKey::Insert, Rml::Input::KI_INSERT },
            { KeyboardKey::Delete, Rml::Input::KI_DELETE },
            { KeyboardKey::Home, Rml::Input::KI_HOME },
            { KeyboardKey::End, Rml::Input::KI_END },
            { KeyboardKey::PageUp, Rml::Input::KI_PRIOR },
            { KeyboardKey::PageDown, Rml::Input::KI_NEXT },
            { KeyboardKey::Print, Rml::Input::KI_PRINT },
            { KeyboardKey::Plus, Rml::Input::KI_ADD },
            { KeyboardKey::Minus, Rml::Input::KI_SUBTRACT },
            { KeyboardKey::LeftBracket, Rml::Input::KI_OEM_4 },
            { KeyboardKey::RightBracket, Rml::Input::KI_OEM_6 },
            { KeyboardKey::Semicolon, Rml::Input::KI_OEM_1 },
            { KeyboardKey::Quote, Rml::Input::KI_OEM_7 },
            { KeyboardKey::Comma, Rml::Input::KI_OEM_COMMA },
            { KeyboardKey::Period, Rml::Input::KI_OEM_PERIOD },
            { KeyboardKey::Slash, Rml::Input::KI_OEM_2 },
            { KeyboardKey::Backslash, Rml::Input::KI_OEM_5 },
            { KeyboardKey::GraveAccent, Rml::Input::KI_OEM_3 },
            { KeyboardKey::Pause, Rml::Input::KI_PAUSE },
        };
        static bool first = true;

        auto addRange = [](KeyboardKey start, KeyboardKey end, Rml::Input::KeyIdentifier rmluiStart) {
            auto j = to_underlying(rmluiStart);
            for (auto i = to_underlying(start); i < to_underlying(end); i++)
            {
                map[(KeyboardKey)i] = (Rml::Input::KeyIdentifier)j++;
            }
        };

        if (first)
        {
            addRange(KeyboardKey::F1, KeyboardKey::F12, Rml::Input::KI_F1);
            addRange(KeyboardKey::NumPad0, KeyboardKey::NumPad9, Rml::Input::KI_NUMPAD0);
            addRange(KeyboardKey::Key0, KeyboardKey::Key9, Rml::Input::KI_0);
            addRange(KeyboardKey::KeyA, KeyboardKey::KeyZ, Rml::Input::KI_A);
            first = false;
        }
        return map;
    }

    const RmluiAppComponentImpl::KeyboardModifierMap& RmluiAppComponentImpl::getKeyboardModifierMap() noexcept
    {
        static KeyboardModifierMap map
        {
            { to_underlying(KeyboardModifierGroup::Ctrl), Rml::Input::KM_CTRL },
            { to_underlying(KeyboardModifierGroup::Shift), Rml::Input::KM_SHIFT },
            { to_underlying(KeyboardModifierGroup::Alt), Rml::Input::KM_ALT },
            { to_underlying(KeyboardModifierGroup::Meta), Rml::Input::KM_META },
            { KeyboardKey::CapsLock, Rml::Input::KM_CAPSLOCK },
            { KeyboardKey::NumLock, Rml::Input::KM_NUMLOCK },
            { KeyboardKey::ScrollLock, Rml::Input::KM_SCROLLLOCK },
        };
        return map;
    }

    int RmluiAppComponentImpl::getKeyModifierState() const noexcept
    {
        int state = 0;
        if (!_app || _context == nullptr)
        {
            return state;
        }
        auto& kb = _app->getInput().getKeyboard();
        auto mods = kb.getModifiers();

        for (auto& elm : getKeyboardModifierMap())
        {
            auto modPtr = std::get_if<uint8_t>(&elm.first);
            if (modPtr != nullptr && mods | *modPtr)
            {
                state |= elm.second;
            }
            else if (kb.getKey(std::get<KeyboardKey>(elm.first)))
            {
                state |= elm.second;
            }
        }
        return 0;
    }

    void RmluiAppComponentImpl::onKeyboardKey(KeyboardKey key, uint8_t modifiers, bool down) noexcept
    {
        if (_context == nullptr)
        {
            return;
        }
        auto& keyMap = getKeyboardMap();
        auto itr = keyMap.find(key);
        if (itr == keyMap.end())
        {
            return;
        }
        if (down)
        {
            _context->ProcessKeyDown(itr->second, getKeyModifierState());
        }
        else
        {
            _context->ProcessKeyUp(itr->second, getKeyModifierState());
        }
    }

    void RmluiAppComponentImpl::onKeyboardChar(const Utf8Char& chr) noexcept
    {
        if (_context == nullptr)
        {
            return;
        }
        _context->ProcessTextInput(chr.data);
    }

    void RmluiAppComponentImpl::onMouseActive(bool active) noexcept
    {
        if (_context == nullptr)
        {
            return;
        }
        if (!active)
        {
            _context->ProcessMouseLeave();
        }
    }

    void RmluiAppComponentImpl::onMousePositionChange(const glm::vec2& delta) noexcept
    {
        if (_context == nullptr)
        {
            return;
        }
        _context->ProcessMouseMove(delta.x, delta.y, getKeyModifierState());
    }

    void RmluiAppComponentImpl::onMouseScrollChange(const glm::vec2& delta) noexcept
    {
        if (_context == nullptr)
        {
            return;
        }
        _context->ProcessMouseWheel(delta.x, getKeyModifierState());
    }

    void RmluiAppComponentImpl::onMouseButton(MouseButton button, bool down) noexcept
    {
        if (_context == nullptr)
        {
            return;
        }
        int i = -1;
        switch (button)
        {
        case MouseButton::Left:
            i = 0;
            break;
        case MouseButton::Right:
            i = 1;
            break;
        case MouseButton::Middle:
            i = 2;
            break;
        }
        if (down)
        {
            _context->ProcessMouseButtonDown(i, getKeyModifierState());
        }
        else
        {
            _context->ProcessMouseButtonUp(i, getKeyModifierState());
        }
    }

    RmluiAppComponent::RmluiAppComponent() noexcept
        : _impl(std::make_unique<RmluiAppComponentImpl>())
    {
    }

    OptionalRef<Rml::Context> RmluiAppComponent::getContext() const noexcept
    {
        return _impl->getContext();
    }

    void RmluiAppComponent::init(App& app)
    {
        _impl->init(app);
    }

    void RmluiAppComponent::shutdown() noexcept
    {
        _impl->shutdown();
    }

    bgfx::ViewId RmluiAppComponent::render(bgfx::ViewId viewId) const noexcept
    {
        if(_impl->render(viewId))
        {
            ++viewId;
        }
        return viewId;
    }

    void RmluiAppComponent::updateLogic(float dt) noexcept
    {
        _impl->update(dt);
    }
}