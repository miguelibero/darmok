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
#include <darmok/asset.hpp>
#include <darmok/program.hpp>

namespace darmok
{
    const CEGUI::String CeguiRenderer::_rendererId("darmok");

	CeguiRenderer::CeguiRenderer(App& app) noexcept
		: _app(app)
        , _defaultRenderTarget(*this)
        , _shaderWrapper(app.getAssets().getStandardProgramLoader()(StandardProgramType::Gui))
	{
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
        // Solid & Textured use the same StandardProgramType::Gui
        CEGUI::RefCounted<CEGUI::RenderMaterial> material(new CEGUI::RenderMaterial(&_shaderWrapper));
        return material;
    }

    CEGUI::GeometryBuffer& CeguiRenderer::createGeometryBufferTextured(CEGUI::RefCounted<CEGUI::RenderMaterial> renderMaterial) 
    {
        auto buffer = new CeguiGeometryBuffer(renderMaterial);
        return *buffer;
    }

    CEGUI::GeometryBuffer& CeguiRenderer::createGeometryBufferColoured(CEGUI::RefCounted<CEGUI::RenderMaterial> renderMaterial) 
    {
        auto buffer = new CeguiGeometryBuffer(renderMaterial);
        return *buffer;
    }

    bx::AllocatorI* CeguiRenderer::getAllocator() const noexcept
    {
        return _app.getAssets().getAllocator();
    }

    CEGUI::TextureTarget* CeguiRenderer::createTextureTarget(bool addStencilBuffer)
    {
        auto target = std::make_unique<CeguiTextureTarget>(*this, addStencilBuffer, getAllocator());
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

    CeguiTexture& CeguiRenderer::doCreateTexture(const CEGUI::String& name)
    {
        if (_textures.find(name) != _textures.end())
        {
            throw CEGUI::AlreadyExistsException(
                "A texture named '" + name + "' already exists.");
        }
        auto texture = std::make_unique<CeguiTexture>(getAllocator(), name);
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
        _textures.erase(itr);
        auto logger = CEGUI::Logger::getSingletonPtr();
        if (logger)
        {
            logger->logEvent("[DarmokRenderer] Destroyed texture: " + name);
        }
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
}