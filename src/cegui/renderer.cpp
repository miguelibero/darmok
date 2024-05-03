#include "renderer.hpp"
#include "texture.hpp"
#include "texture_target.hpp"
#include "resource.hpp"
#include "geometry.hpp"
#include <CEGUI/Logger.h>
#include <CEGUI/System.h>
#include <CEGUI/Exceptions.h>
#include <CEGUI/RenderMaterial.h>
#include <darmok/app.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/color.hpp>
#include <darmok/image.hpp>
#include <darmok/texture.hpp>

#include "embedded_shader.hpp"
#include "generated/shaders/cegui_vertex.h"
#include "generated/shaders/cegui_fragment.h"
#include "generated/shaders/cegui_vertex_layout.h"
#include "generated/shaders/cegui_solid_vertex.h"
#include "generated/shaders/cegui_solid_fragment.h"
#include "generated/shaders/cegui_solid_vertex_layout.h"

namespace darmok
{
    const CEGUI::String CeguiRenderer::_rendererId("darmok");
    const CEGUI::String CeguiRenderer::_solidWhiteTextureName = "solidWhite";

    const bgfx::EmbeddedShader CeguiRenderer::_embeddedShaders[] =
    {
        BGFX_EMBEDDED_SHADER(cegui_vertex),
        BGFX_EMBEDDED_SHADER(cegui_fragment),
        BGFX_EMBEDDED_SHADER(cegui_solid_vertex),
        BGFX_EMBEDDED_SHADER(cegui_solid_fragment),
        BGFX_EMBEDDED_SHADER_END()
    };

	CeguiRenderer::CeguiRenderer(App& app) noexcept
		: _app(app)
        , _defaultRenderTarget(*this)
        , _texturedProgram(std::make_shared<Program>("cegui", _embeddedShaders, cegui_vertex_layout))
        , _solidProgram(std::make_shared<Program>("cegui_solid", _embeddedShaders, cegui_solid_vertex_layout))
        , _texturedShaderWrapper(_texturedProgram)
        , _solidShaderWrapper(_solidProgram)
        , _viewId(0)
        , _textureTargetNum(0)
	{
        auto& winSize = app.getWindow().getPixelSize();
        setDisplaySize(CEGUI::Sizef(winSize.x, winSize.y));
	}

    CeguiRenderer::~CeguiRenderer() noexcept
    {
        // intentionally let blank for forwarded std::unique_ptr destructors
    }

    CEGUI::RenderTarget& CeguiRenderer::getDefaultRenderTarget() 
    {
        return _defaultRenderTarget;
    }

    CEGUI::RefCounted<CEGUI::RenderMaterial> CeguiRenderer::createRenderMaterial(const CEGUI::DefaultShaderType shaderType) const 
    {
        auto shaderWrapper = &_texturedShaderWrapper;
        if (shaderType == CEGUI::DefaultShaderType::Solid)
        {
            shaderWrapper = &_solidShaderWrapper;
        }
        return CEGUI::RefCounted<CEGUI::RenderMaterial>(new CEGUI::RenderMaterial(shaderWrapper));
    }

    CEGUI::GeometryBuffer& CeguiRenderer::createGeometryBufferTextured(CEGUI::RefCounted<CEGUI::RenderMaterial> renderMaterial) 
    {
        auto buffer = new CeguiGeometryBuffer(renderMaterial, *this);
        addGeometryBuffer(*buffer);
        return *buffer;
    }

    CEGUI::GeometryBuffer& CeguiRenderer::createGeometryBufferColoured(CEGUI::RefCounted<CEGUI::RenderMaterial> renderMaterial) 
    {
        auto buffer = new CeguiGeometryBuffer(renderMaterial, *this);
        addGeometryBuffer(*buffer);
        return *buffer;
    }

    bx::AllocatorI* CeguiRenderer::getAllocator() const noexcept
    {
        return _app.getAssets().getAllocator();
    }

    CEGUI::TextureTarget* CeguiRenderer::createTextureTarget(bool addStencilBuffer)
    {
        auto target = std::make_unique<CeguiTextureTarget>(*this, _textureTargetNum++, addStencilBuffer);
        auto ptr = target.get();
        _textureTargets.push_back(std::move(target));
        return ptr;
    }

