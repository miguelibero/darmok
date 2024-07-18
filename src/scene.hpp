#pragma once

#include <bgfx/bgfx.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/render_graph.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <memory>

namespace darmok
{
    class ISceneComponent;
    class Scene;
    class App;

    class SceneImpl final
    {
    public:
        SceneImpl(Scene& sceme) noexcept;
        ~SceneImpl() noexcept;
        void addSceneComponent(std::unique_ptr<ISceneComponent>&& comp) noexcept;
        bool removeSceneComponent(const ISceneComponent& comp) noexcept;
        bool hasSceneComponent(const ISceneComponent& comp) const noexcept;

        EntityRegistry& getRegistry();
        const EntityRegistry& getRegistry() const;

        RenderGraphDefinition& getRenderGraph() noexcept;
        const RenderGraphDefinition& getRenderGraph() const noexcept;

        OptionalRef<App> getApp() noexcept;
        OptionalRef<const App> getApp() const noexcept;

        void init(App& app);
        void updateLogic(float dt);
        void shutdown();
    private:
        std::vector<std::unique_ptr<ISceneComponent>> _components;
        EntityRegistry _registry;
        Scene& _scene;
        OptionalRef<App> _app;
        RenderGraphDefinition _renderGraph;

        void onCameraConstructed(EntityRegistry& registry, Entity entity);
        void onCameraDestroyed(EntityRegistry& registry, Entity entity);
    };
}
