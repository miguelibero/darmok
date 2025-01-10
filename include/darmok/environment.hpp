#pragma once

#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/color.hpp>
#include <darmok/mesh.hpp>

#include <memory>
#include <vector>

namespace darmok
{
    class Texture;
    class Program;

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


    struct GridConfig final
    {
        float separation = 1.0F;
        Color color = Colors::white();
        float width = 0.01;
    };

    struct GridRendererConfig final
    {
        Color xAxisColor = Colors::red();
        Color yAxisColor = Colors::green();
        Color zAxisColor = Colors::blue();
        std::array<GridConfig, 2> grids = {
            GridConfig{ 1.F, Colors::fromNumber(0x505050FF) },
            GridConfig{ 10.F, Colors::fromNumber(0x707070FF) }
        };
    };

    class GridRenderer final : public ITypeCameraComponent<GridRenderer>
    {
    public:
        using Config = GridRendererConfig;

        GridRenderer(const Config& config = {}) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) override;
    private:
        Config _config;
        OptionalRef<Camera> _cam;
        std::unique_ptr<Program> _program;
        std::unique_ptr<IMesh> _mesh;
        bgfx::UniformHandle _color1Uniform;
        bgfx::UniformHandle _color2Uniform;
        bgfx::UniformHandle _dataUniform;
    };

    class DARMOK_EXPORT EnvironmentMap final
    {
    public:
    };
}