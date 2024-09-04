#pragma once

#include <bgfx/bgfx.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/viewport.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <memory>

namespace darmok
{
    class ISceneComponent;
    class Scene;
    class App;
    class FrameBuffer;

    class SceneImpl final : IRenderChainDelegate
    {
    public:
        SceneImpl(Scene& sceme) noexcept;
        ~SceneImpl() noexcept;

        void addSceneComponent(entt::id_type type, std::unique_ptr<ISceneComponent>&& component) noexcept;
        bool removeSceneComponent(entt::id_type type) noexcept;
        bool hasSceneComponent(entt::id_type type) const noexcept;
        OptionalRef<ISceneComponent> getSceneComponent(entt::id_type type) noexcept;
        OptionalRef<const ISceneComponent> getSceneComponent(entt::id_type type) const noexcept;

        void setName(const std::string& name) noexcept;
        const std::string& getName() const noexcept;

        EntityRegistry& getRegistry();
        const EntityRegistry& getRegistry() const;

        RenderGraphDefinition& getRenderGraph() noexcept;
        const RenderGraphDefinition& getRenderGraph() const noexcept;
        RenderChain& getRenderChain() noexcept;
        const RenderChain& getRenderChain() const noexcept;

        const std::optional<Viewport>& getViewport() const noexcept;
        void setViewport(const std::optional<Viewport>& vp) noexcept;
        Viewport getCurrentViewport() const noexcept;

        void init(App& app);
        void update(float dt);
        void renderReset();
        void shutdown();
    private:
        using Components = std::vector<std::pair<entt::id_type, std::unique_ptr<ISceneComponent>>>;

        std::string _name;
        Components _components;
        EntityRegistry _registry;
        Scene& _scene;
        OptionalRef<App> _app;
        RenderGraphDefinition _renderGraph;
        RenderChain _renderChain;
        std::optional<Viewport> _viewport;

        Components::iterator findSceneComponent(entt::id_type type) noexcept;
        Components::const_iterator findSceneComponent(entt::id_type type) const noexcept;

        void onCameraConstructed(EntityRegistry& registry, Entity entity);
        void onCameraDestroyed(EntityRegistry& registry, Entity entity);

        Viewport getRenderChainViewport() const noexcept override;
        void onRenderChainInputChanged() noexcept override;
        void updateRenderGraph() noexcept;
    };
}
