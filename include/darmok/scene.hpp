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

        DLLEXPORT Entity createEntity() noexcept;
        DLLEXPORT bool destroyEntity(Entity entity) noexcept;

        template<typename T>
        auto getComponentView() noexcept
        {
            return getRegistry().view<T>();
        }

        template<typename T>
		Entity getEntity(const T& component) noexcept
		{
			auto& storage = getRegistry().storage<T>();
			return entt::to_entity(storage, component);
		}

        template<typename T>
        OptionalRef<T> getComponent(Entity entity) noexcept
        {
            return getRegistry().try_get<T>(entity);
        }

        template<typename T, typename... A>
        T& getOrAddComponent(Entity entity, A&&... args) noexcept
        {
            return getRegistry().get_or_emplace<T>(entity, std::forward<A>(args)...);
        }

        template<typename T, typename... A>
        T& addComponent(Entity entity, A&&... args) noexcept
        {
            return getRegistry().emplace<T>(entity, std::forward<A>(args)...);
        }

        template<typename T>
        bool removeComponent(Entity entity) noexcept
        {
            return getRegistry().remove<T>(entity) > 0;
        }

        template<typename T>
        OptionalRef<T> getComponentInParent(Entity entity) noexcept
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
            auto parent = trans->getParent();
            if (!parent)
            {
                return nullptr;
            }
            return getComponentInParent<T>(getEntity(parent.value()));
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
                auto childEntity = getEntity(child.value());
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

        template<typename T, typename C>
        void getComponentsInChildren(Entity entity, C& container) noexcept
        {
            auto& registry = getRegistry();
            auto comp = registry.try_get<T>(entity);
            if (comp != nullptr)
            {
                container.emplace_back(comp);
            }
            auto trans = registry.try_get<Transform>(entity);
            if (trans == nullptr)
            {
                return;
            }
            for (auto& child : trans->getChildren())
            {
                auto childEntity = getEntity(child.value());
                if (childEntity != entt::null)
                {
                    getComponentsInChildren<T, C>(childEntity, container);
                }
            }
        }

    private:
        std::unique_ptr<SceneImpl> _impl;
    };
    
    class SceneAppComponent final : public AppComponent
    {
    public:
        DLLEXPORT SceneAppComponent(const std::shared_ptr<Scene>& scene = nullptr) noexcept;
        DLLEXPORT std::shared_ptr<Scene> getScene() const noexcept;
        DLLEXPORT SceneAppComponent& setScene(const std::shared_ptr<Scene>& scene) noexcept;

        void init(App& app) override;
        void shutdown() override;
        bgfx::ViewId render(bgfx::ViewId viewId) const override;
        void updateLogic(float dt) override;
    private:
        std::shared_ptr<Scene> _scene;
        OptionalRef<App> _app;
    };
}

