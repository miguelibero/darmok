#include <darmok/rmlui.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/texture.hpp>
#include "rmlui.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bx/math.h>

#include "generated/rmlui/shaders/basic.vertex.h"
#include "generated/rmlui/shaders/basic.fragment.h"
#include "generated/rmlui/shaders/solid.vertex.h"
#include "generated/rmlui/shaders/solid.fragment.h"
#include "generated/rmlui/shaders/basic.vlayout.h"

namespace darmok
{
    struct RmluiUtils final
    {
        template<typename T>
        static Rml::Vector2<T> convert(const glm::vec2& v)
        {
            return { (T)v.x, (T)v.y };
        }

        template<typename T>
        static glm::vec<2, T> convert(const Rml::Vector2<T>& v)
        {
            return { v.x, v.y };
        }
    };

    Rml::CompiledGeometryHandle RmluiRenderInterface::CompileGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture) noexcept
    {
        auto itr = _textures.find(texture);
        DataView vertData(vertices, sizeof(Rml::Vertex) * numVertices);
        DataView idxData(indices, sizeof(int) * numIndices);
        MeshConfig meshConfig;
        meshConfig.index32 = true;
        auto mesh = std::make_unique<Mesh>(_layout, vertData, idxData, meshConfig);
        Rml::CompiledGeometryHandle handle = mesh->getVertexHandle().idx;
        _compiledGeometries.emplace(handle, CompiledGeometry{ std::move(mesh), texture });
        return handle;
    }

    void RmluiRenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle handle, const Rml::Vector2f& translation) noexcept
    {
        if (!_encoder)
        {
            return;
        }
        auto itr = _compiledGeometries.find(handle);
        if (itr == _compiledGeometries.end())
        {
            return;
        }
        itr->second.mesh->render(_encoder.value());
        submitGeometry(itr->second.texture, translation);
    }

    void RmluiRenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle handle) noexcept
    {
        auto itr = _compiledGeometries.find(handle);
        if (itr != _compiledGeometries.end())
        {
            _compiledGeometries.erase(itr);
        }
    }

    bool RmluiRenderInterface::LoadTexture(Rml::TextureHandle& handle, Rml::Vector2i& dimensions, const Rml::String& source) noexcept
    {
        const auto file = Rml::GetFileInterface();
        const auto fileHandle = file->Open(source);

        if (!fileHandle)
        {
            return false;
        }

        Data data(file->Length(fileHandle));
        file->Read(data.ptr(), data.size(), fileHandle);
        file->Close(fileHandle);

        return createTexture(handle, data.view(), dimensions);
    }

    bool RmluiRenderInterface::GenerateTexture(Rml::TextureHandle& handle, const Rml::byte* source, const Rml::Vector2i& dimensions) noexcept
    {
        auto size = 4 * sizeof(Rml::byte) * dimensions.x * dimensions.y;
        return createTexture(handle, DataView(source, size), dimensions);
    }

    bool RmluiRenderInterface::createTexture(Rml::TextureHandle& handle, const DataView& data, const Rml::Vector2i& dimensions) noexcept
    {
        TextureConfig config;
        config.format = bgfx::TextureFormat::RGBA8;
        config.size = glm::uvec2(dimensions.x, dimensions.y);
        try
        {
            auto texture = std::make_unique<Texture>(data, config);
            handle = texture->getHandle().idx;
            _textures.emplace(handle, std::move(texture));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    void RmluiRenderInterface::ReleaseTexture(Rml::TextureHandle handle) noexcept
    {
        auto itr = _textures.find(handle);
        if (itr != _textures.end())
        {
            _textures.erase(itr);
        }
    }

    void RmluiRenderInterface::SetTransform(const Rml::Matrix4f* transform) noexcept
    {
        if (transform == nullptr)
        {
            _transform = glm::mat4(1);
        }
        else
        {
            _transform = (glm::mat4)*transform->data();
        }
    }

    RmluiRenderInterface::RmluiRenderInterface() noexcept
        : _frameBuffer{ bgfx::kInvalidHandle }
    {
    }

    void RmluiRenderInterface::RenderGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept
    {
        if (!_encoder)
        {
            return;
        }

        DataView vertData(vertices, numVertices * sizeof(Rml::Vertex));
        DataView idxData(indices, numIndices * sizeof(int));
        TransientMesh mesh(_layout, vertData, idxData, true);
        mesh.render(_encoder.value(), 0);

        submitGeometry(texture, translation);
    }

    void RmluiRenderInterface::setupView() noexcept
    {
        auto viewId = _viewId.value();
        static const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
        bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);
        bgfx::setViewName(viewId, "RmlUI");
        bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);

        auto dim = GetContext()->GetDimensions();
        bgfx::setViewRect(_viewId.value(), 0, 0, dim.x, dim.y);

        glm::mat4 proj(1);
        bx::mtxOrtho(glm::value_ptr(proj), 0, dim.x, dim.y, 0.F, -1000.F, 1000.F, 0.F, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(viewId, glm::value_ptr(_transform), glm::value_ptr(proj));

        bgfx::setViewFrameBuffer(viewId, _frameBuffer);
    }

    void RmluiRenderInterface::submitGeometry(Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept
    {
        if (!_viewId || _program == nullptr || !_encoder)
        {
            return;
        }

        if (!_viewSetup)
        {
            setupView();
            _viewSetup = true;
        }

        auto trans = glm::translate(glm::mat4(1), glm::vec3(translation.x, translation.y, 0.0f));
        _encoder->setTransform(glm::value_ptr(trans));

        auto programHandle = _program->getHandle();

        if (texture)
        {
            auto itr = _textures.find(texture);
            if (itr != _textures.end())
            {
                _encoder->setTexture(0, _textureUniform, itr->second->getHandle());
            }
        }
        else
        {
            programHandle = _solidProgram->getHandle();
        }


        static const uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_ALPHA
            ;

        _encoder->setState(state);
        _encoder->submit(_viewId.value(), programHandle);
        _rendered = true;
    }

    void RmluiRenderInterface::EnableScissorRegion(bool enable) noexcept
    {
        if (!_viewId)
        {
            return;
        }
        _scissorEnabled = enable;
        if (enable)
        {
            bgfx::setViewScissor(_viewId.value(), _scissor.x, _scissor.y, _scissor.z, _scissor.w);
        }
        else
        {
            bgfx::setViewScissor(_viewId.value());
        }
    }

    void RmluiRenderInterface::SetScissorRegion(int x, int y, int width, int height) noexcept
    {
        _scissor = glm::ivec4(x, y, width, height);
        if (_scissorEnabled)
        {
            EnableScissorRegion(true);
        }
    }

    const bgfx::EmbeddedShader RmluiRenderInterface::_embeddedShaders[] =
    {
        BGFX_EMBEDDED_SHADER(rmlui_basic_vertex),
        BGFX_EMBEDDED_SHADER(rmlui_basic_fragment),
        BGFX_EMBEDDED_SHADER(rmlui_solid_vertex),
        BGFX_EMBEDDED_SHADER(rmlui_solid_fragment),
        BGFX_EMBEDDED_SHADER_END()
    };

    void RmluiRenderInterface::init(App& app)
    {
        _program = std::make_unique<Program>("rmlui_basic", _embeddedShaders, rmlui_basic_vlayout);
        _solidProgram = std::make_unique<Program>("rmlui_solid", _embeddedShaders, rmlui_basic_vlayout);
        _layout = _program->getVertexLayout();
        _textureUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
        _scissorEnabled = false;
        _rendered = false;
        _viewSetup = false;
    }

    void RmluiRenderInterface::shutdown() noexcept
    {
        _program.reset();
        if (isValid(_textureUniform))
        {
            bgfx::destroy(_textureUniform);
        }
    }

    void RmluiRenderInterface::beforeRender(bgfx::ViewId viewId) noexcept
    {
        if (_encoder)
        {
            bgfx::end(_encoder.ptr());
        }

        _viewId = viewId;

        _encoder = bgfx::begin();
        _rendered = false;
        _viewSetup = false;
        _transform = glm::mat4(1);
    }

    bool RmluiRenderInterface::afterRender() noexcept
    {
        if (_encoder)
        {
            bgfx::end(_encoder.ptr());
            _encoder.reset();
        }
        _viewId.reset();
        SetTransform(nullptr);
        return _rendered;
    }


    void RmluiRenderInterface::setTargetTextures(const std::vector<std::shared_ptr<Texture>>& textures) noexcept
    {
        if (_targetTextures != textures)
        {
            if (isValid(_frameBuffer))
            {
                bgfx::destroy(_frameBuffer);
            }
            _targetTextures = textures;
            std::vector<bgfx::TextureHandle> handles;
            handles.reserve(textures.size());
            for (auto& tex : textures)
            {
                handles.push_back(tex->getHandle());
            }
            _frameBuffer = bgfx::createFrameBuffer(handles.size(), &handles.front());
        }
    }

    const std::vector<std::shared_ptr<Texture>>& RmluiRenderInterface::getTargetTextures() noexcept
    {
        return _targetTextures;
    }

    RmluiSystemInterface::RmluiSystemInterface() noexcept
        : _elapsedTime(0)
    {
    }

    void RmluiSystemInterface::init(App& app)
    {
        _elapsedTime = 0;
    }

    void RmluiSystemInterface::update(float dt) noexcept
    {
        _elapsedTime += dt;
    }

    double RmluiSystemInterface::GetElapsedTime()
    {
        return _elapsedTime;
    }

    void RmluiFileInterface::init(App& app)
    {
        _dataLoader = app.getAssets().getDataLoader();
    }

    Rml::FileHandle RmluiFileInterface::Open(const Rml::String& path) noexcept
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

    OptionalRef<RmluiFileInterface::Element> RmluiFileInterface::find(Rml::FileHandle file) noexcept
    {
        auto itr = _elements.find(file);
        if (itr == _elements.end())
        {
            return nullptr;
        }
        return itr->second;
    }

    void RmluiFileInterface::Close(Rml::FileHandle file) noexcept
    {
        _elements.erase(file);
    }

    size_t RmluiFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file) noexcept
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

    bool RmluiFileInterface::Seek(Rml::FileHandle file, long offset, int origin) noexcept
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

    size_t RmluiFileInterface::Tell(Rml::FileHandle file) noexcept
    {
        auto elm = find(file);
        if (!elm)
        {
            return 0;
        }
        return elm->position;
    }

    size_t RmluiFileInterface::Length(Rml::FileHandle file) noexcept
    {
        auto elm = find(file);
        if (!elm)
        {
            return 0;
        }
        return elm->data.size();
    }

    bool RmluiFileInterface::LoadFile(const Rml::String& path, Rml::String& out_data) noexcept
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

    RmluiAppComponentImpl::RmluiAppComponentImpl(const std::string& name) noexcept
        : _inputActive(true)
        , _name(name)
    {
    }

    RmluiAppComponentImpl::RmluiAppComponentImpl(const std::string& name, const glm::uvec2& size) noexcept
        : _size(size)
        , _inputActive(false)
        , _name(name)
    {
    }

    void RmluiAppComponentImpl::setSize(const std::optional<glm::uvec2>& size) noexcept
    {
        _size = size;
        if (!_context)
        {
            return;
        }
        if (size)
        {
            _context->SetDimensions(RmluiUtils::convert<int>(size.value()));
        }
        else if (_app)
        {
            auto& winSize = _app->getWindow().getPixelSize();
            _context->SetDimensions(RmluiUtils::convert<int>(winSize));
        }
    }

    const std::optional<glm::uvec2>& RmluiAppComponentImpl::getSize() const noexcept
    {
        return _size;
    }

    void RmluiAppComponentImpl::setInputActive(bool active) noexcept
    {
        _inputActive = active;
    }

    bool RmluiAppComponentImpl::getInputActive() const noexcept
    {
        return _inputActive;
    }

    OptionalRef<Rml::Context> RmluiAppComponentImpl::getContext() const noexcept
    {
        return _context;
    }

    RmluiRenderInterface& RmluiAppComponentImpl::getRenderInterface() noexcept
    {
        return _render;
    }

    void RmluiSharedAppComponent::init(App& app) noexcept
    {
        _system.init(app);
        _file.init(app);

        Rml::SetSystemInterface(&_system);
        Rml::SetFileInterface(&_file);

        Rml::Initialise();
    }

    void RmluiSharedAppComponent::shutdown() noexcept
    {
        Rml::Shutdown();
    }

    void RmluiSharedAppComponent::update(float dt) noexcept
    {
        _system.update(dt);
    }

    void RmluiAppComponentImpl::init(App& app)
    {
        _app = app;
        _shared = app.getSharedComponent<RmluiSharedAppComponent>();

        _render.init(app);
        auto size = _size ? _size.value() : app.getWindow().getPixelSize();
        _context = Rml::CreateContext(_name, RmluiUtils::convert<int>(size), &_render);

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

        Rml::RemoveContext(_context->GetName());
        Rml::ReleaseTextures();

        _shared.reset();
        _context.reset();
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
        return _context->Update();
    }

    void RmluiAppComponentImpl::onWindowPixelSize(const glm::uvec2& size) noexcept
    {
        if (_context == nullptr)
        {
            return;
        }
        _context->SetDimensions(RmluiUtils::convert<int>(size));
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
        if (!_inputActive || _context == nullptr)
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
        if (!_inputActive || _context == nullptr)
        {
            return;
        }
        _context->ProcessTextInput(chr.to_string());
    }

    void RmluiAppComponentImpl::onMouseActive(bool active) noexcept
    {
        if (!_inputActive || _context == nullptr)
        {
            return;
        }
        if (!active)
        {
            _context->ProcessMouseLeave();
        }
    }

    void RmluiAppComponentImpl::onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept
    {
        if (!_inputActive || _context == nullptr || !_app)
        {
            return;
        }
        auto y = _app->getWindow().getPixelSize().y - absolute.y;
        _context->ProcessMouseMove(absolute.x, y, getKeyModifierState());
    }

    void RmluiAppComponentImpl::onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept
    {
        if (!_inputActive || _context == nullptr)
        {
            return;
        }
        _context->ProcessMouseWheel(RmluiUtils::convert<float>(delta), getKeyModifierState());
    }

    void RmluiAppComponentImpl::onMouseButton(MouseButton button, bool down) noexcept
    {
        if (!_inputActive || _context == nullptr)
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

    RmluiAppComponent::RmluiAppComponent(const std::string& name) noexcept
        : _impl(std::make_unique<RmluiAppComponentImpl>(name))
    {
    }

    RmluiAppComponent::RmluiAppComponent(const std::string& name, const glm::uvec2& size) noexcept
        : _impl(std::make_unique<RmluiAppComponentImpl>(name, size))
    {
    }

    OptionalRef<Rml::Context> RmluiAppComponent::getContext() const noexcept
    {
        return _impl->getContext();
    }

    RmluiAppComponent& RmluiAppComponent::setTargetTextures(const std::vector<std::shared_ptr<Texture>>& textures) noexcept
    {
        _impl->getRenderInterface().setTargetTextures(textures);
        return *this;
    }

    const std::vector<std::shared_ptr<Texture>>& RmluiAppComponent::getTargetTextures() noexcept
    {
        return _impl->getRenderInterface().getTargetTextures();
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