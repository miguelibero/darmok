#pragma once

#include <memory>
#include <darmok/export.h>
#include <darmok/render.hpp>
#include <darmok/render_graph.hpp>

namespace darmok
{
    class Program;
    class Scene;
    class App;
    class Material;

    class DARMOK_EXPORT ForwardRenderer final : public IRenderer, public IRenderPass
    {
    public:
        ForwardRenderer() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(RenderGraphResources& res) noexcept override;

        void addComponent(std::unique_ptr<IRenderComponent>&& comp) noexcept override;
    private:
        const static std::string _name;
        bgfx::ViewId _viewId;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        std::vector<std::unique_ptr<IRenderComponent>> _components;

        void beforeRenderView(bgfx::ViewId viewId);
        void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder);
    };
}