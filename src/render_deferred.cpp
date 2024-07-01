#pragma once

#include <darmok/render_deferred.hpp>

namespace darmok
{
    bgfx::ViewId DeferredRenderer::render(bgfx::ViewId viewId) const
    {
        return viewId;
    }
}