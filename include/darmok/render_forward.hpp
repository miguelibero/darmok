#pragma once

#include <darmok/camera.hpp>

namespace darmok
{
    class Program;
    class ILightingComponent;

    class ForwardRenderer final : public ICameraRenderer
    {
    public:
        ForwardRenderer(const std::shared_ptr<Program>& program, const OptionalRef<ILightingComponent>& lighting = nullptr) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
    protected:
        bgfx::ViewId render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const override;
    private:
        std::shared_ptr<Program> _program;
        OptionalRef<ILightingComponent> _lighting;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
    };
}