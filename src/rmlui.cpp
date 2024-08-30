#include <darmok/rmlui.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/texture.hpp>
#include <darmok/image.hpp>
#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/shape.hpp>
#include <darmok/scene.hpp>
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
            _rmluiTransform = glm::mat4(1);
        }
        else
        {
            _rmluiTransform = (glm::mat4)*transform->data();
        }
    }

    RmluiRenderInterface::RmluiRenderInterface(const std::shared_ptr<Program>& prog, bx::AllocatorI& alloc) noexcept
        : _textureUniform(bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler))
        , _scissorEnabled(false)
        , _scissor(0)
        , _rmluiTransform(1)
        , _sceneTransform(1)
        , _alloc(alloc)
        , _program(prog)
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
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_MSAA
        | BGFX_STATE_BLEND_ALPHA
        ;

    glm::mat4 RmluiRenderInterface::getTransform(const glm::vec2& position)
    {
        // reverse the y axis
        auto scale = glm::scale(glm::mat4(1), glm::vec3(1, -1, 1));
        glm::vec3 pos(
            position.x + _viewport.origin.x,
            position.y - _viewport.origin.y - _viewport.size.y,
            0.F);
        return _sceneTransform * _rmluiTransform * glm::translate(scale, pos);
    }

    void RmluiRenderInterface::submitGeometry(Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept
    {
        if (!_program || !_encoder || !_viewId)
        {
            return;
        }

        auto trans = getTransform(RmluiUtils::convert(translation));
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

    void RmluiRenderInterface::beforeRender(IRenderGraphContext& context, const glm::mat4& sceneTransform, const Viewport& viewport) noexcept
    {
        _encoder = context.getEncoder();
        _viewId = context.getViewId();
        _rmluiTransform = glm::mat4(1);
        _sceneTransform = sceneTransform;
        _viewport = viewport;
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

        auto trans = getTransform(position);
        _encoder->setTransform(glm::value_ptr(trans));

        mesh->render(_encoder.value());
        _encoder->setState(_state);
        _encoder->submit(_viewId.value(), _program->getHandle());
    }

    void RmluiRenderInterface::afterRender() noexcept
    {
        _viewId.reset();
        _encoder.reset();
        _rmluiTransform = glm::mat4(1);
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

    RmluiCanvasImpl::RmluiCanvasImpl(const std::string& name, const std::optional<Viewport>& vp)
        : _inputActive(false)
        , _mousePosition(0)
        , _viewport(vp)
        , _enabled(true)
        , _name(name)
        , _mousePositionMode(MousePositionMode::Delta)
    {
    }

    void RmluiCanvasImpl::init(RmluiRendererImpl& comp)
    {
        _comp = comp;
        auto size = getCurrentViewport().size;
        _render.emplace(comp.getProgram(), comp.getAllocator());
        _context = Rml::CreateContext(_name, RmluiUtils::convert<int>(size), &_render.value());
        if (!_context)
        {
            throw std::runtime_error("Failed to create rmlui context");
        }
        _context->EnableMouseCursor(true);
    }

    void RmluiCanvasImpl::shutdown() noexcept
    {
        if (_context)
        {
            Rml::RemoveContext(_context->GetName());
            _context.reset();
        }

        Rml::ReleaseTextures();

        _comp.reset();
        _render.reset();
    }

    void RmluiCanvasImpl::renderReset() noexcept
    {
        if (!_viewport && _comp && _context)
        {
            if (auto cam = _comp->getCamera())
            {
                auto size = cam->getCurrentViewport().size;
                _context->SetDimensions(RmluiUtils::convert<int>(size));
            }
        }
    }

    bool RmluiCanvasImpl::getInputActive() const noexcept
    {
        return _inputActive;
    }

    void RmluiCanvasImpl::setInputActive(bool active) noexcept
    {
        _inputActive = active;
    }

    void RmluiCanvasImpl::setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept
    {
        _context->SetDefaultScrollBehavior(behaviour, speedFactor);
    }

    bool RmluiCanvasImpl::getEnabled() const noexcept
    {
        return _enabled;
    }

    void RmluiCanvasImpl::setEnabled(bool enabled) noexcept
    {
        _enabled = enabled;
    }

    void RmluiCanvasImpl::setMousePositionMode(MousePositionMode mode) noexcept
    {
        _mousePositionMode = mode;
    }

    RmluiCanvasImpl::MousePositionMode RmluiCanvasImpl::getMousePositionMode() const noexcept
    {
        return _mousePositionMode;
    }

    Viewport RmluiCanvasImpl::getCurrentViewport() const noexcept
    {
        if (_viewport)
        {
            return _viewport.value();
        }
        if (_comp)
        {
            if (auto cam = _comp->getCamera())
            {
                return cam->getCurrentViewport();
            }
        }
        return glm::uvec2(0);
    }

    const std::optional<Viewport>& RmluiCanvasImpl::getViewport() const noexcept
    {
        return _viewport;
    }

    void RmluiCanvasImpl::setViewport(const std::optional<Viewport>& vp) noexcept
    {
        if (_viewport != vp)
        {
            _viewport = vp;
            renderReset();
        }
    }

    std::string RmluiCanvasImpl::getName() const noexcept
    {
        return _context->GetName();
    }

    Rml::Context& RmluiCanvasImpl::getContext() noexcept
    {
        return _context.value();
    }

    const Rml::Context& RmluiCanvasImpl::getContext() const noexcept
    {
        return _context.value();
    }

    RmluiCanvasImpl::~RmluiCanvasImpl() noexcept
    {
        if (_context)
        {
            Rml::RemoveContext(_context->GetName());
        }
    }

    const glm::vec2& RmluiCanvasImpl::getMousePosition() const noexcept
    {
        return _mousePosition;
    }

    void RmluiCanvasImpl::setMousePosition(const glm::vec2& position) noexcept
    {
        auto pos = position;

        auto vp = getCurrentViewport();
        pos.x = Math::clamp(pos.x, 0.F, (float)vp.size.x);
        pos.y = Math::clamp(pos.y, 0.F, (float)vp.size.y);

        _mousePosition = pos;

        int modState = 0;
        if (_comp)
        {
            modState = _comp->getKeyModifierState();
        }
        _context->ProcessMouseMove(pos.x, pos.y, modState);
    }

    void RmluiCanvasImpl::setViewportMousePosition(const glm::vec2& position, OptionalRef<const Transform> canvasTrans)
    {
        if (!_comp)
        {
            return;
        }
        auto cam = _comp->getCamera();
        if (!cam)
        {
            return;
        }

        glm::vec3 pos(position, 0.F);
        auto ray = cam->viewportPointToRay(pos);
        if (canvasTrans)
        {
            ray *= canvasTrans->getWorldInverse();
        }
        if (auto dist = ray.intersect(Plane(glm::vec3(0, 0, -1))))
        {
            auto pos = ray * dist.value();
            setLocalMousePosition(pos);
        }
    }

    glm::vec2 RmluiCanvasImpl::getLocalMousePosition() const noexcept
    {
        auto vp = getCurrentViewport();
        auto pos = _mousePosition;
        pos.y = vp.size.y - pos.y;
        pos += glm::vec2(vp.origin);
        return pos;
    }

    void RmluiCanvasImpl::setLocalMousePosition(const glm::vec2& position) noexcept
    {
        auto vp = getCurrentViewport();
        auto pos = position;
        pos -= glm::vec2(vp.origin);
        pos.y = vp.size.y - pos.y;
        setMousePosition(pos);
    }

    void RmluiCanvasImpl::addViewportMousePositionDelta(const glm::vec2& delta, OptionalRef<const Transform> canvasTrans) noexcept
    {
        if (!_comp)
        {
            return;
        }
        auto cam = _comp->getCamera();
        if (!cam)
        {
            return;
        }

        glm::vec4 pos(getLocalMousePosition(), 0, 1);

        if (canvasTrans)
        {
            pos = canvasTrans->getWorldMatrix() * pos;
        }
        pos = glm::vec4(cam->worldToViewportPoint(pos), 1);
        pos += glm::vec4(delta, 0, 0);
        pos = glm::vec4(cam->viewportToWorldPoint(pos), 1);
        if (canvasTrans)
        {
            pos = canvasTrans->getWorldInverse() * pos;
        }
        
        setLocalMousePosition(pos);
    }

    void RmluiCanvasImpl::processKey(Rml::Input::KeyIdentifier key, int state, bool down) noexcept
    {
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

    void RmluiCanvasImpl::processTextInput(const Rml::String& str) noexcept
    {
        if (!_inputActive)
        {
            return;
        }
        _context->ProcessTextInput(str);
    }

    void RmluiCanvasImpl::processMouseLeave() noexcept
    {
        if (!_inputActive)
        {
            return;
        }
        _context->ProcessMouseLeave();
    }

    void RmluiCanvasImpl::processMouseWheel(const Rml::Vector2f& val, int keyState) noexcept
    {
        if (!_inputActive)
        {
            return;
        }
        _context->ProcessMouseWheel(val, keyState);
    }

    void RmluiCanvasImpl::processMouseButton(int num, int keyState, bool down) noexcept
    {
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

    bool RmluiCanvasImpl::update() noexcept
    {
        if (!_enabled)
        {
            return false;
        }
        return _context->Update();
    }

    void RmluiCanvasImpl::render(IRenderGraphContext& context, const glm::mat4& trans) noexcept
    {
        if (!_enabled || !_render)
        {
            return;
        }
        _render->beforeRender(context, trans, getCurrentViewport());
        if (!_context->Render())
        {
            return;
        }
        auto cursorSprite = getMouseCursorSprite();
        if (cursorSprite)
        {
            auto pos = getMousePosition();
            _render->renderMouseCursor(cursorSprite.value(), pos);
        }
        _render->afterRender();
    }

    OptionalRef<const Rml::Sprite> RmluiCanvasImpl::getMouseCursorSprite() const noexcept
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

    OptionalRef<const Rml::Sprite> RmluiCanvasImpl::getMouseCursorSprite(Rml::ElementDocument& doc) const noexcept
    {
        if (!_comp)
        {
            return nullptr;
        }
        auto style = doc.GetStyleSheet();
        if (style == nullptr)
        {
            return nullptr;
        }

        std::string cursorName = _comp->getSystem().getMouseCursor();
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

    RmluiCanvas::RmluiCanvas(const std::string& name, const std::optional<Viewport>& vp) noexcept
        : _impl(std::make_unique<RmluiCanvasImpl>(name, vp))
    {
    }

    RmluiCanvas::~RmluiCanvas() noexcept
    {
        // empty on purpose
    }

    std::string RmluiCanvas::getName() const noexcept
    {
        return _impl->getName();
    }

    const std::optional<Viewport>& RmluiCanvas::getViewport() const noexcept
    {
        return _impl->getViewport();
    }

    Viewport RmluiCanvas::getCurrentViewport() const noexcept
    {
        return _impl->getCurrentViewport();
    }

    RmluiCanvas& RmluiCanvas::setViewport(const std::optional<Viewport>& vp) noexcept
    {
        _impl->setViewport(vp);
        return *this;
    }

    RmluiCanvas& RmluiCanvas::setMousePositionMode(MousePositionMode mode) noexcept
    {
        _impl->setMousePositionMode(mode);
        return *this;
    }

    RmluiCanvas::MousePositionMode RmluiCanvas::getMousePositionMode() const noexcept
    {
        return _impl->getMousePositionMode();
    }

    RmluiCanvas& RmluiCanvas::setEnabled(bool enabled) noexcept
    {
        _impl->setEnabled(enabled);
        return *this;
    }

    bool RmluiCanvas::getEnabled() const noexcept
    {
        return _impl->getEnabled();
    }

    RmluiCanvas& RmluiCanvas::setInputActive(bool active) noexcept
    {
        _impl->setInputActive(active);
        return *this;
    }

    bool RmluiCanvas::getInputActive() const noexcept
    {
        return _impl->getInputActive();
    }

    void RmluiCanvas::setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept
    {
        _impl->setScrollBehavior(behaviour, speedFactor);
    }

    RmluiCanvas& RmluiCanvas::setMousePosition(const glm::vec2& position) noexcept
    {
        _impl->setMousePosition(position);
        return *this;
    }

    const glm::vec2& RmluiCanvas::getMousePosition() const noexcept
    {
        return _impl->getMousePosition();
    }

    Rml::Context& RmluiCanvas::getContext() noexcept
    {
        return _impl->getContext();
    }

    const Rml::Context& RmluiCanvas::getContext() const noexcept
    {
        return _impl->getContext();
    }

    RmluiCanvasImpl& RmluiCanvas::getImpl() noexcept
    {
        return *_impl;
    }

    const RmluiCanvasImpl& RmluiCanvas::getImpl() const noexcept
    {
        return *_impl;
    }

    RmluiRendererImpl::~RmluiRendererImpl() noexcept
    {
        if (_app)
        {
            shutdown();
        }
    }

    void RmluiRendererImpl::init(Camera& cam, Scene& scene, App& app)
    {
        _system.init(app);
        _file.init(app);

        _cam = cam;
        _scene = scene;
        _app = app;

        Rml::SetSystemInterface(&_system);
        Rml::SetFileInterface(&_file);

        Rml::Initialise();

        ProgramDefinition progDef;
        progDef.loadStaticMem(rmlui_program);
        _program = std::make_shared<Program>(progDef);

        app.getInput().getKeyboard().addListener(*this);
        app.getInput().getMouse().addListener(*this);

        for (auto entity : _cam->createEntityView<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            canvas->getImpl().init(*this);
        }

        scene.getRegistry().on_construct<RmluiCanvas>().connect<&RmluiRendererImpl::onCanvasConstructed>(*this);
        scene.getRegistry().on_destroy<RmluiCanvas>().connect<&RmluiRendererImpl::onCanvasDestroyed>(*this);
    }

    void RmluiRendererImpl::onCanvasConstructed(EntityRegistry& registry, Entity entity)
    {
        if (!_scene)
        {
            return;
        }
        if (auto canvas = _scene->getComponent<RmluiCanvas>(entity))
        {
            canvas->getImpl().init(*this);
        }
    }

    void RmluiRendererImpl::onCanvasDestroyed(EntityRegistry& registry, Entity entity)
    {
        if (!_scene)
        {
            return;
        }
        if (auto canvas = _scene->getComponent<RmluiCanvas>(entity))
        {
            canvas->getImpl().shutdown();
        }
    }

    void RmluiRendererImpl::shutdown()
    {
        if (_app)
        {
            _app->getInput().getKeyboard().removeListener(*this);
            _app->getInput().getMouse().removeListener(*this);
        }

        if (_scene)
        {
            _scene->getRegistry().on_construct<Camera>().disconnect<&RmluiRendererImpl::onCanvasConstructed>(*this);
            _scene->getRegistry().on_destroy<Camera>().disconnect<&RmluiRendererImpl::onCanvasDestroyed>(*this);
        }


        if (_cam)
        {
            for (auto entity : _cam->createEntityView<RmluiCanvas>())
            {
                auto canvas = _scene->getComponent<RmluiCanvas>(entity);
                canvas->getImpl().shutdown();
            }
        }

        Rml::ReleaseTextures();

        _app.reset();
        _cam.reset();
        _scene.reset();
        _program.reset();

        Rml::Shutdown();
    }
    
    RmluiSystemInterface& RmluiRendererImpl::getSystem() noexcept
    {
        return _system;
    }

    bx::AllocatorI& RmluiRendererImpl::getAllocator()
    {
        return _app->getAssets().getAllocator();
    }

    std::shared_ptr<Program> RmluiRendererImpl::getProgram() const noexcept
    {
        return _program;
    }

    OptionalRef<const Camera> RmluiRendererImpl::getCamera() const noexcept
    {
        return _cam;
    }

    void RmluiRendererImpl::update(float dt) noexcept
    {
        _system.update(dt);
        if (_cam)
        {
            for (auto entity : _cam->createEntityView<RmluiCanvas>())
            {
                auto canvas = _scene->getComponent<RmluiCanvas>(entity);
                canvas->getImpl().update();
            }
        }
    }

    void RmluiRendererImpl::renderReset() noexcept
    {
        if (!_cam)
        {
            return;
        }
        for (auto entity : _cam->createEntityView<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            canvas->getImpl().renderReset();
        }
    }

    void RmluiRendererImpl::beforeRenderView(IRenderGraphContext& context)
    {
        if (!_cam)
        {
            return;
        }
        for (auto entity : _cam->createEntityView<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            glm::mat4 mtx(1);
            if (auto trans = _scene->getComponent<const Transform>(entity))
            {
                mtx = trans->getWorldMatrix();
            }
            canvas->getImpl().render(context, mtx);
        }
    }

    const RmluiRendererImpl::KeyboardMap& RmluiRendererImpl::getKeyboardMap() noexcept
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

    const RmluiRendererImpl::KeyboardModifierMap& RmluiRendererImpl::getKeyboardModifierMap() noexcept
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

    int RmluiRendererImpl::getKeyModifierState() const noexcept
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

    void RmluiRendererImpl::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& mods, bool down) noexcept
    {
        if (!_cam)
        {
            return;
        }
        auto& keyMap = getKeyboardMap();
        auto itr = keyMap.find(key);
        if (itr == keyMap.end())
        {
            return;
        }
        auto& rmlKey = itr->second;
        auto state = getKeyModifierState();
        for (auto entity : _cam->createEntityView<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            canvas->getImpl().processKey(rmlKey, state, down);
        }
    }

    void RmluiRendererImpl::onKeyboardChar(const Utf8Char& chr) noexcept
    {
        if (!_cam)
        {
            return;
        }
        Rml::String str = chr.toString();
        for (auto entity : _cam->createEntityView<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            canvas->getImpl().processTextInput(str);
        }
    }

    void RmluiRendererImpl::onMouseActive(bool active) noexcept
    {
        if (active || !_cam)
        {
            return;
        }
        for (auto entity : _cam->createEntityView<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            canvas->getImpl().processMouseLeave();
        }
    }

    void RmluiRendererImpl::onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept
    {
        if (!_app || !_cam)
        {
            return;
        }
        auto& win = _app->getWindow();
        auto vp = _cam->getCurrentViewport();

        // transform to normalized position

        auto screenDelta = win.windowToScreenDelta(delta);
        auto vpDelta = vp.screenToViewportDelta(screenDelta);

        auto screenPos = win.windowToScreenPoint(absolute);
        auto vpPos = vp.screenToViewportPoint(screenPos);
            
        for (auto entity : _cam->createEntityView<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            if (!canvas->getInputActive())
            {
                continue;
            }
            auto mode = canvas->getMousePositionMode();
            if (mode == RmluiCanvasMousePositionMode::Disabled)
            {
                continue;
            }
            auto trans = _scene->getComponent<const Transform>(entity);
            if (mode == RmluiCanvasMousePositionMode::Delta)
            {
                canvas->getImpl().addViewportMousePositionDelta(vpDelta, trans);
            }
            else
            {
                canvas->getImpl().setViewportMousePosition(vpPos, trans);
            }
        }
    }

    void RmluiRendererImpl::onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept
    {
        if (!_cam)
        {
            return;
        }
        auto rmlDelta = RmluiUtils::convert<float>(delta) * -1;
        auto state = getKeyModifierState();
        for (auto entity : _cam->createEntityView<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            canvas->getImpl().processMouseWheel(rmlDelta, state);
        }
    }

    void RmluiRendererImpl::onMouseButton(MouseButton button, bool down) noexcept
    {
        if (!_cam)
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
        auto state = getKeyModifierState();
        for (auto entity : _cam->createEntityView<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            canvas->getImpl().processMouseButton(i, state, down);
        }
    }

    RmluiRenderer::RmluiRenderer() noexcept
        : _impl(std::make_unique<RmluiRendererImpl>())
    {
    }

    RmluiRenderer::~RmluiRenderer() noexcept
    {
        // left empty to get the forward declaration of the impl working
    }

    void RmluiRenderer::init(Camera& cam, Scene& scene, App& app)
    {
        _impl->init(cam, scene, app);
    }

    void RmluiRenderer::shutdown() noexcept
    {
        _impl->shutdown();
    }

    void RmluiRenderer::renderReset() noexcept
    {
        _impl->renderReset();
    }

    void RmluiRenderer::beforeRenderView(IRenderGraphContext& context)
    {
        _impl->beforeRenderView(context);
    }

    void RmluiRenderer::update(float dt) noexcept
    {
        _impl->update(dt);
    }

    RmluiRendererImpl& RmluiRenderer::getImpl() noexcept
    {
        return *_impl;
    }

    const RmluiRendererImpl& RmluiRenderer::getImpl() const noexcept
    {
        return *_impl;
    }

}