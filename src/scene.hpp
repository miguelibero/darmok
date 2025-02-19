#pragma once

#include <bgfx/bgfx.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/utils.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace darmok
{
    class ISceneDelegate;
    class ISceneComponent;
    class Scene;
    class App;
    class Camera;
    class FrameBuffer;

    using SceneComponentRefs = std::vector<std::reference_wrapper<ISceneComponent>>;
    using ConstSceneComponentRefs = std::vector<std::reference_wrapper<const ISceneComponent>>;

    class SceneImpl final : IRenderChainDelegate
    {
    public:
        SceneImpl(Scene& scene) noexcept;
        ~SceneImpl() noexcept;

        void setDelegate(const OptionalRef<ISceneDelegate>& dlg) noexcept;
        const OptionalRef<ISceneDelegate>& getDelegate() const noexcept;

        void addSceneComponent(std::unique_ptr<ISceneComponent>&& component) noexcept;
        bool removeSceneComponent(entt::id_type type) noexcept;
        bool hasSceneComponent(entt::id_type type) const noexcept;
        OptionalRef<ISceneComponent> getSceneComponent(entt::id_type type) noexcept;
        OptionalRef<const ISceneComponent> getSceneComponent(entt::id_type type) const noexcept;
        SceneComponentRefs getSceneComponents() noexcept;
        ConstSceneComponentRefs getSceneComponents() const noexcept;

        entt::id_type getId() const noexcept;
        void setName(const std::string& name) noexcept;
        const std::string& getName() const noexcept;
        std::string toString() const noexcept;
        std::string getDescName() const noexcept;

        void setPaused(bool paused) noexcept;
        bool isPaused() const noexcept;

        std::vector<Entity> getRootEntities() const noexcept;

        void destroyEntities() noexcept;
        void destroyEntities(const EntityFilter& filter);
        void destroyEntity(Entity entity, bool destroyChildren = false) noexcept;

        void destroyEntityImmediate(Entity entity, bool destroyChildren = false) noexcept;
        void destroyEntitiesImmediate() noexcept;
        void destroyEntitiesImmediate(const EntityFilter& filter) noexcept;

        EntityRegistry& getRegistry() noexcept;
        const EntityRegistry& getRegistry() const noexcept;
        bool removeComponent(Entity entity, entt::id_type typeId) noexcept;

        RenderChain& getRenderChain() noexcept;
        const RenderChain& getRenderChain() const noexcept;

        const std::optional<Viewport>& getViewport() const noexcept;
        void setViewport(const std::optional<Viewport>& vport) noexcept;
        Viewport getCurrentViewport() const noexcept;

        void init(App& app);
        void update(float deltaTime);
        bgfx::ViewId renderReset(bgfx::ViewId viewId);
        void render();
        void shutdown();

        void setUpdateFilter(const EntityFilter& filter) noexcept;
        const EntityFilter& getUpdateFilter() const noexcept;
    private:
        using Components = std::vector<std::shared_ptr<ISceneComponent>>;

        std::unordered_set<Entity> _pendingDestroy;
        bool _pendingDestroyAll;
        EntityFilter _pendingDestroyFilter;
        std::string _name;
        bool _paused;
        Components _components;
        EntityRegistry _registry;
        Scene& _scene;
        OptionalRef<App> _app;
        RenderChain _renderChain;
        std::optional<Viewport> _viewport;
        EntityFilter _updateFilter;
        OptionalRef<ISceneDelegate> _delegate;

        Components::iterator findSceneComponent(entt::id_type type) noexcept;
        Components::const_iterator findSceneComponent(entt::id_type type) const noexcept;

        void onCameraConstructed(EntityRegistry& registry, Entity entity);
        void onCameraDestroyed(EntityRegistry& registry, Entity entity);

        Viewport getRenderChainViewport() const noexcept override;
        void onRenderChainChanged() noexcept override;
        void destroyPendingEntities() noexcept;

        bool shouldCameraRender(const Camera& cam) const;
        bool shouldEntityBeSerialized(Entity entity) const;
    };
}
