#pragma once

#include <memory>
#include <darmok/export.h>
#include <darmok/camera.hpp>

namespace darmok
{
    class Program;
    class Scene;
    class App;
    class Material;

    class DARMOK_EXPORT ForwardRenderer final : public ICameraRenderer
    {
    public:
        ForwardRenderer() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        ForwardRenderer& setInvalidMaterial(const std::shared_ptr<Material>& mat) noexcept;
        void shutdown() noexcept override;
        bgfx::ViewId render(bgfx::ViewId viewId) const override;
    private:
        const static std::string _name;
        std::shared_ptr<Material> _invalidMaterial;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
    };
}