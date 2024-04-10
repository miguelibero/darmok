#pragma once

#include <darmok/camera.hpp>

namespace darmok
{
    class DeferredRenderer final : public ICameraRenderer
    {
    protected:
        bgfx::ViewId render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const override;
    };
}