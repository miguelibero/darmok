#include "texture_target.hpp"

namespace darmok
{
    CeguiTextureTarget::CeguiTextureTarget(CEGUI::Renderer& renderer, bool addStencilBuffer, bx::AllocatorI* alloc) noexcept
        : _renderer(renderer)
        , _texture(alloc)
        , TextureTarget(addStencilBuffer)
    {
    }

    void CeguiTextureTarget::clear()
    {
    }

    CEGUI::Texture& CeguiTextureTarget::getTexture() const
    {
        return _texture;
    }

    void CeguiTextureTarget::declareRenderSize(const CEGUI::Sizef& sz)
    {
    }

    bool CeguiTextureTarget::isImageryCache() const
    {
        return false;
    }

    void CeguiTextureTarget::updateMatrix() const
    {

    }

    CEGUI::Renderer& CeguiTextureTarget::getOwner()
    {
        return _renderer;
    }
}