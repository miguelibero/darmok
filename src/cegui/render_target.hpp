#pragma once

#include <CEGUI/RenderTarget.h>

namespace darmok
{
    class CeguiRenderer;

    class CeguiRenderTarget : public CEGUI::RenderTarget
    {
    public:
        CeguiRenderTarget(CeguiRenderer& renderer) noexcept;
        bool isImageryCache() const noexcept override;
        void activate() noexcept override;
        void deactivate() noexcept override;
        void updateMatrix() const noexcept override;
        CEGUI::Renderer& getOwner() noexcept override;
    protected:
        CeguiRenderer& _renderer;
    };
}