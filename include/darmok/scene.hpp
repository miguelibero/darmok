#pragma once

#include <memory>
#include <cstdint>
#include <darmok/app.hpp>
#include <glm/glm.hpp>
#include <bx/bx.h>
#include <entt/entt.hpp>

namespace darmok
{
    class SceneImpl;

    typedef uint32_t Entity;
    typedef entt::basic_registry<Entity> Registry;

    class BX_NO_VTABLE ISceneRenderer
    {
    public:
        virtual ~ISceneRenderer() = default;
        virtual void init() {};
        virtual void render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry) = 0;
    };

    class BX_NO_VTABLE ISceneLogicUpdater
    {
    public:
        virtual ~ISceneLogicUpdater() = default;
        virtual void init() {};
        virtual void updateLogic(Registry& registry) = 0;
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
        Transform(const glm::vec3& position = glm::vec3(), const glm::vec3& rotation = glm::vec3(), const glm::vec3& scale = glm::vec3(1), const glm::vec3& pivot = glm::vec3());
    
        const glm::vec3& getPosition() const;
        const glm::vec3& getRotation() const;
        const glm::vec3& getScale() const;
        const glm::vec3& getPivot() const;

        bool setPosition(const glm::vec3& v);
        bool setRotation(const glm::vec3& v);
        bool setScale(const glm::vec3& v);
        bool setPivot(const glm::vec3& v);

        bool update();
        const glm::mat4x4& getMatrix();
        const glm::mat4x4& getMatrix() const;

        static bool bgfxConfig(Entity entity, bgfx::Encoder& encoder, Registry& registry);
    
    private:
        bool _changed;
        glm::vec3 _position;
        glm::vec3 _rotation;
        glm::vec3 _scale;
        glm::vec3 _pivot;
        glm::mat4x4 _matrix;
    };

    typedef std::array<uint8_t, 4> Color;

    struct Colors
    {
        static const uint8_t maxValue;
        static const Color black;
        static const Color white;
        static const Color red;
        static const Color green;
        static const Color blue;
    };

    typedef uint16_t VertexIndex;

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
        Camera(const glm::mat4x4& matrix = {});
        const glm::mat4x4& getMatrix() const;
        void setMatrix(const glm::mat4x4& matrix);

    private:
        glm::mat4x4 _matrix;
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

