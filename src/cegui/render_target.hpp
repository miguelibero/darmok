#pragma once

#include <CEGUI/RenderTarget.h>

namespace darmok
{
    class CeguiRenderTarget final : public CEGUI::RenderTarget
    {
    public:
        CeguiRenderTarget(CEGUI::Renderer& renderer) noexcept;
        bool isImageryCache() const override;
        void updateMatrix() const override;
        CEGUI::Renderer& getOwner() override;
    private:
        CEGUI::Renderer& _renderer;
    };
}