#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <bx/bx.h>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>

namespace darmok
{
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

    class Scene;

    class BX_NO_VTABLE ISceneLogicUpdater
    {
    public:
        virtual ~ISceneLogicUpdater() = default;
        virtual void init(Scene& scene, App& app) { };
        virtual void shutdown() { }
        virtual void update(float deltaTime) = 0;
    };

    class SceneImpl;

    class Scene final
    {
    public:
        Scene() noexcept;
        ~Scene() noexcept;

        template<typename T, typename... A>
        T& addLogicUpdater(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addLogicUpdater(std::move(ptr));
            return ref;
        }

        void init(App& app);
        void updateLogic(float dt);
        bgfx::ViewId render(bgfx::ViewId viewId);
        void shutdown();

        void addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater);

        EntityRegistry& getRegistry();
        const EntityRegistry& getRegistry() const;

    private:
        std::unique_ptr<SceneImpl> _impl;
    };
    
    class SceneAppComponent final : public AppComponent
    {
    public:
        SceneAppComponent(const std::shared_ptr<Scene>& scene = nullptr) noexcept;
        
        const std::shared_ptr<Scene>& getScene() const noexcept;
        void setScene(const std::shared_ptr<Scene>& scene) noexcept;
        const std::vector<std::shared_ptr<Scene>>& getScenes() const noexcept;
        std::shared_ptr<Scene> addScene() noexcept;
        bool addScene(const std::shared_ptr<Scene>& scene) noexcept;
        bool removeScene(const std::shared_ptr<Scene>& scene) noexcept;

        void init(App& app) override;
        void shutdown() override;
        bgfx::ViewId render(bgfx::ViewId viewId) const override;
        void updateLogic(float dt) override;
    private:
        std::shared_ptr<Scene> _mainScene;
        std::vector<std::shared_ptr<Scene>> _scenes;
        OptionalRef<App> _app;
    };
}

