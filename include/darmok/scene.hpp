#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <bx/bx.h>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/transform.hpp>

namespace darmok
{    
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

        DLLEXPORT void addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater);

        DLLEXPORT EntityRegistry& getRegistry();
        DLLEXPORT const EntityRegistry& getRegistry() const;

        template<typename T>
		Entity getEntity(const T& component) noexcept
		{
			auto& storage = getRegistry().storage<T>();
			return entt::to_entity(storage, component);
		}

        template<typename T>
        OptionalRef<T> getComponentInChildren(Entity entity) noexcept
        {
            auto& registry = getRegistry();
            auto comp = registry.try_get<T>(entity);
            if (comp != nullptr)
            {
                return OptionalRef<T>(comp);
            }
            auto trans = registry.try_get<Transform>(entity);
            if (trans == nullptr)
            {
                return nullptr;
            }
            for (auto& child : trans->getChildren())
            {
                auto childEntity = entt::to_entity(registry.storage<Transform>(), child.value());
                if (childEntity != entt::null)
                {
                    auto comp = getComponentInChildren<T>(childEntity);
                    if (comp)
                    {
                        return comp;
                    }
                }
            }
            return nullptr;
        }

    private:
        std::unique_ptr<SceneImpl> _impl;
    };
    
    class SceneAppComponent final : public AppComponent
    {
    public:
        DLLEXPORT SceneAppComponent(const std::shared_ptr<Scene>& scene = nullptr) noexcept;
        
        DLLEXPORT std::shared_ptr<Scene> getScene() const noexcept;
        DLLEXPORT void setScene(const std::shared_ptr<Scene>& scene) noexcept;
        DLLEXPORT const std::vector<std::shared_ptr<Scene>>& getScenes() const noexcept;
        DLLEXPORT std::shared_ptr<Scene> addScene() noexcept;
        DLLEXPORT bool addScene(const std::shared_ptr<Scene>& scene) noexcept;
        DLLEXPORT bool removeScene(const std::shared_ptr<Scene>& scene) noexcept;

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

