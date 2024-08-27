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
        virtual void init(Scene& scene, App& app) { }
        virtual void shutdown() { }
        virtual void renderReset() { }
        virtual void update(float deltaTime) { }
    };

    class SceneImpl;
    class RenderGraphDefinition;
    class RenderChain;

    class DARMOK_EXPORT Scene final
    {
    public:
        Scene() noexcept;
        ~Scene() noexcept;

        SceneImpl& getImpl() noexcept;
        const SceneImpl& getImpl() const noexcept;

        void init(App& app);
        void update(float dt);
        void renderReset();
        void shutdown();

        RenderGraphDefinition& getRenderGraph() noexcept;
        const RenderGraphDefinition& getRenderGraph() const noexcept;
        RenderChain& getRenderChain() noexcept;
        const RenderChain& getRenderChain() const noexcept;

        void addSceneComponent(entt::id_type type, std::unique_ptr<ISceneComponent>&& component) noexcept;
        bool removeSceneComponent(entt::id_type type) noexcept;
        bool hasSceneComponent(entt::id_type type) const noexcept;
        OptionalRef<ISceneComponent> getSceneComponent(entt::id_type type) noexcept;
        OptionalRef<const ISceneComponent> getSceneComponent(entt::id_type type) const noexcept;

        template<typename T>
        OptionalRef<T> getSceneComponent() noexcept
        {
            auto ref = getSceneComponent(entt::type_hash<T>::value());
            if (ref)
            {
                return (T*)ref.ptr();
            }
            return nullptr;
        }

        template<typename T>
        OptionalRef<const T> getSceneComponent() const noexcept
        {
            auto ref = getSceneComponent(entt::type_hash<T>::value());
            if (ref)
            {
                return (const T*)ref.ptr();
            }
            return nullptr;
        }

        template<typename T, typename... A>
        T& addSceneComponent(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addSceneComponent(entt::type_hash<T>::value(), std::move(ptr));
            return ref;
        }

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

        template<typename C>
        bool forEachParent(Entity entity, const C& callback)
        {
            if (entity == entt::null)
            {
                return false;
            }
            auto trans = getRegistry().try_get<Transform>(entity);
            if (trans == nullptr)
            {
                return false;
            }
            if (callback(entity, *trans))
            {
                return true;
            }
            auto parent = trans->getParent();
            if (!parent)
            {
                return false;
            }
            entity = getEntity(parent.value());
            return forEachParent(entity, callback);
        }

        template<typename C>
        bool forEachChild(Entity entity, const C& callback)
        {
            if (entity == entt::null)
            {
                return false;
            }
            auto trans = getRegistry().try_get<Transform>(entity);
            if (trans == nullptr)
            {
                return false;
            }
            if (callback(entity, *trans))
            {
                return true;
            }
            for (auto& child : trans->getChildren())
            {
                entity = getEntity(child.get());
                if (forEachChild(entity, callback))
                {
                    return true;
                }
            }
            return false;
        }

        template<typename T>
        OptionalRef<T> getComponentInParent(Entity entity) noexcept
        {
            OptionalRef<T> comp;
            forEachParent(entity, [&comp, this](auto entity, auto& trans) {
                comp = getComponent<T>(entity);
                return !comp.empty();
            });
            return comp;
        }

        template<typename T>
        OptionalRef<T> getComponentInChildren(Entity entity) noexcept
        {
            OptionalRef<T> comp;
            forEachChild(entity, [&comp, this](auto entity, auto& trans) {
                comp = getComponent<T>(entity);
                return !comp.empty();
            });
            return comp;
        }

        template<typename T, typename C>
        void getComponentsInChildren(Entity entity, C& container) noexcept
        {
            forEachChild(entity, [&container, this](auto entity, auto& trans) {
                if (auto comp = getComponent<T>(entity))
                {
                    container.emplace_back(comp);
                }
                return false;
            });
        }

    private:
        std::unique_ptr<SceneImpl> _impl;
    };

    class DARMOK_EXPORT SceneAppComponent final : public IAppComponent
    {
    public:
        SceneAppComponent(const std::shared_ptr<Scene>& scene = nullptr) noexcept;
        std::shared_ptr<Scene> getScene(size_t i = 0) const noexcept;
        SceneAppComponent& setScene(const std::shared_ptr<Scene>& scene, size_t i = 0) noexcept;
        std::shared_ptr<Scene> addScene() noexcept;
        SceneAppComponent& addScene(const std::shared_ptr<Scene>& scene) noexcept;

        void init(App& app) override;
        void shutdown() override;
        void renderReset() override;
        void update(float dt) override;

    private:
        using Scenes = std::vector<std::shared_ptr<Scene>>;
        Scenes _scenes;
        OptionalRef<App> _app;
    };
}

