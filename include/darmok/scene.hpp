#pragma once

#include <memory>
#include <cstdint>
#include <darmok/app.hpp>
#include <glm/glm.hpp>
#include <bx/bx.h>
#include <entt/entt.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class SceneImpl;

    typedef uint32_t Entity;
    typedef entt::basic_registry<Entity> EntityRegistry;
    typedef entt::basic_runtime_view<entt::basic_sparse_set<Entity>> EntityRuntimeView;

    class BX_NO_VTABLE IEntityFilter
    {
    public:
        virtual ~IEntityFilter() = default;
        virtual void init(EntityRegistry& registry) { _registry = registry; };
        virtual void operator()(EntityRuntimeView& view) const = 0;
    private:
        OptionalRef<EntityRegistry> _registry;
    protected:
        EntityRegistry& getRegistry() const { return _registry.value(); }
    };

    template<typename T>
    class EntityComponentFilter final : public IEntityFilter
    {
    public:
        void operator()(EntityRuntimeView& view) const override
        {
            view.iterate(getRegistry().storage<T>());
        }
    };

    class BX_NO_VTABLE ISceneRenderer
    {
    public:
        virtual ~ISceneRenderer() = default;
        virtual void init(EntityRegistry& registry) { _registry = registry; };
        virtual void render(EntityRuntimeView& entities, bgfx::Encoder& encoder, bgfx::ViewId viewId) = 0;
    private:
        OptionalRef<EntityRegistry> _registry;
    protected:
        EntityRegistry& getRegistry() const { return _registry.value(); }
    };

    class BX_NO_VTABLE ISceneLogicUpdater
    {
    public:
        virtual ~ISceneLogicUpdater() = default;
        virtual void init(EntityRegistry& registry) { _registry = registry; };
        virtual void update(float deltaTime) = 0;
    private:
        OptionalRef<EntityRegistry> _registry;
    protected:
        EntityRegistry& getRegistry() const { return _registry.value(); }
    };

    class Scene final
    {
    public:
        Scene();
        ~Scene();
        Entity createEntity();

        template<typename T, typename... A>
        decltype(auto) addComponent(const Entity entity, A&&... args)
        {
            return getRegistry().emplace<T, A...>(entity, std::forward<A>(args)...);
        }

        template<typename T>
        decltype(auto) getComponent(const Entity entity)
        {
            return getRegistry().get<T>(entity);
        }

        template<typename T>
        OptionalRef<T> tryGetComponent(const Entity entity)
        {
            return getRegistry().try_get<T>(entity);
        }

        template<typename T, typename... A>
        T& addRenderer(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addRenderer(std::move(ptr));
            return ref;
        }

        template<typename T, typename... A>
        T& addLogicUpdater(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addLogicUpdater(std::move(ptr));
            return ref;
        }

        void init();
        void updateLogic(float dt);
        void render(bgfx::ViewId viewId);

        void addRenderer(std::unique_ptr<ISceneRenderer>&& renderer);
        void addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater);

    private:
        EntityRegistry& getRegistry();
        const EntityRegistry& getRegistry() const;

        std::unique_ptr<SceneImpl> _impl;
    };
    
    class SceneAppComponent final : public AppComponent
    {
    public:
        Scene& getScene();
        const Scene& getScene() const;

        void init() override;
        void render(bgfx::ViewId viewId) override;
        void updateLogic(float dt) override;
    private:
        Scene _scene;
    };
}

