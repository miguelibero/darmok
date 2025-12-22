#pragma once

#include <memory>
#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/protobuf/camera.pb.h>

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

        using Definition = protobuf::ForwardRenderer;

        static Definition createDefinition() noexcept;

        ForwardRenderer() noexcept;
        ~ForwardRenderer() noexcept;
        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> load(const Definition& def) noexcept;
        expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
        expected<void, std::string> render() noexcept override;
        expected<void, std::string> shutdown() noexcept override;

    private:
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        OptionalRef<MaterialAppComponent> _materials;
        std::optional<bgfx::ViewId> _viewId;
    };
}