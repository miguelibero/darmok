#pragma once

#include <darmok/scene.hpp>
#include <darmok/program_def.hpp>

namespace darmok
{
    class LightRenderUpdater;
    class Program;

    class ForwardRenderer final : public CameraSceneRenderer
    {
    public:
        ForwardRenderer(OptionalRef<LightRenderUpdater> lights = nullptr);
        void init(Scene& scene, App& app) noexcept override;
        const ProgramDefinition& getProgramDefinition() const noexcept;
    protected:
        bgfx::ViewId render(const Camera& cam, bgfx::Encoder& encoder, bgfx::ViewId viewId) override;
    private:
        OptionalRef<LightRenderUpdater> _lights;
        std::shared_ptr<Program> _program;
        ProgramDefinition _progDef;
    };
}