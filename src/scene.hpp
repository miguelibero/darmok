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

        void addSceneComponent(entt::id_type type, std::unique_ptr<ISceneComponent>&& component) noexcept;
        bool removeSceneComponent(entt::id_type type) noexcept;
        bool hasSceneComponent(entt::id_type type) const noexcept;
        OptionalRef<ISceneComponent> getSceneComponent(entt::id_type type) noexcept;
        OptionalRef<const ISceneComponent> getSceneComponent(entt::id_type type) const noexcept;

        EntityRegistry& getRegistry();
        const EntityRegistry& getRegistry() const;

        RenderGraphDefinition& getRenderGraph() noexcept;
        const RenderGraphDefinition& getRenderGraph() const noexcept;

        OptionalRef<App> getApp() noexcept;
        OptionalRef<const App> getApp() const noexcept;

        void init(App& app);
        void update(float dt);
        void renderReset();
        void shutdown();
    private:
        using Components = std::vector<std::pair<entt::id_type, std::unique_ptr<ISceneComponent>>>;

        Components _components;
        EntityRegistry _registry;
        Scene& _scene;
        OptionalRef<App> _app;
        RenderGraphDefinition _renderGraph;

        Components::iterator findComponent(entt::id_type type) noexcept;
        Components::const_iterator findComponent(entt::id_type type) const noexcept;

        void onCameraConstructed(EntityRegistry& registry, Entity entity);
        void onCameraDestroyed(EntityRegistry& registry, Entity entity);
    };
}
