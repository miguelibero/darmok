#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

namespace CEGUI
{
    class RenderTarget;
}

namespace darmok
{
    class CeguiRenderer;

    class CeguiTargetTransform final
    {
    public:
        CeguiTargetTransform(CEGUI::RenderTarget& target, CeguiRenderer& renderer) noexcept;
        void activate(bool matrixValid) noexcept;
        void deactivate() noexcept;
        float updateMatrix(float fovyHalfTan) const noexcept;
        CeguiRenderer& getRenderer() noexcept;
        const CeguiRenderer& getRenderer() const noexcept;
        glm::mat4 getMatrix() const noexcept;
    private:
        CEGUI::RenderTarget& _target;
        CeguiRenderer& _renderer;
        mutable glm::mat4 _view;
        mutable glm::mat4 _proj;
    };
}