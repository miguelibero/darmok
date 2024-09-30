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
#include <filesystem>

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

    glm::mat4 RmluiUtils::convert(const Rml::Matrix4f& v) noexcept
    {
        return {
            convert(v.GetColumn(0)),
            convert(v.GetColumn(1)),
            convert(v.GetColumn(2)),
            convert(v.GetColumn(3))
        };
    }

    Rectangle RmluiUtils::convert(const Rml::Rectanglef& v) noexcept
    {
        return Rectangle(RmluiUtils::convert(v.Size()), RmluiUtils::convert(v.Position()));
    }

    RmluiRenderInterface::RmluiRenderInterface(App& app) noexcept
        : _app(app)
        , _scissorEnabled(false)
        , _scissor(0)
        , _trans(1)
        , _baseTrans(1)
    {
        _textureUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

        ProgramDefinition progDef;
        progDef.loadStaticMem(rmlui_program);
        _program = std::make_unique<Program>(progDef);
    }

    RmluiRenderInterface::~RmluiRenderInterface() noexcept
    {
        if (isValid(_textureUniform))
        {
            bgfx::destroy(_textureUniform);
            _textureUniform.idx = bgfx::kInvalidHandle;
        }
    }

    Rml::CompiledGeometryHandle RmluiRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) noexcept
    {
        DataView vertData(vertices.data(), sizeof(Rml::Vertex) * vertices.size());
        DataView idxData(indices.data(), sizeof(int) * indices.size());
        MeshConfig meshConfig;
        meshConfig.index32 = true;
        auto mesh = std::make_unique<Mesh>(_program->getVertexLayout(), vertData, idxData, meshConfig);
        Rml::CompiledGeometryHandle handle = mesh->getVertexHandle().idx + 1;
        _meshes.emplace(handle, std::move(mesh));
        return handle;
    }

    void RmluiRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) noexcept
    {
        if (!_context)
        {
            return;
        }
        auto meshItr = _meshes.find(geometry);
        if (meshItr == _meshes.end())
        {
            return;
        }

        auto viewId = _context->getViewId();
        auto& encoder = _context->getEncoder();

        meshItr->second->render(encoder);

        auto trans = getTransformMatrix(RmluiUtils::convert(translation));
        encoder.setTransform(glm::value_ptr(trans));

        ProgramDefines defines{ "TEXTURE_DISABLE" };
        auto texItr = _textures.find(texture);
        if (texItr != _textures.end())
        {
            encoder.setTexture(0, _textureUniform, texItr->second->getHandle());
            defines.clear();
        }
        encoder.setState(_state);
        encoder.submit(viewId, _program->getHandle(defines));
    }

    void RmluiRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle handle) noexcept
    {
        auto itr = _meshes.find(handle);
        if (itr != _meshes.end())
        {
            _meshes.erase(itr);
        }
    }

    Rml::TextureHandle RmluiRenderInterface::LoadTexture(Rml::Vector2i& dimensions, const Rml::String& source) noexcept
    {
        const auto file = Rml::GetFileInterface();
        const auto fileHandle = file->Open(source);

        if (!fileHandle)
        {
            return false;
        }

        auto& alloc = _app.getAssets().getAllocator();
        Data data(file->Length(fileHandle), alloc);
        file->Read(data.ptr(), data.size(), fileHandle);
        file->Close(fileHandle);

        try
        {
            Image img(data, alloc);
            auto size = img.getSize();
            dimensions.x = size.x;
            dimensions.y = size.y;
            auto texture = std::make_unique<Texture>(img);
            Rml::TextureHandle handle = texture->getHandle().idx + 1;
            _textureSources.emplace(source, *texture);
            _textures.emplace(handle, std::move(texture));
            return handle;
        }
        catch (...)
        {
            return 0;
        }
    }

    Rml::TextureHandle RmluiRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i dimensions) noexcept
    {
        auto size = 4 * sizeof(Rml::byte) * dimensions.x * dimensions.y;
        TextureConfig config;
        config.format = bgfx::TextureFormat::RGBA8;
        config.size = glm::uvec2(dimensions.x, dimensions.y);
        DataView data(source.data(), size);

        try
        {
            auto texture = std::make_unique<Texture>(data, config);
            Rml::TextureHandle handle = texture->getHandle().idx + 1;
            _textures.emplace(handle, std::move(texture));
            return handle;
        }
        catch (...)
        {
            return 0;
        }
    }

    void RmluiRenderInterface::ReleaseTexture(Rml::TextureHandle handle) noexcept
    {
        auto itr = _textures.find(handle);
        if (itr != _textures.end())
        {
            auto ptr = itr->second.get();
            auto itr2 = std::find_if(_textureSources.begin(), _textureSources.end(),
                [ptr](auto& elm) { return &elm.second.get() == ptr; });
            if (itr2 != _textureSources.end())
            {
                _textureSources.erase(itr2);
            }
            _textures.erase(itr);
        }
    }

    void RmluiRenderInterface::SetTransform(const Rml::Matrix4f* transform) noexcept
    {
        if (transform == nullptr)
        {
            _trans = glm::mat4(1);
        }
        else
        {
            _trans = RmluiUtils::convert(*transform);
        }
    }

    const uint64_t RmluiRenderInterface::_state = 0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_MSAA
        | BGFX_STATE_BLEND_ALPHA
        ;

    glm::mat4 RmluiRenderInterface::getTransformMatrix(const glm::vec2& position)
    {
        glm::vec3 pos(position.x, position.y, 0.F);
        auto mtx = glm::translate(glm::mat4(1), pos);
        return _baseTrans * _trans * mtx;
    }

    void RmluiRenderInterface::EnableScissorRegion(bool enable) noexcept
    {
        if (!_context)
        {
            return;
        }
        _scissorEnabled = enable;
        auto viewId = _context->getViewId();
        if (enable)
        {
            bgfx::setViewScissor(viewId, _scissor.x, _scissor.y, _scissor.z, _scissor.w);
        }
        else
        {
            bgfx::setViewScissor(viewId);
        }
    }

    void RmluiRenderInterface::SetScissorRegion(Rml::Rectanglei region) noexcept
    {
        _scissor = glm::ivec4(RmluiUtils::convert(region.Position()), RmluiUtils::convert(region.Size()));
        if (_scissorEnabled)
        {
            EnableScissorRegion(true);
        }
    }

    void RmluiRenderInterface::renderCanvas(RmluiCanvasImpl& canvas, IRenderGraphContext& context) noexcept
    {
        std::lock_guard lock(_canvasMutex);
        _context = context;
        _baseTrans = canvas.getRenderMatrix();
        
        if (!canvas.getContext().Render())
        {
            return;
        }

        if (auto cursorSprite = canvas.getMouseCursorSprite())
        {
            renderSprite(cursorSprite.value(), canvas.getMousePosition());
        }

        _context.reset();
        _baseTrans = glm::mat4();
    }

    OptionalRef<Texture> RmluiRenderInterface::getSpriteTexture(const Rml::Sprite& sprite) noexcept
    {
        auto& texSrc = sprite.sprite_sheet->texture_source;
        std::filesystem::path src = texSrc.GetSource();
        if (!src.is_absolute())
        {
            std::filesystem::path defSrc(texSrc.GetDefinitionSource());
            src = defSrc.parent_path() / src;
        }
        auto itr = _textureSources.find(src.string());
        if (itr == _textureSources.end())
        {
            Rml::Vector2i dim;
            LoadTexture(dim, src.string());
            itr = _textureSources.find(src.string());
        }
        if (itr == _textureSources.end())
        {
            return nullptr;
        }
        return itr->second.get();
    }

    bool RmluiRenderInterface::renderSprite(const Rml::Sprite& sprite, const glm::vec2& position) noexcept
    {
        auto texture = getSpriteTexture(sprite);
        if (!texture)
        {
            return false;
        }
        auto rect = RmluiUtils::convert(sprite.rectangle);
        auto atlasElm = TextureAtlasElement::create(TextureAtlasBounds{ rect.size, rect.origin });

        TextureAtlasMeshConfig config;
        config.type = MeshType::Transient;
        auto mesh = atlasElm.createSprite(_program->getVertexLayout(), texture->getSize(), config);

        auto& encoder = _context->getEncoder();
        auto viewId = _context->getViewId();

        encoder.setTexture(0, _textureUniform, texture->getHandle());

        auto trans = getTransformMatrix(position);
        encoder.setTransform(glm::value_ptr(trans));

        mesh->render(encoder);
        encoder.setState(_state);
        encoder.submit(viewId, _program->getHandle());

        return true;
    }

    RmluiSystemInterface::RmluiSystemInterface(App& app) noexcept
        : _elapsedTime(0)
    {
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

    RmluiFileInterface::RmluiFileInterface(App& app)
        : _dataLoader(app.getAssets().getDataLoader())
    {
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

    RmluiCanvasImpl::RmluiCanvasImpl(RmluiCanvas& canvas, const std::string& name, const std::optional<glm::uvec2>& size)
        : _canvas(canvas)
        , _inputActive(false)
        , _mousePosition(0)
        , _size(size)
        , _visible(true)
        , _name(name)
        , _mousePositionMode(MousePositionMode::Relative)
        , _offset(0)
    {
    }

    RmluiCanvasImpl::~RmluiCanvasImpl() noexcept
    {
        shutdown();
    }

    void RmluiCanvasImpl::init(RmluiRendererImpl& comp)
    {
        if (_context)
        {
            shutdown();
        }

        _comp = comp;
        auto size = getCurrentSize();
        _context = Rml::CreateContext(_name, RmluiUtils::convert<int>(size), &_comp->getRmluiRender());
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
            Rml::ReleaseTextures();
        }
        _comp.reset();
    }

    void RmluiCanvasImpl::updateContextSize() noexcept
    {
        if (!_context)
        {
            return;
        }
        auto size = getCurrentSize();
        _context->SetDimensions(RmluiUtils::convert<int>(size));
    }

    void RmluiCanvasImpl::renderReset() noexcept
    {
        updateContextSize();
    }

    bool RmluiCanvasImpl::isInputActive() const noexcept
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

    bool RmluiCanvasImpl::isVisible() const noexcept
    {
        return _visible;
    }

    void RmluiCanvasImpl::setVisible(bool visible) noexcept
    {
        _visible = visible;
    }

    void RmluiCanvasImpl::setMousePositionMode(MousePositionMode mode) noexcept
    {
        _mousePositionMode = mode;
    }

    RmluiCanvasImpl::MousePositionMode RmluiCanvasImpl::getMousePositionMode() const noexcept
    {
        return _mousePositionMode;
    }

    glm::uvec2 RmluiCanvasImpl::getCurrentSize() const noexcept
    {
        if (_size)
        {
            return _size.value();
        }
        if (_comp)
        {
            return _comp->getViewportSize();
        }
        return glm::uvec2(0);
    }

    const std::optional<glm::uvec2>& RmluiCanvasImpl::getSize() const noexcept
    {
        return _size;
    }

    void RmluiCanvasImpl::setSize(const std::optional<glm::uvec2>& size) noexcept
    {
        if (_size != size)
        {
            _size = size;
            updateContextSize();
        }
    }

    const glm::vec3& RmluiCanvasImpl::getOffset() const noexcept
    {
        return _offset;
    }

    void RmluiCanvasImpl::setOffset(const glm::vec3& offset) noexcept
    {
        _offset = offset;
    }

    std::string RmluiCanvasImpl::getName() const noexcept
    {
        return _context->GetName();
    }

    glm::mat4 RmluiCanvasImpl::getRenderMatrix() const noexcept
    {
        if (!_comp)
        {
            return glm::mat4(1);
        }
        auto trans = _comp->getTransform(_canvas);
        auto baseModel = getBaseModelMatrix();
        if (trans)
        {
            // if the canvas has a transform we use that
            return trans->getWorldMatrix() * baseModel;
        }

        auto mtx = _comp->getDefaultProjectionMatrix() * baseModel;
        if (auto cam = _comp->getCamera())
        {
            // if the canvas does not have a transform, it means we want to use the full screen
            // so we have to invert the camera view proj since it will be in the bgfx view transform
            mtx = cam->getModelInverse() * glm::inverse(cam->getProjectionMatrix()) * mtx;
        }
        return mtx;
    }

    glm::mat4 RmluiCanvasImpl::getProjectionMatrix() const noexcept
    {
        if (!_comp)
        {
            return glm::mat4(1);
        }
        if (auto trans = _comp->getTransform(_canvas))
        {
            if (auto cam = _comp->getCamera())
            {
                return cam->getProjectionMatrix();
            }
        }
        return _comp->getDefaultProjectionMatrix();
    }

    glm::mat4 RmluiCanvasImpl::getModelMatrix() const noexcept
    {
        if (!_comp)
        {
            return glm::mat4(1);
        }

        glm::mat4 model(1);

        if (auto trans = _comp->getTransform(_canvas))
        {
            if (auto cam = _comp->getCamera())
            {
                model *= cam->getModelMatrix();
            }
            model *= trans->getWorldMatrix();
        }

        model *= getBaseModelMatrix();

        return model;
    }

    glm::mat4 RmluiCanvasImpl::getBaseModelMatrix() const noexcept
    {
        auto size = getCurrentSize();
        auto offset = _offset;
        offset.y += size.y;
        auto model = glm::translate(glm::mat4(1), offset);
        // invert the y axis because rmlui renders upside down
        model = glm::scale(model, glm::vec3(1, -1, 1));
        return model;
    }

    Rml::Context& RmluiCanvasImpl::getContext()
    {
        return _context.value();
    }

    const Rml::Context& RmluiCanvasImpl::getContext() const
    {
        return _context.value();
    }

    OptionalRef<RmluiRendererImpl> RmluiCanvasImpl::getComponent() noexcept
    {
        return _comp;
    }

    OptionalRef<const RmluiRendererImpl> RmluiCanvasImpl::getComponent() const noexcept
    {
        return _comp;
    }

    const glm::vec2& RmluiCanvasImpl::getMousePosition() const noexcept
    {
        return _mousePosition;
    }

    void RmluiCanvasImpl::setMousePosition(const glm::vec2& position) noexcept
    {
        auto pos = position;

        auto size = getCurrentSize();
        pos.x = Math::clamp(pos.x, 0.F, (float)size.x);
        pos.y = Math::clamp(pos.y, 0.F, (float)size.y);

        _mousePosition = pos;

        int modState = 0;
        if (_comp)
        {
            modState = _comp->getKeyModifierState();
        }
        _context->ProcessMouseMove(pos.x, pos.y, modState);
    }

    void RmluiCanvasImpl::setViewportMousePosition(const glm::vec2& position) noexcept
    {
        if (position.x < 0.F || position.x > 1.F)
        {
            return;
        }
        if (position.y < 0.F || position.y > 1.F)
        {
            return;
        }

        glm::vec3 pos(position, 0.F);

        auto model = getModelMatrix();
        auto proj = getProjectionMatrix();
        auto ray = Ray::unproject(pos, model, proj);

        if (auto dist = ray.intersect(Plane(glm::vec3(0, 0, -1))))
        {
            pos = ray * dist.value();
            setMousePosition(pos);
        }
    }

    glm::vec2 RmluiCanvasImpl::getViewportMousePosition() const noexcept
    {
        glm::vec3 pos(getMousePosition(), 0);

        auto model = getModelMatrix();
        auto proj = getProjectionMatrix();
        auto vp = Viewport().getValues();

        return glm::project(pos, model, proj, vp);
    }

    void RmluiCanvasImpl::applyViewportMousePositionDelta(const glm::vec2& delta) noexcept
    {
        if (delta.x == 0.F && delta.y == 0.F)
        {
            return;
        }

        glm::vec3 pos(getMousePosition(), 0);

        auto model = getModelMatrix();
        auto proj = getProjectionMatrix();
        auto vp = Viewport().getValues();

        pos = glm::project(pos, model, proj, vp);
        pos += glm::vec3(delta, 0);
        pos = glm::unProject(pos, model, proj, vp);
        
        setMousePosition(pos);
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
        return _context->Update();
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

        std::string cursorName = _comp->getRmluiSystem().getMouseCursor();
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

    void RmluiCanvasImpl::addListener(IRmluiCanvasListener& listener) noexcept
    {
        _listeners.insert(listener);
    }

    void RmluiCanvasImpl::addListener(std::unique_ptr<IRmluiCanvasListener>&& listener) noexcept
    {
        _listeners.insert(std::move(listener));
    }

    bool RmluiCanvasImpl::removeListener(const IRmluiCanvasListener& listener) noexcept
    {
        return _listeners.erase(listener);
    }

    void RmluiCanvasImpl::onCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element)
    {
        for(auto& listener : _listeners)
        {
            listener.onRmluiCustomEvent(event, value, element);
        }
    }

    bool RmluiCanvasImpl::loadScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine)
    {
        for (auto& listener : _listeners)
        {
            if (listener.loadRmluiScript(doc, content, sourcePath, sourceLine))
            {
                return true;
            }
        }
        return false;
    }

    void RmluiCanvasImpl::render(IRenderGraphContext& context) noexcept
    {
        if (!_visible || !_comp || !_context)
        {
            return;
        }

        auto& render = _comp->getRmluiRender();
        render.renderCanvas(*this, context);        
    }

    RmluiCanvas::RmluiCanvas(const std::string& name, const std::optional<glm::uvec2>& size) noexcept
        : _impl(std::make_unique<RmluiCanvasImpl>(*this, name, size))
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

    const std::optional<glm::uvec2>& RmluiCanvas::getSize() const noexcept
    {
        return _impl->getSize();
    }

    glm::uvec2 RmluiCanvas::getCurrentSize() const noexcept
    {
        return _impl->getCurrentSize();
    }

    RmluiCanvas& RmluiCanvas::setSize(const std::optional<glm::uvec2>& size) noexcept
    {
        _impl->setSize(size);
        return *this;
    }

    const glm::vec3& RmluiCanvas::getOffset() const noexcept
    {
        return _impl->getOffset();
    }

    RmluiCanvas& RmluiCanvas::setOffset(const glm::vec3& offset) noexcept
    {
        _impl->setOffset(offset);
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

    RmluiCanvas& RmluiCanvas::setVisible(bool visible) noexcept
    {
        _impl->setVisible(visible);
        return *this;
    }

    bool RmluiCanvas::isVisible() const noexcept
    {
        return _impl->isVisible();
    }

    RmluiCanvas& RmluiCanvas::setInputActive(bool active) noexcept
    {
        _impl->setInputActive(active);
        return *this;
    }

    bool RmluiCanvas::isInputActive() const noexcept
    {
        return _impl->isInputActive();
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

    RmluiCanvas& RmluiCanvas::applyViewportMousePositionDelta(const glm::vec2& delta) noexcept
    {
        _impl->applyViewportMousePositionDelta(delta);
        return *this;
    }

    RmluiCanvas& RmluiCanvas::setViewportMousePosition(const glm::vec2& position) noexcept
    {
        _impl->setViewportMousePosition(position);
        return *this;
    }

    glm::vec2 RmluiCanvas::getViewportMousePosition() const noexcept
    {
        return _impl->getViewportMousePosition();
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

    RmluiCanvas& RmluiCanvas::addListener(IRmluiCanvasListener& listener) noexcept
    {
        _impl->addListener(listener);
        return *this;
    }

    RmluiCanvas& RmluiCanvas::addListener(std::unique_ptr<IRmluiCanvasListener>&& listener) noexcept
    {
        _impl->addListener(std::move(listener));
        return *this;
    }

    bool RmluiCanvas::removeListener(const IRmluiCanvasListener& listener) noexcept
    {
        return _impl->removeListener(listener);
    }

    RmluiPlugin::RmluiPlugin(App& app) noexcept
        : _app(app)
        , _render(app)
        , _system(app)
        , _file(app)
    {
        Rml::SetSystemInterface(&_system);
        Rml::SetFileInterface(&_file);

        Rml::Initialise();

        Rml::RegisterPlugin(this);
        Rml::Factory::RegisterEventListenerInstancer(this);
        Rml::Factory::RegisterElementInstancer("body", this);
    }

    RmluiPlugin::~RmluiPlugin() noexcept
    {
        Rml::Factory::RegisterEventListenerInstancer(nullptr);
        Rml::UnregisterPlugin(this);

        _eventForwarders.clear();

        Rml::Shutdown();
    }

    RmluiSystemInterface& RmluiPlugin::getSystem() noexcept
    {
        return _system;
    }

    RmluiRenderInterface& RmluiPlugin::getRender() noexcept
    {
        return _render;
    }

    std::weak_ptr<RmluiPlugin> RmluiPlugin::_instance;

    std::weak_ptr<RmluiPlugin> RmluiPlugin::getWeakInstance() noexcept
    {
        return _instance;
    }

    std::shared_ptr<RmluiPlugin> RmluiPlugin::getInstance(App& app) noexcept
    {
        auto ptr = _instance.lock();
        if (ptr)
        {
            return ptr;
        }
        ptr = std::shared_ptr<RmluiPlugin>(new RmluiPlugin(app));
        _instance = ptr;
        return ptr;
    }

    Rml::EventListener* RmluiPlugin::InstanceEventListener(const Rml::String& value, Rml::Element* element)
    {
        auto ptr = std::make_unique<RmluiEventForwarder>(value, *element);
        return _eventForwarders.emplace_back(std::move(ptr)).get();
    }

    void RmluiPlugin::OnDocumentUnload(Rml::ElementDocument* doc) noexcept
    {
        auto itr = std::remove_if(_eventForwarders.begin(), _eventForwarders.end(), [doc](auto& fwd) {
            return fwd->getElement().GetOwnerDocument() == doc;
        });
        _eventForwarders.erase(itr, _eventForwarders.end());
    }

    void RmluiPlugin::onCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element)
    {
        for (auto& renderer : _renderers)
        {
            renderer.get().onCustomEvent(event, value, element);
        }
    }

    void RmluiPlugin::addRenderer(RmluiRendererImpl& renderer) noexcept
    {
        _renderers.emplace_back(renderer);
    }

    bool RmluiPlugin::removeRenderer(const RmluiRendererImpl& renderer) noexcept
    {
        auto ptr = &renderer;
        auto itr = std::remove_if(_renderers.begin(), _renderers.end(),
            [ptr](auto& ref) { return &ref.get() == ptr; });
        if (itr == _renderers.end())
        {
            return false;
        }
        _renderers.erase(itr, _renderers.end());
        return true;
    }

    bool RmluiPlugin::loadScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine)
    {
        for (auto& renderer : _renderers)
        {
            if (renderer.get().loadScript(doc, content, sourcePath, sourceLine))
            {
                return true;
            }
        }
        return false;
    }

    bool RmluiPlugin::loadExternalScript(Rml::ElementDocument& doc, const std::string& sourcePath)
    {
        auto data = _app.getAssets().getDataLoader()(sourcePath);
        if (data.empty())
        {
            return false;
        }
        return loadScript(doc, data.stringView(), sourcePath);
    }

    Rml::ElementPtr RmluiPlugin::InstanceElement(Rml::Element* parent, const Rml::String& tag, const Rml::XMLAttributes& attributes) noexcept
    {
        return Rml::ElementPtr(new RmluiDocument(*this, tag));
    }

    void RmluiPlugin::ReleaseElement(Rml::Element* element) noexcept
    {
        delete element;
    }

    RmluiDocument::RmluiDocument(RmluiPlugin& plugin, const Rml::String& tag)
        : Rml::ElementDocument(tag)
        , _plugin(plugin)
    {
    }

    void RmluiDocument::LoadInlineScript(const Rml::String& content, const Rml::String& sourcePath, int sourceLine)
    {
        _plugin.loadScript(*this, content, sourcePath, sourceLine);
    }

    void RmluiDocument::LoadExternalScript(const Rml::String& sourcePath)
    {
        _plugin.loadExternalScript(*this, sourcePath);
    }

    RmluiEventForwarder::RmluiEventForwarder(const std::string& value, Rml::Element& element) noexcept
        : _value(value)
        , _element(element)
    {
    }

    RmluiEventForwarder::~RmluiEventForwarder() noexcept
    {
        remove();
    }

    const std::string RmluiEventForwarder::_eventAttrPrefix = "on";

    void RmluiEventForwarder::remove() noexcept
    {
        for (auto& [name, attr] : _element.GetAttributes())
        {
            std::string val = attr.Get<Rml::String>();
            if (name.starts_with(_eventAttrPrefix) && val == _value)
            {
                _element.RemoveEventListener(name.substr(_eventAttrPrefix.size()), this);
            }
        }
    }

    const std::string& RmluiEventForwarder::getValue() const noexcept
    {
        return _value;
    }

    Rml::Element& RmluiEventForwarder::getElement() noexcept
    {
        return _element;
    }

    void RmluiEventForwarder::ProcessEvent(Rml::Event& event)
    {
        if (auto shared = RmluiPlugin::getWeakInstance().lock())
        {
            shared->onCustomEvent(event, _value, _element);
        }
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
        _plugin = RmluiPlugin::getInstance(app);
        _plugin->addRenderer(*this);

        _cam = cam;
        _scene = scene;
        _app = app;

        _renderGraph.clear();
        _renderGraph.setName("Rmlui");

        app.getInput().getKeyboard().addListener(*this);
        app.getInput().getMouse().addListener(*this);

        for (auto entity : _cam->getEntities<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            if (!canvas->getImpl().getComponent())
            {
                canvas->getImpl().init(*this);
            }
        }

        scene.onConstructComponent<RmluiCanvas>().connect<&RmluiRendererImpl::onCanvasConstructed>(*this);
        scene.onDestroyComponent<RmluiCanvas>().connect<&RmluiRendererImpl::onCanvasDestroyed>(*this);

        _cam->getRenderGraph().setChild(_renderGraph);
    }

    void RmluiRendererImpl::onCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element)
    {
        auto doc = element.GetOwnerDocument();
        if (!doc)
        {
            return;
        }
        auto context = doc->GetContext();
        if (!context)
        {
            return;
        }
        for (auto entity : _cam->getEntities<RmluiCanvas>())
        {
            auto& canvas = _scene->getComponent<RmluiCanvas>(entity)->getImpl();
            if (&canvas.getContext() == context)
            {
                canvas.onCustomEvent(event, value, element);
                break;
            }
        }
    }

    bool RmluiRendererImpl::loadScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine)
    {
        auto context = doc.GetContext();
        if (!context)
        {
            return false;
        }
        for (auto entity : _cam->getEntities<RmluiCanvas>())
        {
            auto& canvas = _scene->getComponent<RmluiCanvas>(entity)->getImpl();
            if (&canvas.getContext() == context)
            {
                if (canvas.loadScript(doc, content, sourcePath, sourceLine))
                {
                    return true;
                }
            }
        }
        return false;
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
            if (canvas->getImpl().getComponent() == this)
            {
                canvas->getImpl().shutdown();
            }
        }
    }

    void RmluiRendererImpl::shutdown()
    {
        _plugin.reset();

        if (_app)
        {
            _app->getInput().getKeyboard().removeListener(*this);
            _app->getInput().getMouse().removeListener(*this);
        }

        if (_scene)
        {
            _scene->onConstructComponent<Camera>().disconnect<&RmluiRendererImpl::onCanvasConstructed>(*this);
            _scene->onDestroyComponent<Camera>().disconnect<&RmluiRendererImpl::onCanvasDestroyed>(*this);
        }

        if (_cam)
        {
            for (auto entity : _cam->getEntities<RmluiCanvas>())
            {
                auto canvas = _scene->getComponent<RmluiCanvas>(entity);
                canvas->getImpl().shutdown();
            }
        }

        if (_plugin)
        {
            _plugin->removeRenderer(*this);
        }

        Rml::ReleaseTextures();

        _app.reset();
        _cam.reset();
        _scene.reset();
    }
    
    RmluiSystemInterface& RmluiRendererImpl::getRmluiSystem() noexcept
    {
        return _plugin->getSystem();
    }

    RmluiRenderInterface& RmluiRendererImpl::getRmluiRender() noexcept
    {
        return _plugin->getRender();
    }

    bx::AllocatorI& RmluiRendererImpl::getAllocator()
    {
        return _app->getAssets().getAllocator();
    }

    OptionalRef<const Camera> RmluiRendererImpl::getCamera() const noexcept
    {
        return _cam;
    }

    glm::uvec2 RmluiRendererImpl::getViewportSize() const noexcept
    {
        if (_cam)
        {
            return _cam->getCurrentViewport().size;
        }
        if (_scene)
        {
            return _scene->getCurrentViewport().size;
        }
        if (_app)
        {
            return _app->getWindow().getPixelSize();
        }
        return glm::uvec2(1);
    }

    OptionalRef<Transform> RmluiRendererImpl::getTransform(const RmluiCanvas& canvas) noexcept
    {
        if (!_scene)
        {
            return nullptr;
        }
        auto entity = _scene->getEntity(canvas);
        if (entity == entt::null)
        {
            return nullptr;
        }
        return _scene->getComponent<Transform>(entity);
    }

    glm::mat4 RmluiRendererImpl::getDefaultProjectionMatrix() const noexcept
    {
        auto botLeft = glm::vec2(0);
        auto topRight = glm::vec2(getViewportSize());
        return Math::ortho(botLeft, topRight);
    }

    RenderGraphDefinition& RmluiRendererImpl::getRenderGraph() noexcept
    {
        return _renderGraph;
    }

    const RenderGraphDefinition& RmluiRendererImpl::getRenderGraph() const noexcept
    {
        return _renderGraph;
    }

    void RmluiRendererImpl::update(float dt) noexcept
    {
        getRmluiSystem().update(dt);
        if (_cam && _scene)
        {
            for (auto entity : _cam->getEntities<RmluiCanvas>())
            {
                auto canvas = _scene->getComponent<RmluiCanvas>(entity);
                canvas->getImpl().update();
            }
        }
        if (_cam)
        {
            _cam->getRenderGraph().setChild(_renderGraph);
        }
    }

    void RmluiRendererImpl::renderReset() noexcept
    {
        _renderGraph.clear();
        if (!_cam || !_scene)
        {
            return;
        }

        for (auto entity : _cam->getEntities<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            canvas->getImpl().renderReset();
        }
    }

    void RmluiRendererImpl::beforeRenderView(IRenderGraphContext& context) noexcept
    {
        if (!_scene)
        {
            return;
        }
        auto& encoder = context.getEncoder();
        for (auto entity : _cam->getEntities<RmluiCanvas>())
        {
            if (auto canvas = _scene->getComponent<RmluiCanvas>(entity))
            {
                _cam->setEntityTransform(entity, encoder);
                canvas->getImpl().render(context);
            }
        }
    }

    void RmluiRendererImpl::loadFont(const std::string& path, bool fallback) noexcept
    {
        Rml::LoadFontFace(path, fallback);
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
        for (auto entity : _cam->getEntities<RmluiCanvas>())
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
        for (auto entity : _cam->getEntities<RmluiCanvas>())
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
        for (auto entity : _cam->getEntities<RmluiCanvas>())
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
        auto screenDelta = win.windowToScreenDelta(delta);
        auto vpDelta = vp.screenToViewportDelta(screenDelta);
        
        auto screenPos = win.windowToScreenPoint(absolute);
        auto vpPos = vp.screenToViewportPoint(screenPos);

        for (auto entity : _cam->getEntities<RmluiCanvas>())
        {
            auto canvas = _scene->getComponent<RmluiCanvas>(entity);
            if (!canvas->isInputActive())
            {
                continue;
            }
            auto mode = canvas->getMousePositionMode();
            if (mode == RmluiCanvasMousePositionMode::Disabled)
            {
                continue;
            }
            if (mode == RmluiCanvasMousePositionMode::Relative)
            {
                canvas->getImpl().applyViewportMousePositionDelta(vpDelta);
            }
            else
            {
                canvas->getImpl().setViewportMousePosition(vpPos);
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
        for (auto entity : _cam->getEntities<RmluiCanvas>())
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
        for (auto entity : _cam->getEntities<RmluiCanvas>())
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

    void RmluiRenderer::loadFont(const std::string& path, bool fallback) noexcept
    {
        _impl->loadFont(path, fallback);
    }

    void RmluiRenderer::renderReset() noexcept
    {
        _impl->renderReset();
    }

    void RmluiRenderer::beforeRenderView(IRenderGraphContext& context) noexcept
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