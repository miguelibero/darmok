#pragma once

#include <darmok/export.h>
#include <memory>
#include <cstdint>
#include <vector>
#include <darmok/glm.hpp>
#include <bx/bx.h>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/transform.hpp>

namespace darmok
{    
    class Scene;

    class DARMOK_EXPORT BX_NO_VTABLE ISceneComponent
    {
    public:
        virtual ~ISceneComponent() = default;
        virtual void init(Scene& scene, App& app) { };
        virtual void shutdown() { }
        virtual void update(float deltaTime) = 0;
    };

    class SceneImpl;

    class DARMOK_EXPORT Scene final
    {
    public:
        Scene() noexcept;
        ~Scene() noexcept;

        template<typename T, typename... A>
        T& addSceneComponent(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addSceneComponent(std::move(ptr));
            return ref;
        }

        SceneImpl& getImpl() noexcept;
        const SceneImpl& getImpl() const noexcept;

        void init(App& app);
        void updateLogic(float dt);
        bgfx::ViewId render(bgfx::ViewId viewId);
        void shutdown();

        void addSceneComponent(std::unique_ptr<ISceneComponent>&& component) noexcept;
        bool removeSceneComponent(const ISceneComponent& component) noexcept;
        bool hasSceneComponent(const ISceneComponent& component) const noexcept;

        EntityRegistry& getRegistry();
        const EntityRegistry& getRegistry() const;

        Entity createEntity() noexcept;
        bool destroyEntity(Entity entity) noexcept;

        template<typename T>
        auto getComponentView() const noexcept
        {
            return getRegistry().view<T>();
        }

        template<typename T>
        auto getComponentView() noexcept
        {
            return getRegistry().view<T>();
        }

        template<typename T>
        bool hasComponent(const T& component) const noexcept
        {
            auto view = getComponentView<T>();
            auto end = std::cend(view);
            auto it = std::find_if(std::cbegin(view), end, [&component, &view](const auto& entity) {
                return &view.get<T>(entity) == &component;
            });
            return it != end;
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

    class DARMOK_EXPORT SceneAppComponent final : public AppComponent
    {
    public:
        SceneAppComponent(const std::shared_ptr<Scene>& scene = nullptr) noexcept;
        std::shared_ptr<Scene> getScene() const noexcept;
        SceneAppComponent& setScene(const std::shared_ptr<Scene>& scene) noexcept;

        void init(App& app) override;
        void shutdown() override;
        bgfx::ViewId render(bgfx::ViewId viewId) const override;
        void updateLogic(float dt) override;
    private:
        std::shared_ptr<Scene> _scene;
        OptionalRef<App> _app;
    };
}

