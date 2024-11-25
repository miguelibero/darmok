#pragma once

#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/render_debug.hpp>
#include <darmok/color_fwd.hpp>

#include <memory>
#include <vector>

namespace darmok
{
    class Texture;
    class Program;
    class IMesh;

    class DARMOK_EXPORT SkyboxRenderer final : public ITypeCameraComponent<SkyboxRenderer>
    {
    public:
        SkyboxRenderer(const std::shared_ptr<Texture>& texture) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) override;
    private:
        OptionalRef<Camera> _cam;
        std::unique_ptr<IMesh> _mesh;
        bgfx::UniformHandle _texUniform;
        std::shared_ptr<Texture> _texture;
        std::unique_ptr<Program> _program;
    };

    class GridRenderer final : public ITypeCameraComponent<GridRenderer>
    {
    public:

        struct GridConfig
        {
            glm::vec2 separation;
            Color color;
        };

        using Config = std::vector<GridConfig>;

        static const Config defaultConfig;

        GridRenderer(const Config& config = defaultConfig) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) override;
        void onCameraTransformChanged() override;
    private:
        Config _config;
        OptionalRef<Camera> _cam;
        std::unique_ptr<IMesh> _mesh;
        DebugRenderer _debugRender;

        void updateMesh() noexcept;
    };

    class DARMOK_EXPORT EnvironmentMap final
    {
    public:
    };
}