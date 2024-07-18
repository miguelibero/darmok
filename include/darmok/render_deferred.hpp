#pragma once

#include <darmok/export.h>
#include <darmok/render.hpp>

namespace darmok
{
    class Camera;

    class DARMOK_EXPORT DeferredRenderer final : public IRenderer
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;

        void addComponent(std::unique_ptr<IRenderComponent>&& comp) noexcept override;
    private:
        OptionalRef<Camera> _cam;
        std::vector<std::unique_ptr<IRenderComponent>> _components;

        void beforeRenderView(bgfx::ViewId viewId);
        void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder);
    };
}