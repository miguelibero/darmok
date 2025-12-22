#pragma once

#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/color.hpp>
#include <darmok/mesh.hpp>
#include <darmok/protobuf/camera.pb.h>

#include <memory>
#include <vector>

namespace darmok
{
    class Texture;
    class Program;

    class DARMOK_EXPORT SkyboxRenderer final : public ITypeCameraComponent<SkyboxRenderer>
    {
    public:
        using Definition = protobuf::SkyboxRenderer;

        SkyboxRenderer(const std::shared_ptr<Texture>& texture) noexcept;
        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;
    private:
        OptionalRef<Camera> _cam;
        std::unique_ptr<Mesh> _mesh;
        UniformHandle _texUniform;
        std::shared_ptr<Texture> _texture;
        std::unique_ptr<Program> _program;
    };


    class GridRenderer final : public ITypeCameraComponent<GridRenderer>
    {
    public:
        using Definition = protobuf::GridRenderer;

        static Definition createDefinition() noexcept;

        GridRenderer(const Definition& def = createDefinition()) noexcept;
        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> load(const Definition& def) noexcept;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;
    private:
        Definition _def;
        OptionalRef<Camera> _cam;
        std::unique_ptr<Program> _program;
        std::unique_ptr<Mesh> _mesh;
        UniformHandle _color1Uniform;
        UniformHandle _color2Uniform;
        UniformHandle _dataUniform;
    };

    class DARMOK_EXPORT EnvironmentMap final
    {
    public:
    };
}