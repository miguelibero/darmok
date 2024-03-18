#pragma once

#include <darmok/scene.hpp>

namespace darmok
{
    class LightRenderUpdater;

    class ForwardRenderer final : public CameraSceneRenderer
    {
    public:
        ForwardRenderer(OptionalRef<LightRenderUpdater> lights = nullptr);
    protected:
        bgfx::ViewId render(const Camera& cam, bgfx::Encoder& encoder, bgfx::ViewId viewId) override;
    private:
        OptionalRef<LightRenderUpdater> _lights;
    };
}