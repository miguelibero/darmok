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
    struct Viewport;
    struct EntityFilter;

    class DARMOK_EXPORT Scene final
    {
    public:
        Scene() noexcept;
        ~Scene() noexcept;

        std::string toString() const noexcept;

        SceneImpl& getImpl() noexcept;
        const SceneImpl& getImpl() const noexcept;

        void init(App& app);
        void update(float dt);
        void renderReset();
        void shutdown();

        Scene& setName(const std::string& name) noexcept;
        const std::string& getName() const noexcept;

        RenderGraphDefinition& getRenderGraph() noexcept;
        const RenderGraphDefinition& getRenderGraph() const noexcept;
        RenderChain& getRenderChain() noexcept;
        const RenderChain& getRenderChain() const noexcept;

        const std::optional<Viewport>& getViewport() const noexcept;
        Scene& setViewport(const std::optional<Viewport>& vp) noexcept;
        Viewport getCurrentViewport() const noexcept;

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

        template<typename T, typename... A>
        T& getOrAddSceneComponent(A&&... args) noexcept
        {
            if (auto comp = getSceneComponent<T>())
            {
                return comp.value();
            }
            return addSceneComponent<T>(std::forward<A>(args)...);
        }

        Entity createEntity() noexcept;
        void destroyEntity(Entity entity) noexcept;
        bool isValidEntity(Entity entity) const noexcept;

        template<typename T>
        auto getComponents() const noexcept
        {
            return getRegistry().view<T>();
        }

        template<typename T>
        auto getComponents() noexcept
        {
            return getRegistry().view<T>();
        }

        EntityRuntimeView getEntities(const EntityFilter& filter) const noexcept;

        template<typename T>
        EntityRuntimeView getEntities(const EntityFilter& filter) const noexcept
        {
            if (auto storage = getRegistry().storage<T>())
            {
                auto view = createEntityRuntimeView(filter);
                view.iterate(*storage);
                return view;
            }
            return EntityRuntimeView();
        }

        template<typename T>
        bool hasComponent(Entity entity) const noexcept
        {
            return hasComponent(entity, entt::type_hash<T>::value());
        }

        template<typename T>
        bool hasSpecificComponent(const T& component) const noexcept
        {
            auto view = getComponents<T>();
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
        OptionalRef<T> getCustomComponent(Entity entity, entt::id_type type) noexcept
        {
            auto& storage = getRegistry().storage<T>(type);
            if (!storage.contains(entity))
            {
                return nullptr;
            }
            return storage.get(entity);
        }

        template<typename T, typename... A>
        T& addCustomComponent(Entity entity, entt::id_type type, A&&... args) noexcept
        {
            return getRegistry().storage<T>(type).emplace(entity, std::forward<A>(args)...);
        }

        template<typename T>
        bool removeComponent(Entity entity) noexcept
        {
            return removeComponent(entity, entt::type_hash<T>::value()) > 0;
        }

        template<typename T, typename It>
        size_t removeComponents(It first, It last) noexcept
        {
            return getRegistry().remove<T>(first, last);
        }

        bool removeComponent(Entity entity, entt::id_type typeId) noexcept;
        bool hasComponent(Entity entity, entt::id_type typeId) const noexcept;

        template<typename C>
        bool forEachEntity(const C& callback)
        {
            for (auto& entity : getComponents<Entity>())
            {
                if (callback(entity))
                {
                    return true;
                }
            }
            return false;
        }

        template<typename C>
        bool forEachParent(Entity entity, const C& callback)
        {
            if (entity == entt::null)
            {
                return false;
            }
            auto trans = getComponent<Transform>(entity);
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
            auto trans = getComponent<Transform>(entity);
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
                    container.emplace_back(comp.value());
                }
                return false;
            });
        }

        template<typename T>
        OptionalRef<T> getFirstComponent() noexcept
        {
            for (auto [entity, comp] : getComponents<T>().each())
            {
                return comp;
            }
            return nullptr;
        }

        template<typename T>
        std::vector<std::reference_wrapper<T>> getComponentsInChildren(Entity entity) noexcept
        {
            std::vector<std::reference_wrapper<T>> components;
            getComponentsInChildren<T>(entity, components);
            return components;
        }

        static void registerComponentDependency(entt::id_type typeId1, entt::id_type typeId2);

        template<typename T1, typename T2>
        static void registerComponentDependency()
        {
            registerComponentDependency(entt::type_hash<T1>::value(), entt::type_hash<T2>::value());
        }


        template<typename T>
        auto onConstructComponent()
        {
            return getRegistry().on_construct<T>();
        }

        template<typename T>
        auto onDestroyComponent()
        {
            return getRegistry().on_construct<T>();
        }

    private:
        std::unique_ptr<SceneImpl> _impl;

        EntityRuntimeView createEntityRuntimeView(const EntityFilter& filter) const noexcept;
        EntityRegistry& getRegistry();
        const EntityRegistry& getRegistry() const;
    };

    struct DARMOK_EXPORT EntityFilter final
    {
        using Container = std::unordered_set<entt::id_type>;

        template<typename T>
        EntityFilter& include() noexcept
        {
            return include(entt::type_hash<T>::value());
        }

        EntityFilter& include(entt::id_type idType) noexcept;

        template<typename T>
        EntityFilter& exclude() noexcept
        {
            return exclude(entt::type_hash<T>::value());
        }

        EntityFilter& exclude(entt::id_type idType) noexcept;

        const Container& getIncludes() const noexcept;
        const Container& getExcludes() const noexcept;
        std::string toString() const noexcept;

        bool operator==(const EntityFilter& other) const noexcept;
        bool operator!=(const EntityFilter& other) const noexcept;

        EntityFilter operator+(const EntityFilter& other) const noexcept;
        EntityFilter& operator+=(const EntityFilter& other) noexcept;

    private:
        Container _includes;
        Container _excludes;
    };

    class DARMOK_EXPORT SceneAppComponent final : public IAppComponent
    {
    public:
        using Scenes = std::vector<std::shared_ptr<Scene>>;

        SceneAppComponent(const std::shared_ptr<Scene>& scene = nullptr) noexcept;
        std::shared_ptr<Scene> getScene(size_t i = 0) const noexcept;
        SceneAppComponent& setScene(const std::shared_ptr<Scene>& scene, size_t i = 0) noexcept;
        std::shared_ptr<Scene> addScene() noexcept;
        SceneAppComponent& addScene(const std::shared_ptr<Scene>& scene) noexcept;
        const Scenes& getScenes() const noexcept;

        void init(App& app) override;
        void shutdown() override;
        void renderReset() override;
        void update(float dt) override;

    private:
        Scenes _scenes;
        OptionalRef<App> _app;
    };
}

