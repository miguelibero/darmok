#pragma once

#include <memory>
#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/material_fwd.hpp>

namespace darmok
{
    class Program;
    class Scene;
    class App;
    class MaterialAppComponent;

    class DARMOK_EXPORT ForwardRenderer final : public ITypeCameraComponent<ForwardRenderer>, public IRenderPass
    {
    public:
        ForwardRenderer() noexcept;
        ~ForwardRenderer() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void renderReset() noexcept override;
        void shutdown() noexcept override;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) override;
        void renderPassExecute(IRenderGraphContext& context) noexcept override;

    private:
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        OptionalRef<MaterialAppComponent> _materials;
        
        void renderEntities(IRenderGraphContext& context, const EntityRuntimeView& entities, OpacityType opacity) noexcept;
    };
}