#include "texture_target.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include <cegui/PropertyHelper.h>
#include <darmok/texture.hpp>
#include <darmok/data.hpp>

namespace darmok
{
    uint32_t CeguiTextureTarget::s_textureNumber = 0;

    CeguiTextureTarget::CeguiTextureTarget(CeguiRenderer& renderer, bool addStencilBuffer) noexcept
        : CeguiRenderTarget(renderer)
        , TextureTarget(addStencilBuffer)
        , _texture(renderer.createRenderTexture(generateTextureName()))
        , _frameBuffer{ bgfx::kInvalidHandle }
    {
    }

    CEGUI::String CeguiTextureTarget::generateTextureName() noexcept
    {
        CEGUI::String tmp("_darmok_tt_tex_");
        tmp.append(CEGUI::PropertyHelper<std::uint32_t>::toString(s_textureNumber++));
        return tmp;
    }

    CeguiTextureTarget::~CeguiTextureTarget() noexcept
    {
        _renderer.destroyTexture(_texture);
        destroyFramebuffer();
    }

    void CeguiTextureTarget::destroyFramebuffer() noexcept
    {
        if (isValid(_frameBuffer))
        {
            bgfx::destroy(_frameBuffer);
        }
    }

    void CeguiTextureTarget::clear() noexcept
    {
        auto tex = _texture.getDarmokTexture();
        if (!tex)
        {
            return;
        }
        auto size = tex->getStorageSize();
        std::vector<uint8_t> mem(size);
        tex->update(DataView(mem));
    }

    CEGUI::Texture& CeguiTextureTarget::getTexture() const noexcept
    {
        return _texture;
    }

    void CeguiTextureTarget::declareRenderSize(const CEGUI::Sizef& sz) noexcept
    {
        if (_texture.getSize() == sz)
        {
            return;
        }
        destroyFramebuffer();
        _texture.loadFromSize(sz);
        auto handle = _texture.getBgfxHandle();
        _frameBuffer = bgfx::createFrameBuffer(1, &handle);
        auto name = CeguiUtils::convert(_texture.getName());
        bgfx::setName(_frameBuffer, name.data(), name.size());
    }

    void CeguiTextureTarget::activate() noexcept
    {
        CeguiRenderTarget::activate();
        bgfx::setViewFrameBuffer(_renderer.getViewId(), _frameBuffer);
    }

    void CeguiTextureTarget::deactivate() noexcept
    {
        CeguiRenderTarget::deactivate();
        bgfx::setViewFrameBuffer(_renderer.getViewId(), { bgfx::kInvalidHandle });
    }

    bool CeguiTextureTarget::isImageryCache() const noexcept
    {
        return CeguiRenderTarget::isImageryCache();
    }

    void CeguiTextureTarget::updateMatrix() const noexcept
    {
        CeguiRenderTarget::updateMatrix();
    }

    CEGUI::Renderer& CeguiTextureTarget::getOwner() noexcept
    {
        return CeguiRenderTarget::getOwner();
    }
}