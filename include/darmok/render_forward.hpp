#pragma once

#include <memory>
#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/material_fwd.hpp>

namespace darmok
{
    class Program;
    class Scene;
    class App;
    class MaterialAppComponent;
    class EntityView;

    class DARMOK_EXPORT ForwardRenderer final : public ITypeCameraComponent<ForwardRenderer>
    {
    public:
        ForwardRenderer() noexcept;
        ~ForwardRenderer() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        bgfx::ViewId renderReset(bgfx::ViewId viewId) noexcept override;
        void render() noexcept override;
        void shutdown() noexcept override;
    private:
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        OptionalRef<MaterialAppComponent> _materials;
        std::optional<bgfx::ViewId> _viewId;
    };
}