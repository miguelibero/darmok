#pragma once

#include <darmok/export.h>
#include <memory>
#include <darmok/render_scene.hpp>


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

    class DARMOK_EXPORT EnvironmentMap final
    {
    public:
    };
}