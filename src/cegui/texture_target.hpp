#pragma once

#include <CEGUI/TextureTarget.h>
#include "render_target.hpp"
#include <bgfx/bgfx.h>

namespace darmok
{
    class CeguiTexture;

    class CeguiTextureTarget final : public CEGUI::TextureTarget, public CeguiRenderTarget
    {
    public:
        CeguiTextureTarget(CeguiRenderer& renderer, bool addStencilBuffer) noexcept;
        ~CeguiTextureTarget() noexcept;
        void clear() noexcept override;
        CEGUI::Texture& getTexture() const noexcept override;
        void declareRenderSize(const CEGUI::Sizef& sz) noexcept override;
        void activate() noexcept override;
        void deactivate() noexcept override;
        bool isImageryCache() const noexcept override;
        void updateMatrix() const noexcept override;
        CEGUI::Renderer& getOwner() noexcept override;
    private:
        CeguiTexture& _texture;
        bgfx::FrameBufferHandle _frameBuffer;
        static uint32_t s_textureNumber;

        static CEGUI::String generateTextureName() noexcept;
        void destroyFramebuffer() noexcept;
    };
}