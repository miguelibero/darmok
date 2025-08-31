#pragma once

#include <darmok/export.h>

#include <darmok/glm.hpp>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/transform_fwd.hpp>

#include <memory>
#include <cstdint>
#include <vector>
#include <optional>

#include <bx/bx.h>
#include <bgfx/bgfx.h>

namespace darmok
{    
    class Scene;
    class Camera;

    class DARMOK_EXPORT BX_NO_VTABLE ISceneComponent
    {
    public:
        virtual ~ISceneComponent() = default;
        virtual std::optional<entt::type_info> getSceneComponentType() const { return std::nullopt; }
        virtual void init(Scene& scene, App& app) {}
        virtual void shutdown() {}
        virtual bgfx::ViewId renderReset(bgfx::ViewId viewId) { return viewId; }
        virtual void update(float deltaTime) {}
        virtual void afterLoad() {}
    };

    template<typename T>
    class DARMOK_EXPORT BX_NO_VTABLE ITypeSceneComponent : public ISceneComponent
    {
    public:
        std::optional<entt::type_info> getSceneComponentType() const noexcept override
        {
            return entt::type_id<T>();
        }
    };

    class App;
    class SceneImpl;
    class RenderChain;
    struct Viewport;
    struct EntityFilter;
    class EntityView;

    namespace protobuf
    {
        class Scene;
    }

    using SceneComponentRefs = std::vector<std::reference_wrapper<ISceneComponent>>;
    using ConstSceneComponentRefs = std::vector<std::reference_wrapper<const ISceneComponent>>;
    
    class DARMOK_EXPORT Scene final
    {
    public:
        using Definition = protobuf::Scene;

        Scene() noexcept;
        Scene(App& app) noexcept;
        ~Scene() noexcept;

        SceneImpl& getImpl() noexcept;
        const SceneImpl& getImpl() const noexcept;

        entt::id_type getId() const noexcept;
        Scene& setName(const std::string& name) noexcept;
        const std::string& getName() const noexcept;
        std::string toString() const noexcept;

        Scene& setPaused(bool paused) noexcept;
        bool isPaused() const noexcept;

        RenderChain& getRenderChain() noexcept;
        const RenderChain& getRenderChain() const noexcept;

        const std::optional<Viewport>& getViewport() const noexcept;
        Scene& setViewport(const std::optional<Viewport>& vp) noexcept;
        Viewport getCurrentViewport() const noexcept;

        void addSceneComponent(std::unique_ptr<ISceneComponent>&& component) noexcept;
        bool removeSceneComponent(entt::id_type type) noexcept;
        bool hasSceneComponent(entt::id_type type) const noexcept;
        OptionalRef<ISceneComponent> getSceneComponent(entt::id_type type) noexcept;
        OptionalRef<const ISceneComponent> getSceneComponent(entt::id_type type) const noexcept;
        SceneComponentRefs getSceneComponents() noexcept;
        ConstSceneComponentRefs getSceneComponents() const noexcept;

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
            addSceneComponent(std::move(ptr));
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
        bool isValidEntity(Entity entity) const noexcept;

        Entity getEntity(entt::id_type type, const void* ptr) const noexcept;
        const void* getComponent(Entity entity, entt::id_type type) const noexcept;
        std::unordered_map<entt::type_info, const void*> getComponents(Entity entity) const noexcept;
        std::unordered_map<entt::type_info, void*> getComponents(Entity entity) noexcept;

        void destroyEntity(Entity entity, bool destroyChildren = false) noexcept;
        void destroyEntities() noexcept;
        void destroyEntities(const EntityFilter& filter) noexcept;

        void destroyEntityImmediate(Entity entity, bool destroyChildren = false) noexcept;
        void destroyEntitiesImmediate() noexcept;
        void destroyEntitiesImmediate(const EntityFilter& filter) noexcept;

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

