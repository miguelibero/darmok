#pragma once

#include <darmok/export.h>
#include <darmok/camera.hpp>

namespace darmok
{
    class DARMOK_EXPORT DeferredRenderer final : public ICameraRenderer
    {
    protected:
        bgfx::ViewId render(bgfx::ViewId viewId) const override;
    };
}