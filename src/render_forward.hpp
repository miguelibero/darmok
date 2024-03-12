#pragma once

#include <darmok/scene.hpp>

namespace darmok
{
    class ForwardRenderer final : public ISceneRenderer
    {
        void render(EntityRuntimeView& entities, bgfx::Encoder& encoder, bgfx::ViewId viewId) override;
    };
}