        EntityView getEntities() const noexcept;
        EntityView getEntities(entt::id_type typeId) const noexcept;
        EntityView getEntities(const EntityFilter& filter, entt::id_type typeId = 0) const noexcept;

        template<typename T>
        EntityView getEntities(const EntityFilter& filter) const noexcept
        {
            return getEntities(filter, entt::type_hash<T>::value());
        }

        template<typename T>
        EntityView getEntities() const noexcept
        {
            return getEntities(entt::type_hash<T>::value());
        }

        template<typename T>
        EntityView getUpdateEntities() const noexcept
        {
            return getEntities<T>(getUpdateFilter());
        }

        bool isEntityAlive(Entity entity) const noexcept;

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
                return &view.template get<T>(entity) == &component;
            });
            return it != end;
        }

        template<typename T>
		Entity getEntity(const T& component) const noexcept
		{
            return getEntity(entt::type_hash<T>::value(), &component);
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
            return removeComponent(entity, entt::type_hash<T>::value());
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

        std::vector<Entity> getRootEntities() const noexcept;

        template<typename C>
        bool forEachParent(Entity entity, const C& callback)
        {
            if (entity == entt::null)
            {
                return false;
            }
            auto optTrans = getComponent<Transform>(entity);
            if (!optTrans)
            {
                return false;
            }
            auto& trans = optTrans.value();
            if (callback(entity, trans))
            {
                return true;
            }
            auto parent = getTransformParent(trans);
            if (!parent)
            {
                return false;
            }
            entity = getEntity(parent.value());
            return forEachParent(entity, callback);
        }

        template<typename C>
        bool forEachChild(const C& callback)
        {
            for (auto root : getRootEntities())
            {
                if (forEachChild(root, callback))
                {
                    return true;
                }
            }
            return false;
        }

        template<typename C>
        bool forEachChild(Entity entity, const C& callback)
        {
            if (entity == entt::null)
            {
                return false;
            }
            auto optTrans = getComponent<Transform>(entity);
            if (!optTrans)
            {
                return false;
            }
            auto& trans = optTrans.value();
            if (callback(entity, trans))
            {
                return true;
            }
            for (auto& child : getTransformChildren(trans))
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
            auto view = getComponents<T>();
            if (view.size_hint() == 0)
            {
                return nullptr;
            }
            return view.template get<T>(view.back());
        }

        template<typename T>
        OptionalRef<T> getLastComponent() noexcept
        {
            auto view = getComponents<T>();
            if (view.size_hint() == 0)
            {
                return nullptr;
            }
            return view.template get<T>(view.front());
        }

        template<typename T>
        std::vector<std::reference_wrapper<T>> getComponentsInChildren(Entity entity) noexcept
        {
            std::vector<std::reference_wrapper<T>> components;
            getComponentsInChildren<T>(entity, components);
            return components;
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

        Scene& setUpdateFilter(const EntityFilter& filter) noexcept;
        const EntityFilter& getUpdateFilter() const noexcept;       

    private:
        std::unique_ptr<SceneImpl> _impl;

        friend class SceneArchive;

        EntityRegistry& getRegistry() noexcept;
        const EntityRegistry& getRegistry() const noexcept;

        static OptionalRef<Transform> getTransformParent(const Transform& trans) noexcept;
        static TransformChildren getTransformChildren(const Transform& trans) noexcept;
    };

    class DARMOK_EXPORT SceneAppComponent final : public ITypeAppComponent<SceneAppComponent>
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
        bgfx::ViewId renderReset(bgfx::ViewId viewId) override;
        void render() override;
        void update(float dt) override;

    private:
        Scenes _scenes;
        OptionalRef<App> _app;
    };
}

namespace std
{
    inline std::string to_string(const darmok::Entity& v)
    {
        return to_string(static_cast<uint32_t>(v));
    }

    inline std::ostream& operator<<(std::ostream& out, const darmok::Entity& v)
    {
        return out << to_string(v);
    }
}