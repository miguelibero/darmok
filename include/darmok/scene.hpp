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
    typedef entt::basic_registry<Entity> Registry;

    class BX_NO_VTABLE ISceneRenderer
    {
    public:
        virtual ~ISceneRenderer() = default;
        virtual void init(Registry& registry) {};
        virtual void render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry) = 0;
    };

    class BX_NO_VTABLE ISceneLogicUpdater
    {
    public:
        virtual ~ISceneLogicUpdater() = default;
        virtual void init(Registry& registry) {};
        virtual void updateLogic(float dt, Registry& registry) = 0;
    };

    class Scene final
    {
    public:
        Scene();
        ~Scene();
        Entity createEntity();

        template<typename T, typename... A>
        T& addComponent(const Entity entity, A&&... args)
        {
            return getRegistry().emplace<T, A...>(entity, std::forward<A>(args)...);
        }

        template<typename T>
        T& getComponent(const Entity entity)
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
        Registry& getRegistry();
        const Registry& getRegistry() const;

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


        bool setPosition(const glm::vec3& v);
        bool setRotation(const glm::vec3& v);
        bool setScale(const glm::vec3& v);
        bool setPivot(const glm::vec3& v);
        void setParent(const OptionalRef<Transform>& parent);

        bool update();
        void setMatrix(const glm::mat4& v);
        const glm::mat4& getMatrix();
        const glm::mat4& getMatrix() const;

        static bool bgfxConfig(Entity entity, bgfx::Encoder& encoder, Registry& registry);
    
    private:
        bool _changed;
        glm::vec3 _position;
        glm::vec3 _rotation;
        glm::vec3 _scale;
        glm::vec3 _pivot;
        glm::mat4 _matrix;
        OptionalRef<Transform> _parent;
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
        Camera(const glm::mat4& matrix = {});
        const glm::mat4& getMatrix() const;
        void setMatrix(const glm::mat4& matrix);

    private:
        glm::mat4 _matrix;
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

