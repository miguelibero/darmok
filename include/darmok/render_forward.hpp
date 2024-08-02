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
        void update(float deltaTime) override;
        void renderReset() noexcept override;
        void shutdown() noexcept override;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(IRenderGraphContext& context) noexcept override;

        void addComponent(std::unique_ptr<IRenderComponent>&& comp) noexcept;

        template<typename T, typename... A>
        T& addComponent(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addComponent(std::move(ptr));
            return ref;
        }

    private:
        bgfx::ViewId _viewId;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        std::vector<std::unique_ptr<IRenderComponent>> _components;

        void beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder);
        void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder);
    };
}