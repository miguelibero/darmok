#pragma once

#include <darmok/camera.hpp>
#include <darmok/program_def.hpp>

namespace darmok
{
    class Program;

    class ForwardRenderer final : public ICameraRenderer
    {
    public:
        ForwardRenderer(const std::shared_ptr<Program>& program, const ProgramDefinition& prodDef) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
    protected:
        bgfx::ViewId render(bgfx::Encoder& encoder, bgfx::ViewId viewId) override;
    private:
        std::shared_ptr<Program> _program;
        ProgramDefinition _progDef;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
    };
}