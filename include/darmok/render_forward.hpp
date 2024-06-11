#pragma once

#include <darmok/export.h>
#include <darmok/camera.hpp>

namespace darmok
{
    class Program;
    class Scene;
    class App;

    class DARMOK_EXPORT ForwardRenderer final : public ICameraRenderer
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        bgfx::ViewId render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const override;
    private:
        const static std::string _name;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
    };
}