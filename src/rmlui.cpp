#include <darmok/rmlui.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/texture.hpp>
#include <darmok/image.hpp>
#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/shape.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/program_core.hpp>
#include <darmok/math.hpp>
#include "rmlui.hpp"
#include <darmok/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "generated/rmlui/rmlui.program.h"

namespace darmok
{

    Color RmluiUtils::convert(const Rml::Colourf& v) noexcept
    {
        auto f = Colors::getMaxValue();
        return Color(v.red * f, v.green * f, v.blue * f, v.alpha * f);
    }

    Color RmluiUtils::convert(const Rml::Colourb& v) noexcept
    {
        return Color(v.red, v.green, v.blue, v.alpha);
    }

    Rml::Colourb RmluiUtils::convert(const Color& v) noexcept
    {
        return Rml::Colourb(v.r, v.g, v.b, v.a);
    }

    TextureAtlasBounds RmluiUtils::convert(const Rml::Rectangle& v) noexcept
    {
        return TextureAtlasBounds{ glm::vec2(v.width, v.height), glm::vec2(v.x, v.y) };
    }

    Rml::CompiledGeometryHandle RmluiRenderInterface::CompileGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture) noexcept
    {
        auto itr = _textures.find(texture);
        DataView vertData(vertices, sizeof(Rml::Vertex) * numVertices);
        DataView idxData(indices, sizeof(int) * numIndices);
        MeshConfig meshConfig;
        meshConfig.index32 = true;
        auto mesh = std::make_unique<Mesh>(_program->getVertexLayout(), vertData, idxData, meshConfig);
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

        Data data(file->Length(fileHandle), _alloc);
        file->Read(data.ptr(), data.size(), fileHandle);
        file->Close(fileHandle);

        try
        {
            Image img(data, _alloc);
            auto size = img.getSize();
            dimensions.x = size.x;
            dimensions.y = size.y;
            auto texture = std::make_unique<Texture>(img);
            handle = texture->getHandle().idx;
            _textures.emplace(handle, std::move(texture));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool RmluiRenderInterface::GenerateTexture(Rml::TextureHandle& handle, const Rml::byte* source, const Rml::Vector2i& dimensions) noexcept
    {
        auto size = 4 * sizeof(Rml::byte) * dimensions.x * dimensions.y;
        TextureConfig config;
        config.format = bgfx::TextureFormat::RGBA8;
        config.size = glm::uvec2(dimensions.x, dimensions.y);
        DataView data(source, size);

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

    RmluiRenderInterface::RmluiRenderInterface(const std::shared_ptr<Program>& prog, const Viewport& vp, bx::AllocatorI& alloc) noexcept
        : _frameBuffer{ bgfx::kInvalidHandle }
        , _textureUniform(bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler))
        , _scissorEnabled(false)
        , _scissor(0)
        , _transform(1)
        , _alloc(alloc)
        , _program(prog)
        , _viewId(0)
        , _viewport(vp)
    {
    }

    RmluiRenderInterface::~RmluiRenderInterface() noexcept
    {
        if (isValid(_textureUniform))
        {
            bgfx::destroy(_textureUniform);
        }
    }

    void RmluiRenderInterface::RenderGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept
    {
        if (!_encoder)
        {
            return;
        }

        DataView vertData(vertices, numVertices * sizeof(Rml::Vertex));
        DataView idxData(indices, numIndices * sizeof(int));
        TransientMesh mesh(_program->getVertexLayout(), vertData, idxData, true);
        mesh.render(_encoder.value());

        submitGeometry(texture, translation);
    }

    const uint64_t RmluiRenderInterface::_state = 0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_MSAA
        | BGFX_STATE_BLEND_ALPHA
        ;

    void RmluiRenderInterface::submitGeometry(Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept
    {
        if (!_program || !_encoder || !_viewId)
        {
            return;
        }

        auto trans = glm::translate(glm::mat4(1), glm::vec3(translation.x, translation.y, 0.0f));
        _encoder->setTransform(glm::value_ptr(trans));

        ProgramDefines defines{ "TEXTURE_DISABLE" };
        if (texture)
        {
            auto itr = _textures.find(texture);
            if (itr != _textures.end())
            {
                _encoder->setTexture(0, _textureUniform, itr->second->getHandle());
                defines.clear();
            }
        }
        _encoder->setState(_state);
        _encoder->submit(_viewId.value(), _program->getHandle(defines));
    }

    void RmluiRenderInterface::EnableScissorRegion(bool enable) noexcept
    {
        if (!_viewId)
        {
            return;
        }
        _scissorEnabled = enable;
        auto viewId = _viewId.value();
        if (enable)
        {
            bgfx::setViewScissor(viewId, _scissor.x, _scissor.y, _scissor.z, _scissor.w);
        }
        else
        {
            bgfx::setViewScissor(viewId);
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

    void RmluiRenderInterface::beforeRender(bgfx::Encoder& encoder) noexcept
    {
        _encoder = encoder;
        _transform = glm::mat4(1);
    }

    void RmluiRenderInterface::setViewId(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
        configureView();
    }

    bool RmluiRenderInterface::configureView() noexcept
    {
        if (!_viewId)
        {
            return false;
        }
        auto viewId = _viewId.value();
        static const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
        // bgfx::setViewClear(viewId, clearFlags, 1.F, 0U, 1);
        bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);

        _viewport.configureView(viewId);

        auto bot = glm::vec2(_viewport.origin);
        auto top = bot + glm::vec2(_viewport.size);
        auto proj = Math::ortho(bot.x, top.x, top.y, bot.y);

        bgfx::setViewTransform(viewId, glm::value_ptr(_transform), glm::value_ptr(proj));
        bgfx::setViewFrameBuffer(viewId, _frameBuffer);

        return true;
    }

    void RmluiRenderInterface::setViewport(const Viewport& vp) noexcept
    {
        _viewport = vp;
        configureView();
    }

    void RmluiRenderInterface::renderMouseCursor(const Rml::Sprite& sprite, const glm::vec2& position) noexcept
    {
        if (!_encoder || !_program || !_viewId)
        {
            return;
        }

        auto rmlHandle = sprite.sprite_sheet->texture.GetHandle(this);
        auto itr = _textures.find(rmlHandle);
        if (itr == _textures.end())
        {
            return;
        }
        auto& tex = *itr->second;

        auto textureElm = TextureAtlasElement::create(RmluiUtils::convert(sprite.rectangle));
        TextureAtlasMeshConfig config;
        config.type = MeshType::Transient;
        auto mesh = textureElm.createSprite(_program->getVertexLayout(), tex.getSize(), config);

        _encoder->setTexture(0, _textureUniform, tex.getHandle());

        glm::vec3 p(position, 0);
        auto trans = glm::translate(glm::mat4(1), p);
        _encoder->setTransform(glm::value_ptr(trans));

        mesh->render(_encoder.value());
        _encoder->setState(_state);
        _encoder->submit(_viewId.value(), _program->getHandle());
    }

    void RmluiRenderInterface::afterRender() noexcept
    {
        _encoder.reset();
        SetTransform(nullptr);
    }

    void RmluiRenderInterface::setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept
    {
        if (_targetTexture != texture)
        {
            if (isValid(_frameBuffer))
            {
                bgfx::destroy(_frameBuffer);
            }
            _targetTexture = texture;
            if (_targetTexture != nullptr)
            {
                _frameBuffer = bgfx::createFrameBuffer(1, &_targetTexture->getHandle());
            }
        }
    }

    std::shared_ptr<Texture> RmluiRenderInterface::getTargetTexture() const noexcept
    {
        return _targetTexture;
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

    void RmluiSystemInterface::SetMouseCursor(const Rml::String& name) noexcept
    {
        _mouseCursor = name;
    }

    const std::string& RmluiSystemInterface::getMouseCursor() const noexcept
    {
        return _mouseCursor;
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

    RmluiViewImpl::RmluiViewImpl(const std::string& name, int priority, const Viewport& vp, RmluiAppComponentImpl& comp)
        : _render(comp.getProgram(), vp, comp.getAllocator())
        , _inputActive(false)
        , _mousePosition(0)
        , _comp(comp)
        , _viewport(vp)
        , _fullscreen(false)
        , _enabled(true)
        , _priority(priority)
    {
        _context = Rml::CreateContext(name, RmluiUtils::convert<int>(vp.size), &_render);
        if (!_context)
        {
            throw std::runtime_error("Failed to create rmlui context");
        }
        _context->EnableMouseCursor(true);
    }

    bool RmluiViewImpl::getInputActive() const noexcept
    {
        return _inputActive;
    }

    void RmluiViewImpl::setInputActive(bool active) noexcept
    {
        _inputActive = active;
    }

    bool RmluiViewImpl::getEnabled() const noexcept
    {
        return _enabled;
    }

    void RmluiViewImpl::setEnabled(bool enabled) noexcept
    {
        _enabled = enabled;
    }

    std::string RmluiViewImpl::getName() const noexcept
    {
        return _context->GetName();
    }

    bool RmluiViewImpl::getFullscreen() const noexcept
    {
        return _fullscreen;
    }

    void RmluiViewImpl::setFullscreen(bool enabled) noexcept
    {
        _fullscreen = enabled;
    }

    Rml::Context& RmluiViewImpl::getContext() noexcept
    {
        return _context.value();
    }

    const Rml::Context& RmluiViewImpl::getContext() const noexcept
    {
        return _context.value();
    }

    RmluiViewImpl::~RmluiViewImpl() noexcept
    {
        Rml::RemoveContext(_context->GetName());
    }

    const Viewport& RmluiViewImpl::getViewport() const noexcept
    {
        return _viewport;
    }

    void RmluiViewImpl::setViewport(const Viewport& vp) noexcept
    {
        std::scoped_lock lock(_mutex);
        _viewport = vp;
        _render.setViewport(vp);
        _context->SetDimensions(RmluiUtils::convert<int>(vp.size));
    }

    void RmluiViewImpl::setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept
    {
        _render.setTargetTexture(texture);
        if (texture != nullptr)
        {
            setViewport(texture->getSize());
        }
    }

    std::shared_ptr<Texture> RmluiViewImpl::getTargetTexture() const noexcept
    {
        return _render.getTargetTexture();
    }

    void RmluiViewImpl::setMousePosition(const glm::vec2& position) noexcept
    {
        std::scoped_lock lock(_mutex);
        glm::vec2 p = position;

        // invert vertical since that's how rmlui works
        auto h = _context->GetDimensions().y;
        p.y = h - p.y;

        _mousePosition = p;
        _context->ProcessMouseMove(p.x, p.y, _comp.getKeyModifierState());
    }

    const glm::vec2& RmluiViewImpl::getMousePosition() const noexcept
    {
        return _mousePosition;
    }

    void RmluiViewImpl::processKey(Rml::Input::KeyIdentifier key, int state, bool down) noexcept
    {
        std::scoped_lock lock(_mutex);
        if (!_inputActive)
        {
            return;
        }
        if (down)
        {
            _context->ProcessKeyDown(key, state);
        }
        else
        {
            _context->ProcessKeyUp(key, state);
        }
    }

    void RmluiViewImpl::processTextInput(const Rml::String& str) noexcept
    {
        std::scoped_lock lock(_mutex);
        if (!_inputActive)
        {
            return;
        }
        _context->ProcessTextInput(str);
    }

    void RmluiViewImpl::processMouseLeave() noexcept
    {
        std::scoped_lock lock(_mutex);
        if (!_inputActive)
        {
            return;
        }
        _context->ProcessMouseLeave();
    }

    void RmluiViewImpl::processMouseWheel(const Rml::Vector2f& val, int keyState) noexcept
    {
        std::scoped_lock lock(_mutex);
        if (!_inputActive)
        {
            return;
        }
        _context->ProcessMouseWheel(val, keyState);
    }

    void RmluiViewImpl::processMouseButton(int num, int keyState, bool down) noexcept
    {
        std::scoped_lock lock(_mutex);
        if (!_inputActive)
        {
            return;
        }
        if (down)
        {
            _context->ProcessMouseButtonDown(num, keyState);
        }
        else
        {
            _context->ProcessMouseButtonUp(num, keyState);
        }
    }

    bool RmluiViewImpl::update() noexcept
    {
        std::scoped_lock lock(_mutex);
        if (!_enabled)
        {
            return false;
        }
        return _context->Update();
    }

    void RmluiViewImpl::renderPassDefine(RenderPassDefinition& def) noexcept
    {
        def.setName("Rmlui " + getName());
        def.setPriority(_priority);
    }

    void RmluiViewImpl::renderPassConfigure(bgfx::ViewId viewId) noexcept
    {
        _render.setViewId(viewId);
    }

    void RmluiViewImpl::renderPassExecute(IRenderGraphContext& context) noexcept
    {
        std::scoped_lock lock(_mutex);
        if (!_enabled)
        {
            return;
        }
        auto& encoder = context.getEncoder();
        _render.beforeRender(encoder);
        if (!_context->Render())
        {
            return;
        }
        auto cursorSprite = getMouseCursorSprite();
        if (cursorSprite)
        {
            _render.renderMouseCursor(cursorSprite.value(), _mousePosition);
        }
        _render.afterRender();
    }

    OptionalRef<const Rml::Sprite> RmluiViewImpl::getMouseCursorSprite() const noexcept
    {
        for (int i = 0; i < _context->GetNumDocuments(); i++)
        {
            auto sprite = getMouseCursorSprite(*_context->GetDocument(i));
            if (sprite)
            {
                return sprite;
            }
        }
        return nullptr;
    }

    OptionalRef<const Rml::Sprite> RmluiViewImpl::getMouseCursorSprite(Rml::ElementDocument& doc) const noexcept
    {
        auto style = doc.GetStyleSheet();
        if (style == nullptr)
        {
            return nullptr;
        }

        std::string cursorName = _comp.getSystem().getMouseCursor();
        auto getSprite = [&]() -> const Rml::Sprite*
            {
                if (cursorName.empty())
                {
                    return nullptr;
                }
                return style->GetSprite(std::string("cursor-") + cursorName);
            };

        if (auto sprite = getSprite())
        {
            return sprite;
        }
        auto prop = doc.GetProperty("cursor");
        if (prop == nullptr)
        {
            return nullptr;
        }
        cursorName = prop->Get<std::string>();
        if (auto sprite = getSprite())
        {
            return sprite;
        }
        return nullptr;
    }

    RmluiView::RmluiView(std::unique_ptr<RmluiViewImpl>&& impl) noexcept
        : _impl(std::move(impl))
    {
    }

    RmluiView::~RmluiView() noexcept
    {
        // empty on purpose
    }

    std::string RmluiView::getName() const noexcept
    {
        return _impl->getName();
    }

    bool RmluiView::getFullscreen() const noexcept
    {
        return _impl->getFullscreen();
    }

    RmluiView& RmluiView::setFullscreen(bool enabled) noexcept
    {
        _impl->setFullscreen(enabled);
        return *this;
    }

    const Viewport& RmluiView::getViewport() const noexcept
    {
        return _impl->getViewport();
    }

    void RmluiView::setViewport(const Viewport& viewport) noexcept
    {
        _impl->setViewport(viewport);
    }

    RmluiView& RmluiView::setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept
    {
        _impl->setTargetTexture(texture);
        return *this;
    }


    RmluiView& RmluiView::setEnabled(bool enabled) noexcept
    {
        _impl->setEnabled(enabled);
        return *this;
    }

    bool RmluiView::getEnabled() const noexcept
    {
        return _impl->getEnabled();
    }

    std::shared_ptr<Texture> RmluiView::getTargetTexture() noexcept
    {
        return _impl->getTargetTexture();
    }

    RmluiView& RmluiView::setInputActive(bool active) noexcept
    {
        _impl->setInputActive(active);
        return *this;
    }

    bool RmluiView::getInputActive() const noexcept
    {
        return _impl->getInputActive();
    }

    RmluiView& RmluiView::setMousePosition(const glm::vec2& position) noexcept
    {
        _impl->setMousePosition(position);
        return *this;
    }

    const glm::vec2& RmluiView::getMousePosition() const noexcept
    {
        return _impl->getMousePosition();
    }

    Rml::Context& RmluiView::getContext() noexcept
    {
        return _impl->getContext();
    }

    const Rml::Context& RmluiView::getContext() const noexcept
    {
        return _impl->getContext();
    }

    RmluiViewImpl& RmluiView::getImpl() noexcept
    {
        return *_impl;
    }

    const RmluiViewImpl& RmluiView::getImpl() const noexcept
    {
        return *_impl;
    }

    RmluiAppComponentImpl::~RmluiAppComponentImpl() noexcept
    {
        if (_app)
        {
            shutdown();
        }
    }

    void RmluiAppComponentImpl::init(App& app)
    {
        _system.init(app);
        _file.init(app);

        Rml::SetSystemInterface(&_system);
        Rml::SetFileInterface(&_file);

        Rml::Initialise();

        ProgramDefinition progDef;
        progDef.loadStaticMem(rmlui_program);
        _program = std::make_shared<Program>(progDef);

        _app = app;

        auto& defaultView = getView();
        defaultView.setFullscreen(true);
        defaultView.setInputActive(true);

        onWindowPixelSize(app.getWindow().getPixelSize());
        onMousePositionChange({}, app.getInput().getMouse().getPosition());

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

        for (auto& view : _views)
        {
            _app->getRenderGraph().removePass(view->getImpl());
        }

        Rml::ReleaseTextures();

        _views.clear();
        _app.reset();
        _program.reset();

        Rml::Shutdown();
    }
    
    RmluiSystemInterface& RmluiAppComponentImpl::getSystem() noexcept
    {
        return _system;
    }

    bx::AllocatorI& RmluiAppComponentImpl::getAllocator() noexcept
    {
        return _app->getAssets().getAllocator();
    }

    std::shared_ptr<Program> RmluiAppComponentImpl::getProgram() const noexcept
    {
        return _program;
    }

    RmluiAppComponentImpl::Views& RmluiAppComponentImpl::getViews() noexcept
    {
        return _views;
    }

    RmluiAppComponentImpl::Views::iterator RmluiAppComponentImpl::findView(const std::string& name) noexcept
    {
        return std::find_if(_views.begin(), _views.end(), [&name](auto& view) { return view->getName() == name; });
    }

    RmluiAppComponentImpl::Views::const_iterator RmluiAppComponentImpl::findView(const std::string& name) const noexcept
    {
        return std::find_if(_views.begin(), _views.end(), [&name](auto& view) { return view->getName() == name; });
    }

    bool RmluiAppComponentImpl::hasView(const std::string& name) const noexcept
    {
        return findView(name) != _views.end();
    }

    bool RmluiAppComponentImpl::removeView(const std::string& name)
    {
        auto itr = findView(name);
        if (itr == _views.end())
        {
            return false;
        }
        Rml::ReleaseTextures();
        _app->getRenderGraph().removePass((*itr)->getImpl());
        _views.erase(itr);
        return true;
    }

    OptionalRef<const RmluiView> RmluiAppComponentImpl::getView(const std::string& name) const noexcept
    {
        auto itr = findView(name);
        if (itr == _views.end())
        {
            return nullptr;
        }
        return itr->get();
    }

    std::unique_ptr<RmluiView> RmluiAppComponentImpl::createView(const std::string& name, int priority) noexcept
    {
        auto& size = _app->getWindow().getFramebufferSize();
        return std::make_unique<RmluiView>(
            std::make_unique<RmluiViewImpl>(name, priority, size, *this)
        );
    }

    RmluiView& RmluiAppComponentImpl::getView(const std::string& name)
    {
        auto itr = findView(name);
        if (itr != _views.end())
        {
            return **itr;
        }
        auto& view = _views.emplace_back(createView(name));
        _app->getRenderGraph().addPass(view->getImpl());
        return *view;
    }

    RmluiView& RmluiAppComponentImpl::addView(const std::string& name, int priority)
    {
        auto itr = findView(name);
        if (itr != _views.end())
        {
            throw std::runtime_error("view name already exists");
        }
        auto& view = *_views.emplace(_views.begin(), createView(name, priority));
        _app->getRenderGraph().addPass(view->getImpl());
        return *view;
    }

    void RmluiAppComponentImpl::update(float dt) noexcept
    {
        _system.update(dt);
        for (auto& view : _views)
        {
            view->getImpl().update();
        }
    }

    void RmluiAppComponentImpl::renderReset() noexcept
    {
        auto& graph = _app->getRenderGraph();
        for (auto& view : _views)
        {
            graph.addPass(view->getImpl());
        }
    }

    void RmluiAppComponentImpl::onWindowPixelSize(const glm::uvec2& size) noexcept
    {
        for (auto& view : _views)
        {
            if (!view->getFullscreen())
            {
                continue;
            }
            view->setViewport(size);
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
            { KeyboardModifier::Ctrl, Rml::Input::KM_CTRL },
            { KeyboardModifier::Shift, Rml::Input::KM_SHIFT },
            { KeyboardModifier::Alt, Rml::Input::KM_ALT },
            { KeyboardModifier::Meta, Rml::Input::KM_META },
            { KeyboardKey::CapsLock, Rml::Input::KM_CAPSLOCK },
            { KeyboardKey::NumLock, Rml::Input::KM_NUMLOCK },
            { KeyboardKey::ScrollLock, Rml::Input::KM_SCROLLLOCK },
        };
        return map;
    }

    int RmluiAppComponentImpl::getKeyModifierState() const noexcept
    {
        int state = 0;
        if (!_app)
        {
            return state;
        }
        auto& kb = _app->getInput().getKeyboard();
        auto& mods = kb.getModifiers();

        for (auto& elm : getKeyboardModifierMap())
        {
            if (auto modPtr = std::get_if<KeyboardModifier>(&elm.first))
            {
                if (mods.contains(*modPtr))
                {
                    state |= elm.second;
                }
            }
            else if (kb.getKey(std::get<KeyboardKey>(elm.first)))
            {
                state |= elm.second;
            }
        }
        return 0;
    }

    void RmluiAppComponentImpl::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& mods, bool down) noexcept
    {
        auto& keyMap = getKeyboardMap();
        auto itr = keyMap.find(key);
        if (itr == keyMap.end())
        {
            return;
        }
        auto& rmlKey = itr->second;
        auto state = getKeyModifierState();
        for (auto& view : _views)
        {
            view->getImpl().processKey(rmlKey, state, down);
        }
    }

    void RmluiAppComponentImpl::onKeyboardChar(const Utf8Char& chr) noexcept
    {
        Rml::String str = chr.toString();
        for (auto& view : _views)
        {
            view->getImpl().processTextInput(str);
        }
    }

    void RmluiAppComponentImpl::onMouseActive(bool active) noexcept
    {
        if (active)
        {
            return;
        }
        for (auto& view : _views)
        {
            view->getImpl().processMouseLeave();
        }
    }

    void RmluiAppComponentImpl::onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept
    {
        if (!_app)
        {
            return;
        }
        // transform from window to window screen
        auto pos = _app->getWindow().windowToScreenPoint(absolute);
        // transform to normalized position
        pos = pos / glm::vec2(_app->getWindow().getFramebufferSize());

        for (auto& view : _views)
        {
            if (!view->getInputActive())
            {
                continue;
            }
            auto& vp = view->getViewport();
            // transform from normalized to viewport screen
            auto screenPos = vp.viewportToScreenPoint(pos);
            view->setMousePosition(screenPos);
        }

    }

    void RmluiAppComponentImpl::onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept
    {
        auto rmlDelta = RmluiUtils::convert<float>(delta) * -1;
        auto state = getKeyModifierState();
        for (auto& view : _views)
        {
            view->getImpl().processMouseWheel(rmlDelta, state);
        }
    }

    void RmluiAppComponentImpl::onMouseButton(MouseButton button, bool down) noexcept
    {
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
        auto state = getKeyModifierState();
        for (auto& view : _views)
        {
            view->getImpl().processMouseButton(i, state, down);
        }
    }

    RmluiAppComponent::RmluiAppComponent() noexcept
        : _impl(std::make_unique<RmluiAppComponentImpl>())
    {
    }

    RmluiAppComponent::~RmluiAppComponent() noexcept
    {
        // left empty to get the forward declaration of the impl working
    }

    RmluiAppComponentImpl& RmluiAppComponent::getImpl() noexcept
    {
        return *_impl;
    }

    const RmluiAppComponentImpl& RmluiAppComponent::getImpl() const noexcept
    {
        return *_impl;
    }

    void RmluiAppComponent::init(App& app)
    {
        _impl->init(app);
    }

    void RmluiAppComponent::shutdown() noexcept
    {
        _impl->shutdown();
    }

    void RmluiAppComponent::renderReset() noexcept
    {
        _impl->renderReset();
    }

    void RmluiAppComponent::updateLogic(float dt) noexcept
    {
        _impl->update(dt);
    }

    bool RmluiAppComponent::removeView(const std::string& name)
    {
        return _impl->removeView(name);
    }

    bool RmluiAppComponent::hasView(const std::string& name) const noexcept
    {
        return _impl->hasView(name);
    }

    OptionalRef<const RmluiView> RmluiAppComponent::getView(const std::string& name) const noexcept
    {
        return _impl->getView(name);
    }

    RmluiView& RmluiAppComponent::getView(const std::string& name)
    {
        return _impl->getView(name);
    }

    RmluiView& RmluiAppComponent::addView(const std::string& name, int priority)
    {
        return _impl->addView(name, priority);
    }
}