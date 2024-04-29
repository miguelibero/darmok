#pragma once

#include <CEGUI/TextureTarget.h>
#include "texture.hpp"

namespace darmok
{
    class CeguiTextureTarget final : public CEGUI::TextureTarget
    {
    public:
        CeguiTextureTarget(CEGUI::Renderer& renderer, bool addStencilBuffer, bx::AllocatorI* alloc) noexcept;
        void clear() override;
        CEGUI::Texture& getTexture() const override;
        void declareRenderSize(const CEGUI::Sizef& sz) override;
        bool isImageryCache() const override;
        void updateMatrix() const override;
        CEGUI::Renderer& getOwner() override;
    private:
        CEGUI::Renderer& _renderer;
        mutable CeguiTexture _texture;
    };
}