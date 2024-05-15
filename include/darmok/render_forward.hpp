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
        DLLEXPORT ForwardRenderer(const std::shared_ptr<Program>& program) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
    protected:
        bgfx::ViewId render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const override;
    private:
        const static std::string _name;
        std::shared_ptr<Program> _program;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
    };
}