    void CeguiRenderer::destroyTextureTarget(CEGUI::TextureTarget* target) 
    {
        auto itr = std::find_if(_textureTargets.begin(), _textureTargets.end(),
            [target](const auto& tgt) { return tgt.get() == target; });
        if (itr != _textureTargets.end())
        {
            _textureTargets.erase(itr);
        }
        else
        {
            delete target;
        }
    }

    void CeguiRenderer::destroyAllTextureTargets()
    {
        _textureTargets.clear();
    }

    CeguiTexture& CeguiRenderer::doCreateTexture(const CEGUI::String& name, uint64_t flags)
    {
        if (_textures.find(name) != _textures.end())
        {
            throw CEGUI::AlreadyExistsException(
                "A texture named '" + name + "' already exists.");
        }
        return doAddTexture(std::make_unique<CeguiTexture>(getAllocator(), name, flags));
    }

    CeguiTexture& CeguiRenderer::doAddTexture(std::unique_ptr<CeguiTexture>&& texture)
    {
        auto& name = texture->getName();
        auto logger = CEGUI::Logger::getSingletonPtr();
        if (logger)
        {
            logger->logEvent("[DarmokRenderer] Created texture: " + name);
        }
        auto ptr = texture.get();
        _textures[name] = std::move(texture);
        return *ptr;
    }

    CEGUI::Texture& CeguiRenderer::createTexture(const CEGUI::String& name) 
    {
        return doCreateTexture(name);
    }

    CeguiTexture& CeguiRenderer::createTexture(const CEGUI::String& name, uint64_t flags)
    {
        return doCreateTexture(name, flags);
    }

    CEGUI::Texture& CeguiRenderer::createTexture(const CEGUI::String& name,
        const CEGUI::String& filename,
        const CEGUI::String& resourceGroup) 
    {
        auto& texture = doCreateTexture(name);
        texture.loadFromFile(filename, resourceGroup);
        return texture;
    }

    CEGUI::Texture& CeguiRenderer::createTexture(const CEGUI::String& name, const CEGUI::Sizef& size) 
    {
        auto& texture = doCreateTexture(name);
        texture.loadFromSize(size);
        return texture;
    }

    void CeguiRenderer::destroyTexture(CEGUI::Texture& texture) 
    {
        destroyTexture(texture.getName());
    }

    void CeguiRenderer::destroyTexture(const CEGUI::String& name) 
    {
        auto itr = _textures.find(name);
        if (itr == _textures.end())
        {
            return;
        }
        auto logger = CEGUI::Logger::getSingletonPtr();
        if (logger)
        {
            logger->logEvent("[DarmokRenderer] Destroyed texture: " + name);
        }
        _textures.erase(itr);
    }

    void CeguiRenderer::destroyAllTextures() 
    {
        _textures.clear();
    }

    CEGUI::Texture& CeguiRenderer::getTexture(const CEGUI::String& name) const 
    {
        auto itr = _textures.find(name);
        if (itr == _textures.end())
        {
            throw CEGUI::UnknownObjectException(
                "No texture named '" + name + "' is available.");
        }
        return *itr->second;
    }

    bool CeguiRenderer::isTextureDefined(const CEGUI::String& name) const 
    {
        return _textures.find(name) != _textures.end();
    }

    void CeguiRenderer::beginRendering() 
    {
    }

    void CeguiRenderer::endRendering() 
    {
    }

    void CeguiRenderer::setDisplaySize(const CEGUI::Sizef& sz) 
    {
        _displaySize = sz;
        _defaultRenderTarget.setArea(CEGUI::Rectf(0, 0, sz.d_width, sz.d_height));
    }

    const CEGUI::Sizef& CeguiRenderer::getDisplaySize() const 
    {
        return _displaySize;
    }

    unsigned int CeguiRenderer::getMaxTextureSize() const 
    {
        auto caps = bgfx::getCaps();
        return caps->limits.maxTextureSize;
    }

    const CEGUI::String& CeguiRenderer::getIdentifierString() const 
    {
        return _rendererId;
    }

    bool CeguiRenderer::isTexCoordSystemFlipped() const 
    {
        return false;
    }

    bgfx::ViewId CeguiRenderer::getViewId() const noexcept
    {
        return _viewId;
    }

    void CeguiRenderer::setViewId(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
    }
}