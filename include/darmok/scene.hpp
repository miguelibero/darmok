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
        virtual void update(float dt) = 0;
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

    class Transform final
    {
    public:
        static constexpr auto in_place_delete = true;

        Transform(const glm::mat4& mat, const OptionalRef<Transform>& parent = std::nullopt);
        Transform(const glm::vec3& position = glm::vec3(), const glm::vec3& rotation = glm::vec3(), const glm::vec3& scale = glm::vec3(1), const glm::vec3& pivot = glm::vec3(), const OptionalRef<Transform>& parent = std::nullopt);
    
        const glm::vec3& getPosition() const;
        const glm::vec3& getRotation() const;
        const glm::vec3& getScale() const;
        const glm::vec3& getPivot() const;
        const OptionalRef<Transform>& getParent() const;

        OptionalRef<Transform> getParent();

        Transform& setPosition(const glm::vec3& v);
        Transform& setRotation(const glm::vec3& v);
        Transform& setScale(const glm::vec3& v);
        Transform& setPivot(const glm::vec3& v);
        Transform& setParent(const OptionalRef<Transform>& parent);

        void update();
        void setMatrix(const glm::mat4& v);
        const glm::mat4& getMatrix();
        const glm::mat4& getMatrix() const;
        const glm::mat4& getInverse();
        const glm::mat4& getInverse() const;

        static bool bgfxConfig(Entity entity, bgfx::Encoder& encoder, EntityRegistry& registry);
    
    private:
        bool _matrixUpdatePending;
        bool _inverseUpdatePending;
        glm::vec3 _position;
        glm::vec3 _rotation;
        glm::vec3 _scale;
        glm::vec3 _pivot;
        glm::mat4 _matrix;
        glm::mat4 _inverse;
        OptionalRef<Transform> _parent;

        void setPending(bool v = true);
        bool updateMatrix();
        bool updateInverse();
    };

    typedef glm::vec<2, uint16_t> ViewVec;

    class ViewRect final
    {
    public:
        ViewRect(const ViewVec& size, const ViewVec& origin = {});
        void setSize(const ViewVec& size);
        void setOrigin(const ViewVec& origin);
        const ViewVec& getSize() const;
        const ViewVec& getOrigin() const;
        void bgfxConfig(bgfx::ViewId viewId) const; 
    private:
        ViewVec _size;
        ViewVec _origin;
    };

    class Camera final
    {
    public:
        Camera(const glm::mat4& matrix = {}, bgfx::ViewId viewId = 0);
        const glm::mat4& getMatrix() const;
        Camera& setMatrix(const glm::mat4& matrix);
        Camera& setProjection(float fovy, float aspect, float near, float far);
        Camera& setProjection(float fovy, float aspect, float near = 0.f);
        Camera& setOrtho(float left, float right, float bottom, float top, float near = 0.f, float far = bx::kFloatLargest, float offset = 0.f);
        Camera& setViewId(bgfx::ViewId viewId);
        bgfx::ViewId getViewId() const;
        Camera& setEntityFilter(std::unique_ptr<IEntityFilter>&& filter);

        template<typename T>
        Camera& setEntityComponentFilter()
        {
            return setEntityFilter(std::make_unique<EntityComponentFilter<T>>());
        }

        OptionalRef<const IEntityFilter> getEntityFilter() const;
        OptionalRef<IEntityFilter> getEntityFilter();
    private:
        glm::mat4 _matrix;
        bgfx::ViewId _viewId;
        std::unique_ptr<IEntityFilter> _entityFilter;
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

