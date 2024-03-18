#pragma once

#include <darmok/scene.hpp>

namespace darmok
{
    class DeferredRenderer final : public CameraSceneRenderer
    {
    protected:
        bgfx::ViewId render(const Camera& cam, bgfx::Encoder& encoder, bgfx::ViewId viewId) override;
    };
}