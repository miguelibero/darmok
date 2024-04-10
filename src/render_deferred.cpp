#pragma once

#include <darmok/render_deferred.hpp>

namespace darmok
{
    bgfx::ViewId DeferredRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const
    {
        return viewId;
    }
}