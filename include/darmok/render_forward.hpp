#pragma once

#include <darmok/camera.hpp>

namespace darmok
{
    class Program;
    class Scene;
    class App;

    class ForwardRenderer final : public ICameraRenderer
    {
    public:
        DLLEXPORT void init(Camera& cam, Scene& scene, App& app) noexcept override;
        DLLEXPORT void shutdown() noexcept override;
        DLLEXPORT bgfx::ViewId render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const override;
    private:
        const static std::string _name;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
    };
}