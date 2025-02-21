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
#include <darmok/scene_filter.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/program_core.hpp>
#include <darmok/math.hpp>
#include <darmok/mesh.hpp>
#include <darmok/glm.hpp>

#include "rmlui.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

#include "generated/shaders/rmlui/rmlui.program.h"

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

    RmluiRenderInterface::RmluiRenderInterface(App& app, RmluiCanvasImpl& canvas) noexcept
        : _app(app)
        , _canvas(canvas)
        , _scissorEnabled(false)
        , _scissor(0)
        , _trans(1.F)
        , _viewId(0)
        , _textureUniform{ bgfx::kInvalidHandle }
        , _dataUniform{ bgfx::kInvalidHandle }
    {
        _textureUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
        _dataUniform = bgfx::createUniform("u_rmluiData", bgfx::UniformType::Vec4);

        ProgramDefinition progDef;
        progDef.loadStaticMem(rmlui_program);
        _program = std::make_unique<Program>(progDef);
    }

    RmluiRenderInterface::~RmluiRenderInterface() noexcept
    {
        const std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms{ _textureUniform, _dataUniform };
        for (auto ref : uniforms)
        {
            auto& uniform = ref.get();
            if (isValid(uniform))
            {
                bgfx::destroy(uniform);
                uniform.idx = bgfx::kInvalidHandle;
            }
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
        if (!_encoder)
        {
            return;
        }
        auto meshItr = _meshes.find(geometry);
        if (meshItr == _meshes.end())
        {
            return;
        }

        auto& encoder = _encoder.value();
        meshItr->second->render(encoder);

        auto position = RmluiUtils::convert(translation);

        OptionalRef<Texture> tex;
        auto texItr = _textures.find(texture);
        if (texItr != _textures.end())
        {
            tex = texItr->second.get();
        }

        submit(position, tex);
    }

    void RmluiRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle handle) noexcept
    {
        auto itr = _meshes.find(handle);
        if (itr != _meshes.end())
        {
            _meshes.erase(itr);
        }
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

        auto& encoder = _encoder.value();
        mesh->render(encoder);
        submit(position, texture);

        return true;
    }

    void RmluiRenderInterface::submit(const glm::vec2& position, const OptionalRef<Texture>& texture) noexcept
    {
        if (!_encoder)
        {
            return;
        }

        auto& encoder = _encoder.value();

        auto trans = getTransformMatrix(position);
        encoder.setTransform(glm::value_ptr(trans));

        ProgramDefines defines{ "TEXTURE_DISABLE" };
        if (texture)
        {
            encoder.setTexture(0, _textureUniform, texture->getHandle());
            defines.clear();
        }

        static const uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_ALPHA
            ;

        encoder.setState(state);
        encoder.submit(_viewId, _program->getHandle(defines));
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
            auto flags = _canvas.getTextureFlags(source);
            auto texture = std::make_unique<Texture>(img, flags);
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
            uint64_t flags = _canvas.getDefaultTextureFlags();
            auto texture = std::make_unique<Texture>(data, config, flags);
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
        _trans = glm::mat4(1.F);
        if (transform != nullptr)
        {
            _trans *= RmluiUtils::convert(*transform);
        }
    }
    
    /*

    // https://github.com/mikke89/RmlUi/blob/master/Backends/RmlUi_Renderer_GL2.cpp#L163
    void RmluiRenderInterface::RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f trans) noexcept
    {
        if (!_context)
        {
            return;
        }
        auto& encoder = _context->getEncoder();

        uint32_t failKeep = BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP;

        switch (operation)
        {
        case Rml::ClipMaskOperation::Set:
            encoder.setStencil(failKeep | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_OP_PASS_Z_REPLACE);
        break;
        case Rml::ClipMaskOperation::SetInverse:
            encoder.setStencil(failKeep | BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_OP_PASS_Z_REPLACE);
        break;
        case Rml::ClipMaskOperation::Intersect:
            encoder.setStencil(failKeep | BGFX_STENCIL_OP_PASS_Z_INCR);
        break;
        }

        RenderGeometry(geometry, trans, {});
    }

    Rml::LayerHandle RmluiRenderInterface::PushLayer() noexcept
    {

    }

    void RmluiRenderInterface::CompositeLayers(Rml::LayerHandle source, Rml::LayerHandle destination, Rml::BlendMode blendMode, Rml::Span<const Rml::CompiledFilterHandle> filters) noexcept
    {

    }

    void RmluiRenderInterface::PopLayer() noexcept
    {

    }

    Rml::TextureHandle RmluiRenderInterface::SaveLayerAsTexture() noexcept
    {

    }

    Rml::CompiledFilterHandle RmluiRenderInterface::SaveLayerAsMaskImage() noexcept
    {

    }

    Rml::CompiledFilterHandle RmluiRenderInterface::CompileFilter(const Rml::String& name, const Rml::Dictionary& params) noexcept
    {
        auto prog = _app.getAssets().getProgramLoader()(name);
        Rml::CompiledFilterHandle handle = randomIdType();
        auto& mat = _filterMaterials.emplace(handle, prog).first->second;
        return handle;
    }

    void RmluiRenderInterface::ReleaseFilter(Rml::CompiledFilterHandle filter) noexcept
    {
        _filterMaterials.erase(filter);
    }

    Rml::CompiledShaderHandle RmluiRenderInterface::CompileShader(const Rml::String& name, const Rml::Dictionary& params) noexcept
    {
        auto prog = _app.getAssets().getProgramLoader()(name);
        Rml::CompiledShaderHandle handle = randomIdType();
        auto& mat = _shaderMaterials.emplace(handle, prog).first->second;
        return handle;
    }
    
    void RmluiRenderInterface::RenderShader(Rml::CompiledShaderHandle shader, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) noexcept
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

        auto matItr = _shaderMaterials.find(shader);
        if (matItr == _shaderMaterials.end())
        {
            return;
        }

        auto& encoder = _context->getEncoder();

        meshItr->second->render(encoder);
        auto viewId = _context->getViewId();

        OptionalRef<Texture> tex;
        auto texItr = _textures.find(texture);
        if (texItr != _textures.end())
        {
            tex = texItr->second.get();
        }

        auto position = RmluiUtils::convert(translation);
        auto trans = getTransformMatrix(position);
        encoder.setTransform(glm::value_ptr(trans));

        auto& matComp = _app.getOrAddComponent<MaterialAppComponent>();
        matComp.renderSubmit(viewId, encoder, matItr->second);
    }

    void RmluiRenderInterface::ReleaseShader(Rml::CompiledShaderHandle shader) noexcept
    {
        _shaderMaterials.erase(shader);
    }

    */

    glm::mat4 RmluiRenderInterface::getTransformMatrix(const glm::vec2& position)
    {
        return glm::translate(_trans, glm::vec3(position, 0.F));
    }

    void RmluiRenderInterface::EnableScissorRegion(bool enable) noexcept
    {
        if (!_encoder)
        {
            return;
        }
        _scissorEnabled = enable;
        if (enable)
        {
            bgfx::setViewScissor(_viewId, _scissor.x, _scissor.y, _scissor.z, _scissor.w);
        }
        else
        {
            bgfx::setViewScissor(_viewId);
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

    void RmluiRenderInterface::renderCanvas(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        _viewId = viewId;
        _encoder = encoder;
        _trans = glm::mat4(1.F);

        if (!_canvas.getContext().Render())
        {
            return;
        }

        if (auto cursorSprite = _canvas.getMouseCursorSprite())
        {
            renderSprite(cursorSprite.value(), _canvas.getMousePosition());
        }

        _encoder.reset();
    }

    void RmluiRenderInterface::renderFrame(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        auto tex = _canvas.getFrameTexture();
        if (!tex)
        {
            return;
        }
        auto model = _canvas.getRenderMatrix();
        encoder.setTransform(glm::value_ptr(model));

        encoder.setTexture(0, _textureUniform, tex->getHandle());

        glm::vec3 data(0);
        if (auto depth = _canvas.getForcedDepth())
        {
            data.x = 1.F;
            data.y = depth.value();
        }
        encoder.setUniform(_dataUniform, glm::value_ptr(data));

        auto meshData = MeshData(Rectangle(_canvas.getCurrentSize()));
        meshData.type = MeshType::Transient;
        auto mesh = meshData.createMesh(_program->getVertexLayout());

        static const uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_MSAA
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_BLEND_ALPHA
            ;

        mesh->render(encoder);
        encoder.setState(state);
        encoder.submit(viewId, _program->getHandle());
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

    RmluiSystemInterface::RmluiSystemInterface(App& app) noexcept
        : _elapsedTime(0)
    {
    }

    void RmluiSystemInterface::update(float deltaTime) noexcept
    {
        _elapsedTime += deltaTime;
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
        }
        return (Rml::FileHandle)nullptr;
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

    bool RmluiFileInterface::LoadFile(const Rml::String& path, Rml::String& outData) noexcept
    {
        try
        {
            auto data = _dataLoader.value()(path);
            outData = data.stringView();
            return true;
        }
        catch (...)
        {
        }
        return false;
    }

    RmluiCanvasImpl::RmluiCanvasImpl(RmluiCanvas& canvas, const std::string& name, const std::optional<glm::uvec2>& size)
        : _canvas(canvas)
        , _inputEnabled(false)
        , _mousePosition(0)
        , _size(size)
        , _visible(true)
        , _name(name)
        , _mousePositionMode(MousePositionMode::Relative)
        , _offset(0)
        , _defaultTextureFlags(defaultTextureLoadFlags)
    {
    }

    RmluiCanvasImpl::~RmluiCanvasImpl() noexcept
    {
        shutdown();
    }

    void RmluiCanvasImpl::init(App& app, RmluiSceneComponentImpl& comp)
    {
        if (_context)
        {
            shutdown();
        }

        _comp = comp;
        auto size = getCurrentSize();
        _render = std::make_unique<RmluiRenderInterface>(app, *this);
        std::string name = _name;
        if (auto scene = comp.getScene())
        {
            name += " - scene " + std::to_string(scene->getId());
        }
        _context = Rml::CreateContext(name, RmluiUtils::convert<int>(size), _render.get());
        if (size.x > 0 && size.y > 0)
        {
            _frameBuffer.emplace(size, false);
        }
        if (!_context)
        {
            throw std::runtime_error("Failed to create rmlui context");
        }
        _context->EnableMouseCursor(true);
    }

    void RmluiCanvasImpl::shutdown() noexcept
    {
        _dataTypeRegisters.clear();
        _defaultDataTypeRegister.reset();

        if (_context)
        {
            Rml::RemoveContext(_context->GetName());
            _context.reset();
            Rml::ReleaseTextures(_render ? _render.get() : nullptr);
        }

        // the render interface should be destroyed here
        // but with Rmlui 6.0 that throws an assert
        // the fix is added here https://github.com/mikke89/RmlUi/issues/703
        
        // TODO: remove recycleRender once 6.1 is out on vcpkg
        // do this:
        // _render.reset();
        // Rml::ReleaseRenderManagers();
        // instead of this:
        if (_render)
        {
            _comp->recycleRender(std::move(_render));
        }

        _comp.reset();
        _cam.reset();
        _defaultCam.reset();
    }

    OptionalRef<Texture> RmluiCanvasImpl::getFrameTexture() const noexcept
    {
        if (!_frameBuffer)
        {
            return nullptr;
        }
        return *_frameBuffer->getTexture();
    }

    bool RmluiCanvasImpl::updateCurrentSize() noexcept
    {
        auto size = getCurrentSize();
        if (_context)
        {
            _context->SetDimensions(RmluiUtils::convert<int>(size));
        }
        if (size.x == 0 || size.y == 0)
        {
            _frameBuffer.reset();
            return true;
        }
        else if (!_frameBuffer || _frameBuffer->getSize() != size)
        {
            _frameBuffer.emplace(size, false);
            return true;
        }
        return false;
    }

    bgfx::ViewId RmluiCanvasImpl::renderReset(bgfx::ViewId viewId) noexcept
    {
        if (!_size)
        {
            updateCurrentSize();
        }

        _viewId = viewId;
        updateViewName();

        static const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL | BGFX_CLEAR_COLOR;
        bgfx::setViewClear(viewId, clearFlags, 1.F, 0U, 0);
        bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);
        configureViewSize(viewId);

        return ++viewId;
    }

    void RmluiCanvasImpl::setCamera(const OptionalRef<Camera>& camera) noexcept
    {
        if (_cam != camera)
        {
            _cam = camera;
            updateViewName();
        }
    }

    void RmluiCanvasImpl::updateViewName() noexcept
    {
        if (!_viewId)
        {
            return;
        }
        auto name = "Rmlui Canvas: " + _name;
        if (_cam)
        {
            name = _cam->getViewName(name);
        }
        bgfx::setViewName(_viewId.value(), name.c_str());
    }

    const OptionalRef<Camera>& RmluiCanvasImpl::getCamera() const noexcept
    {
        return _cam;
    }

    OptionalRef<Camera> RmluiCanvasImpl::getCurrentCamera() const noexcept
    {
        return _cam ? _cam : _defaultCam;
    }

    bool RmluiCanvasImpl::isInputEnabled() const noexcept
    {
        return _inputEnabled;
    }

    void RmluiCanvasImpl::setInputEnabled(bool enabled) noexcept
    {
        _inputEnabled = enabled;
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
        if (auto cam = getCurrentCamera())
        {
            return cam->getCombinedViewport().size;
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
            updateCurrentSize();
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

    const std::string& RmluiCanvasImpl::getName() const noexcept
    {
        return _name;
    }    

    Rml::Context& RmluiCanvasImpl::getContext()
    {
        return _context.value();
    }

    const Rml::Context& RmluiCanvasImpl::getContext() const
    {
        return _context.value();
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

        static const Plane plane(glm::vec3(0.F, 0.F, -1.F));

        if (plane.contains(ray.origin))
        {
            setMousePosition(ray.origin);
        }
        else if (auto dist = ray.intersect(plane))
        {
            pos = ray * dist.value();
            setMousePosition(pos);
        }
    }

    glm::vec2 RmluiCanvasImpl::getViewportMousePosition() const noexcept
    {
        const glm::vec3 pos(getMousePosition(), 0.F);

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

        glm::vec3 pos(getMousePosition(), 0.F);

        auto model = getModelMatrix();
        auto proj = getProjectionMatrix();
        auto vp = Viewport().getValues();

        pos = glm::project(pos, model, proj, vp);
        pos += glm::vec3(delta, 0.F);
        pos = glm::unProject(pos, model, proj, vp);
        
        setMousePosition(pos);
    }

    void RmluiCanvasImpl::processKey(Rml::Input::KeyIdentifier key, int state, bool down) noexcept
    {
        if (!_inputEnabled)
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
        if (!_inputEnabled)
        {
            return;
        }
        _context->ProcessTextInput(str);
    }

    void RmluiCanvasImpl::processMouseLeave() noexcept
    {
        if (!_inputEnabled)
        {
            return;
        }
        _context->ProcessMouseLeave();
    }

    void RmluiCanvasImpl::processMouseWheel(const Rml::Vector2f& val, int keyState) noexcept
    {
        if (!_inputEnabled)
        {
            return;
        }
        _context->ProcessMouseWheel(val, keyState);
    }

    void RmluiCanvasImpl::processMouseButton(int num, int keyState, bool down) noexcept
    {
        if (!_inputEnabled)
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

    void RmluiCanvasImpl::updateDefaultCamera() noexcept
    {
        if (!_comp)
        {
            return;
        }
        auto scene = _comp->getScene();
        if (!scene)
        {
            return;
        }
        auto entity = scene->getEntity(_canvas);
        for (auto [camEntity, cam] : scene->getComponents<Camera>().each())
        {
            if (cam.getEntities<RmluiCanvas>().contains(entity))
            {
                _defaultCam = cam;
                break;
            }
        }
    }

    bool RmluiCanvasImpl::update(float deltaTime) noexcept
    {
        updateDefaultCamera();
        if (_delegate)
        {
            _delegate->update(deltaTime);
        }
        return _context->Update();
    }

    OptionalRef<const Rml::Sprite> RmluiCanvasImpl::getMouseCursorSprite() const noexcept
    {
        if (!_context || !_comp)
        {
            return nullptr;
        }
        std::string cursorName = _comp->getRmluiSystem().getMouseCursor();
        if (cursorName.empty())
        {
            return nullptr;
        }
        auto elm = _context->GetHoverElement();
        if (!elm || !elm->IsVisible())
        {
            return nullptr;
        }
        auto style = elm->GetStyleSheet();
        if (!style)
        {
            return nullptr;
        }
        return style->GetSprite(std::string("cursor-") + cursorName);
    }

    void RmluiCanvasImpl::setDelegate(IRmluiCanvasDelegate& dlg) noexcept
    {
        _delegate = dlg;
        _delegatePtr.reset();
    }

    void RmluiCanvasImpl::setDelegate(std::unique_ptr<IRmluiCanvasDelegate>&& dlg) noexcept
    {
        _delegate = *dlg;
        _delegatePtr = std::move(dlg);
    }

    OptionalRef<IRmluiCanvasDelegate> RmluiCanvasImpl::getDelegate() const noexcept
    {
        return _delegate;
    }

    bool RmluiCanvasImpl::onCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element)
    {
        auto doc = element.GetOwnerDocument();
        if (!doc)
        {
            return false;
        }
        if (doc->GetContext() != _context.ptr())
        {
            return false;
        }
        if(_delegate)
        {
            _delegate->onRmluiCustomEvent(event, value, element);
        }
        return true;
    }

    bool RmluiCanvasImpl::loadScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine)
    {
        if (_delegate)
        {
            return _delegate->loadRmluiScript(doc, content, sourcePath, sourceLine);
        }
        return false;
    }

    void RmluiCanvasImpl::render(bgfx::Encoder& encoder) noexcept
    {
        if (!_viewId)
        {
            return;
        }

        auto viewId = _viewId.value();
        encoder.touch(viewId);

        if (!_visible || !_render || !_context)
        {
            return;
        }

        if (!_size && updateCurrentSize())
        {
            configureViewSize(viewId);
        }

        if (!_frameBuffer)
        {
            return;
        }

        _render->renderCanvas(_viewId.value(), encoder);
    }

    void RmluiCanvasImpl::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        if (_render)
        {
            _render->renderFrame(viewId, encoder);
        }
    }

    OptionalRef<Transform> RmluiCanvasImpl::getTransform() const noexcept
    {
        if (!_comp)
        {
            return nullptr;
        }
        auto scene = _comp->getScene();
        if (!scene)
        {
            return nullptr;
        }
        auto entity = scene->getEntity(_canvas);
        if (entity == entt::null)
        {
            return nullptr;
        }
        return scene->getComponent<Transform>(entity);
    }

    std::optional<float> RmluiCanvasImpl::getForcedDepth() const noexcept
    {
        if (getTransform())
        {
            return std::nullopt;
        }
        auto v = glm::vec4(_offset, 1.F);
        if (auto cam = getCurrentCamera())
        {
            v = cam->getProjectionMatrix() * v;
        }
        return Math::clamp(v.z / v.w, 0.F, 1.F);
    }

    glm::mat4 RmluiCanvasImpl::getDefaultProjectionMatrix() const noexcept
    {
        auto botLeft = glm::vec2(0.F);
        auto topRight = glm::vec2(getCurrentSize());
        return Math::ortho(botLeft, topRight, 0.F, 1.F);
    }

    glm::mat4 RmluiCanvasImpl::getProjectionMatrix() const noexcept
    {
        if (auto trans = getTransform())
        {
            if (auto cam = getCurrentCamera())
            {
                return cam->getProjectionMatrix();
            }
        }
        return getDefaultProjectionMatrix();
    }

    glm::mat4 RmluiCanvasImpl::getModelMatrix() const noexcept
    {
        static const glm::vec3 invy(1.F, -1.F, 1.F);

        auto model = glm::scale(glm::mat4(1.F), invy);

        auto offset = _offset;

        if (auto trans = getTransform())
        {
            if (auto cam = getCurrentCamera())
            {
                model *= cam->getViewMatrix();
            }
            model *= trans->getWorldMatrix();
            offset.y += getCurrentSize().y;
        }
        
        model = glm::translate(model, offset);
        model = glm::scale(model, invy);

        return model;
    }

    glm::mat4 RmluiCanvasImpl::getRenderMatrix() const noexcept
    {
        auto trans = getTransform();

        auto offset = _offset + glm::vec3(getCurrentSize(), 0) * 0.5F;
        auto baseModel = glm::translate(glm::mat4(1.F), offset);
        if (trans)
        {
            // if the canvas has a transform we use that
            return trans->getWorldMatrix() * baseModel;
        }

        auto mtx = getDefaultProjectionMatrix() * baseModel;
        if (auto cam = getCurrentCamera())
        {
            // if the canvas does not have a transform, it means we want to use the full screen
            // so we have to invert the camera view proj since it will be in the bgfx view transform
            mtx = cam->getViewInverse() * cam->getProjectionInverse() * mtx;
        }
        return mtx;
    }

    void RmluiCanvasImpl::configureViewSize(bgfx::ViewId viewId) const noexcept
    {
        if (_frameBuffer)
        {
            _frameBuffer->configureView(viewId);
        }

        auto size = getCurrentSize();

        Viewport(size).configureView(viewId);
        auto proj = getDefaultProjectionMatrix();

        static const glm::vec3 invy(1.F, -1.F, 1.F);
        auto view = glm::translate(glm::mat4(1.F), glm::vec3(0.F, size.y, 0.F));
        view = glm::scale(view, invy);

        bgfx::setViewTransform(viewId, glm::value_ptr(view), glm::value_ptr(proj));
    }

    uint64_t RmluiCanvasImpl::getDefaultTextureFlags() const noexcept
    {
        return _defaultTextureFlags;
    }

    void RmluiCanvasImpl::setDefaultTextureFlags(uint64_t flags) noexcept
    {
        _defaultTextureFlags = flags;
    }

    uint64_t RmluiCanvasImpl::getTextureFlags(const std::string& source) const noexcept
    {
        auto itr = _textureFlags.find(source);
        if (itr == _textureFlags.end())
        {
            return _defaultTextureFlags;
        }
        return itr->second;
    }

    void RmluiCanvasImpl::setTextureFlags(const std::string& source, uint64_t flags) noexcept
    {
        _textureFlags[source] = flags;
    }

    Rml::DataTypeRegister& RmluiCanvasImpl::getDefaultDataTypeRegister()
    {
        if (!_defaultDataTypeRegister)
        {
            static const std::string tempModelName;
            _defaultDataTypeRegister = _context->CreateDataModel(tempModelName).GetDataTypeRegister();
            _context->RemoveDataModel(tempModelName);
        }
        return _defaultDataTypeRegister.value();
    }

    Rml::DataModelConstructor RmluiCanvasImpl::createDataModel(const std::string& name)
    {
        if (!_context)
        {
            return {};
        }
        if (_dataTypeRegisters.contains(name))
        {
            return {};
        }
        auto reg = std::make_unique<Rml::DataTypeRegister>();
        auto constructor = _context->CreateDataModel(name, reg.get());
        _dataTypeRegisters[name] = std::move(reg);
        return constructor;
    }

    Rml::DataModelConstructor RmluiCanvasImpl::getDataModel(const std::string& name) noexcept
    {
        if (!_context)
        {
            return {};
        }
        return _context->GetDataModel(name);
    }

    bool RmluiCanvasImpl::removeDataModel(const std::string& name) noexcept
    {
        _dataTypeRegisters.erase(name);
        if (!_context)
        {
            return false;
        }
        return _context->RemoveDataModel(name);
    }

    RmluiCanvas::RmluiCanvas(const std::string& name, const std::optional<glm::uvec2>& size) noexcept
        : _impl(std::make_unique<RmluiCanvasImpl>(*this, name, size))
    {
    }

    RmluiCanvas::~RmluiCanvas() noexcept
    {
        // empty on purpose
    }

    const std::string& RmluiCanvas::getName() const noexcept
    {
        return _impl->getName();
    }

    RmluiCanvas& RmluiCanvas::setCamera(const OptionalRef<Camera>& camera) noexcept
    {
        _impl->setCamera(camera);
        return *this;
    }

    const OptionalRef<Camera>& RmluiCanvas::getCamera() const noexcept
    {
        return _impl->getCamera();
    }

    OptionalRef<Camera> RmluiCanvas::getCurrentCamera() const noexcept
    {
        return _impl->getCurrentCamera();
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

    RmluiCanvas& RmluiCanvas::setInputEnabled(bool enabled) noexcept
    {
        _impl->setInputEnabled(enabled);
        return *this;
    }

    bool RmluiCanvas::isInputEnabled() const noexcept
    {
        return _impl->isInputEnabled();
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

    RmluiCanvas& RmluiCanvas::setDelegate(IRmluiCanvasDelegate& dlg) noexcept
    {
        _impl->setDelegate(dlg);
        return *this;
    }

    RmluiCanvas& RmluiCanvas::setDelegate(std::unique_ptr<IRmluiCanvasDelegate>&& dlg) noexcept
    {
        _impl->setDelegate(std::move(dlg));
        return *this;
    }

    OptionalRef<IRmluiCanvasDelegate> RmluiCanvas::getDelegate() const noexcept
    {
        return _impl->getDelegate();
    }

    Rml::DataModelConstructor RmluiCanvas::createDataModel(const std::string& name)
    {
        return _impl->createDataModel(name);
    }

    Rml::DataModelConstructor RmluiCanvas::getDataModel(const std::string& name) noexcept
    {
        return _impl->getDataModel(name);
    }

    bool RmluiCanvas::removeDataModel(const std::string& name) noexcept
    {
        return _impl->removeDataModel(name);
    }

    uint64_t RmluiCanvas::getDefaultTextureFlags() const noexcept
    {
        return _impl->getDefaultTextureFlags();
    }

    RmluiCanvas& RmluiCanvas::setDefaultTextureFlags(uint64_t flags) noexcept
    {
        _impl->setDefaultTextureFlags(flags);
        return *this;
    }

    uint64_t RmluiCanvas::getTextureFlags(const std::string& source) const noexcept
    {
        return _impl->getTextureFlags(source);
    }

    RmluiCanvas& RmluiCanvas::setTextureFlags(const std::string& source, uint64_t flags) noexcept
    {
        _impl->setTextureFlags(source, flags);
        return *this;
    }

    RmluiPlugin::RmluiPlugin(App& app) noexcept
        : _app(app)
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
        for (auto& comp : _components)
        {
            comp.get().onCustomEvent(event, value, element);
        }
    }

    void RmluiPlugin::addComponent(RmluiSceneComponentImpl& comp) noexcept
    {
        _components.emplace_back(comp);
    }

    bool RmluiPlugin::removeComponent(const RmluiSceneComponentImpl& comp) noexcept
    {
        auto ptr = &comp;
        auto itr = std::remove_if(_components.begin(), _components.end(),
            [ptr](auto& ref) { return &ref.get() == ptr; });
        if (itr == _components.end())
        {
            return false;
        }
        _components.erase(itr, _components.end());
        return true;
    }

    bool RmluiPlugin::loadScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine)
    {
        for (auto& comp : _components)
        {
            if (comp.get().loadScript(doc, content, sourcePath, sourceLine))
            {
                return true;
            }
        }
        return false;
    }

    bool RmluiPlugin::loadExternalScript(Rml::ElementDocument& doc, const std::string& sourcePath)
    {
        auto data = _app.getAssets().getDataLoader()(sourcePath);
        return loadScript(doc, data.stringView(), sourcePath);
    }

    void RmluiPlugin::recycleRender(std::unique_ptr<RmluiRenderInterface>&& render) noexcept
    {
        _renders.push_back(std::move(render));
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
        for (const auto& [name, attr] : _element.GetAttributes())
        {
            const std::string val = attr.Get<Rml::String>();
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

    RmluiSceneComponentImpl::~RmluiSceneComponentImpl()
    {
        if (_app)
        {
            shutdown();
        }
    }

    void RmluiSceneComponentImpl::init(Scene& scene, App& app) noexcept
    {
        _plugin = RmluiPlugin::getInstance(app);
        _plugin->addComponent(*this);

        _scene = scene;
        _app = app;

        app.getInput().getKeyboard().addListener(*this);
        app.getInput().getMouse().addListener(*this);

        for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
        {
            canvas.getImpl().init(app, *this);
        }

        scene.onConstructComponent<RmluiCanvas>().connect<&RmluiSceneComponentImpl::onCanvasConstructed>(*this);
        scene.onDestroyComponent<RmluiCanvas>().connect<&RmluiSceneComponentImpl::onCanvasDestroyed>(*this);
    }

    void RmluiSceneComponentImpl::update(float deltaTime) noexcept
    {
        getRmluiSystem().update(deltaTime);
        if (!_scene)
        {
            return;
        }
        auto entities = _scene->getUpdateEntities<RmluiCanvas>();
        for (auto entity : entities)
        {
            auto& canvas = _scene->getComponent<RmluiCanvas>(entity).value();
            canvas.getImpl().update(deltaTime);
        }
    }

    bgfx::ViewId RmluiSceneComponentImpl::renderReset(bgfx::ViewId viewId) noexcept
    {
        if (_scene)
        {
            for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
            {
                viewId = canvas.getImpl().renderReset(viewId);
            }
        }

        return viewId;
    }

    void RmluiSceneComponentImpl::shutdown() noexcept
    {
        if (_app)
        {
            _app->getInput().getKeyboard().removeListener(*this);
            _app->getInput().getMouse().removeListener(*this);
        }

        if (_scene)
        {
            _scene->onConstructComponent<Camera>().disconnect<&RmluiSceneComponentImpl::onCanvasConstructed>(*this);
            _scene->onDestroyComponent<Camera>().disconnect<&RmluiSceneComponentImpl::onCanvasDestroyed>(*this);
            for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
            {
                canvas.getImpl().shutdown();
            }
        }

        if (_plugin)
        {
            _plugin->removeComponent(*this);
            _plugin.reset();
        }

        _app.reset();
        _scene.reset();

        Rml::ReleaseTextures();
    }

    RmluiSystemInterface& RmluiSceneComponentImpl::getRmluiSystem() noexcept
    {
        return _plugin->getSystem();
    }

    const OptionalRef<Scene>& RmluiSceneComponentImpl::getScene() const noexcept
    {
        return _scene;
    }

    void RmluiSceneComponentImpl::onCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element)
    {
        if (!_scene)
        {
            return;
        }
        for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
        {
            canvas.getImpl().onCustomEvent(event, value, element);
        }
    }

    bool RmluiSceneComponentImpl::loadScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine)
    {
        if (!_scene)
        {
            return false;
        }
        for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
        {
            if (canvas.getImpl().loadScript(doc, content, sourcePath, sourceLine))
            {
                return true;
            }
        }
        return false;
    }

    void RmluiSceneComponentImpl::recycleRender(std::unique_ptr<RmluiRenderInterface>&& render) noexcept
    {
        _plugin->recycleRender(std::move(render));
    }

    void RmluiSceneComponentImpl::onCanvasConstructed(EntityRegistry& registry, Entity entity)
    {
        if (!_scene || !_app)
        {
            return;
        }
        if (auto canvas = _scene->getComponent<RmluiCanvas>(entity))
        {
            canvas->getImpl().init(_app.value(), *this);
        }
    }

    void RmluiSceneComponentImpl::onCanvasDestroyed(EntityRegistry& registry, Entity entity)
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

    const RmluiSceneComponentImpl::KeyboardMap& RmluiSceneComponentImpl::getKeyboardMap() noexcept
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
            auto j = toUnderlying(rmluiStart);
            for (auto i = toUnderlying(start); i < toUnderlying(end); i++)
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

    const RmluiSceneComponentImpl::KeyboardModifierMap& RmluiSceneComponentImpl::getKeyboardModifierMap() noexcept
    {
        static const KeyboardModifierMap map
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

    int RmluiSceneComponentImpl::getKeyModifierState() const noexcept
    {
        int state = 0;
        if (!_app)
        {
            return state;
        }
        auto& keyb = _app->getInput().getKeyboard();
        auto& mods = keyb.getModifiers();

        for (auto& elm : getKeyboardModifierMap())
        {
            if (auto modPtr = std::get_if<KeyboardModifier>(&elm.first))
            {
                if (mods.contains(*modPtr))
                {
                    state |= elm.second;
                }
            }
            else if (keyb.getKey(std::get<KeyboardKey>(elm.first)))
            {
                state |= elm.second;
            }
        }
        return 0;
    }

    void RmluiSceneComponentImpl::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& mods, bool down) noexcept
    {
        if (!_scene)
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
        for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
        {
            canvas.getImpl().processKey(rmlKey, state, down);
        }
    }

    void RmluiSceneComponentImpl::onKeyboardChar(const UtfChar& chr) noexcept
    {
        if (!_scene)
        {
            return;
        }
        Rml::String str = chr.toString();
        for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
        {
            canvas.getImpl().processTextInput(str);
        }
    }

    void RmluiSceneComponentImpl::onMouseActive(bool active) noexcept
    {
        if (active || !_scene)
        {
            return;
        }
        for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
        {
            canvas.getImpl().processMouseLeave();
        }
    }

    void RmluiSceneComponentImpl::onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept
    {
        if (!_app || !_scene)
        {
            return;
        }
        auto& win = _app->getWindow();

        for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
        {
            if (!canvas.isInputEnabled())
            {
                continue;
            }
            auto mode = canvas.getMousePositionMode();
            if (mode == RmluiCanvasMousePositionMode::Disabled)
            {
                continue;
            }
            auto cam = canvas.getCurrentCamera();
            if (!cam)
            {
                continue;
            }
            auto vp = cam->getCombinedViewport();

            if (mode == RmluiCanvasMousePositionMode::Relative)
            {
                auto screenDelta = win.windowToScreenDelta(delta);
                auto vpDelta = vp.screenToViewportDelta(screenDelta);
                canvas.getImpl().applyViewportMousePositionDelta(vpDelta);
            }
            else
            {
                auto screenPos = win.windowToScreenPoint(absolute);
                auto vpPos = vp.screenToViewportPoint(screenPos);
                canvas.getImpl().setViewportMousePosition(vpPos);
            }
        }
    }

    void RmluiSceneComponentImpl::onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept
    {
        if (!_scene)
        {
            return;
        }
        auto rmlDelta = RmluiUtils::convert<float>(delta) * -1;
        auto state = getKeyModifierState();
        for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
        {
            canvas.getImpl().processMouseWheel(rmlDelta, state);
        }
    }

    void RmluiSceneComponentImpl::onMouseButton(MouseButton button, bool down) noexcept
    {
        if (!_scene)
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
        default:
            break;
        }
        auto state = getKeyModifierState();
        for (auto [entity, canvas] : _scene->getComponents<RmluiCanvas>().each())
        {
            canvas.getImpl().processMouseButton(i, state, down);
        }
    }

    RmluiSceneComponent::RmluiSceneComponent() noexcept
        : _impl(std::make_unique<RmluiSceneComponentImpl>())
    {
    }

    RmluiSceneComponent::~RmluiSceneComponent() noexcept
    {
        // left empty to get the forward declaration of the impl working
    }

    void RmluiSceneComponent::init(Scene& scene, App& app) noexcept
    {
        _impl->init(scene, app);
    }

    void RmluiSceneComponent::shutdown() noexcept
    {
        _impl->shutdown();
    }

    bgfx::ViewId RmluiSceneComponent::renderReset(bgfx::ViewId viewId) noexcept
    {
        return _impl->renderReset(viewId);
    }

    void RmluiSceneComponent::update(float deltaTime) noexcept
    {
        _impl->update(deltaTime);
    }

    void RmluiRendererImpl::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
    }

    void RmluiRendererImpl::shutdown() noexcept
    {
        _cam.reset();
        _scene.reset();
    }

    void RmluiRendererImpl::render() noexcept
    {
        auto encoder = bgfx::begin();
        for (auto entity : _cam->getEntities<RmluiCanvas>())
        {
            auto& canvas = _scene->getComponent<RmluiCanvas>(entity).value();
            canvas.getImpl().render(*encoder);
        }
        bgfx::end(encoder);
    }

    bgfx::ViewId RmluiRendererImpl::renderReset(bgfx::ViewId viewId) noexcept
    {
        for (auto entity : _cam->getEntities<RmluiCanvas>())
        {
            auto& canvas = _scene->getComponent<RmluiCanvas>(entity).value();
            viewId = canvas.getImpl().renderReset(viewId);
        }
        return viewId;
    }

    void RmluiRendererImpl::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        for (auto entity : _cam->getEntities<RmluiCanvas>())
        {
            auto& canvas = _scene->getComponent<RmluiCanvas>(entity).value();
            canvas.getImpl().beforeRenderView(viewId, encoder);
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

    void RmluiRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _impl->init(cam, scene, app);
    }

    void RmluiRenderer::shutdown() noexcept
    {
        _impl->shutdown();
    }

    void RmluiRenderer::render() noexcept
    {
        _impl->render();
    }

    bgfx::ViewId RmluiRenderer::renderReset(bgfx::ViewId viewId) noexcept
    {
        return _impl->renderReset(viewId);
    }

    void RmluiRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        _impl->beforeRenderView(viewId, encoder);
    }

}