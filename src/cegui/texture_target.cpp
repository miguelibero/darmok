#include "texture_target.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include <array>
#include <cegui/PropertyHelper.h>
#include <cegui/Sizef.h>
#include <darmok/texture.hpp>
#include <darmok/data.hpp>

namespace darmok
{
    CeguiTextureTarget::CeguiTextureTarget(CeguiRenderer& renderer, uint16_t num, bool addStencilBuffer) noexcept
        : TextureTarget(addStencilBuffer)
        , _trans(*this, renderer)
        , _num(num)
        , _texture(renderer.createTexture(generateTextureName(num), BGFX_TEXTURE_RT))
        , _frameBuffer{ bgfx::kInvalidHandle }
    {
    }

    CEGUI::String CeguiTextureTarget::generateTextureName(uint16_t num) noexcept
    {
        CEGUI::String tmp("_darmok_tt_tex_");
        tmp.append(CEGUI::PropertyHelper<std::uint32_t>::toString(num));
        return tmp;
    }

    CeguiTextureTarget::~CeguiTextureTarget() noexcept
    {
        _trans.getRenderer().destroyTexture(_texture);
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
        std::vector<uint8_t> data(tex->getStorageSize());
        _texture.blitFromMemory(&data.front(), CEGUI::Rectf({ 0, 0 }, _texture.getSize()));
    }

    CEGUI::Texture& CeguiTextureTarget::getTexture() const noexcept
    {
        return _texture;
    }

    void CeguiTextureTarget::declareRenderSize(const CEGUI::Sizef& sz)
    {
        auto& size = _texture.getSize();
        if (sz.d_width <= size.d_width && sz.d_height <= size.d_height)
        {
            return;
        }
        setArea(CEGUI::Rectf(d_area.getPosition(), sz));
        destroyFramebuffer();
        if (sz.d_width == 0 && sz.d_height == 0)
        {
            return;
        }
        _texture.loadFromSize(sz);
        auto name = CeguiUtils::convert(_texture.getName());
        auto handle = _texture.getBgfxHandle();
        _frameBuffer = bgfx::createFrameBuffer(1, &handle);
        bgfx::setName(_frameBuffer, name.data(), name.size());
    }

    void CeguiTextureTarget::activate() noexcept
    {
        TextureTarget::activate();
        _trans.activate(d_matrixValid);
        auto viewId = _trans.getRenderer().getViewId();
        bgfx::setViewFrameBuffer(viewId, _frameBuffer);
    }

    void CeguiTextureTarget::deactivate() noexcept
    {
        TextureTarget::deactivate();
        _trans.deactivate();
    }

    bool CeguiTextureTarget::isImageryCache() const noexcept
    {
        return true;
    }

    void CeguiTextureTarget::updateMatrix() const noexcept
    {
        d_viewDistance = _trans.updateMatrix(d_fovY_halftan);
        d_matrix = _trans.getMatrix();
        d_matrixValid = true;
    }

    CEGUI::Renderer& CeguiTextureTarget::getOwner() noexcept
    {
        return _trans.getRenderer();
    }

    uint16_t CeguiTextureTarget::getNumber() const noexcept
    {
        return _num;
    }
}