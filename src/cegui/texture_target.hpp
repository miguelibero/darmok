#pragma once

#include <CEGUI/TextureTarget.h>
#include "render_target.hpp"
#include "target_transform.hpp"
#include <bgfx/bgfx.h>
#include <optional>

namespace darmok
{
    class CeguiTexture;

    class CeguiTextureTarget final : public CEGUI::TextureTarget
    {
    public:
        CeguiTextureTarget(CeguiRenderer& renderer, uint16_t num, bool addStencilBuffer) noexcept;
        ~CeguiTextureTarget() noexcept;
        void clear() noexcept override;
        CEGUI::Texture& getTexture() const noexcept override;
        void declareRenderSize(const CEGUI::Sizef& sz) override;
        void activate() noexcept override;
        void deactivate() noexcept override;
        bool isImageryCache() const noexcept override;
        void updateMatrix() const noexcept override;
        CEGUI::Renderer& getOwner() noexcept override;
        uint16_t getNumber() const noexcept;
    private:
        CeguiTargetTransform _trans;
        uint16_t _num;
        CeguiTexture& _texture;
        bgfx::FrameBufferHandle _frameBuffer;

        static CEGUI::String generateTextureName(uint16_t num) noexcept;
        void destroyFramebuffer() noexcept;
    };
}