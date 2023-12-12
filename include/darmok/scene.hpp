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

    class BX_NO_VTABLE IEntityRenderer
    {
    public:
        virtual ~IEntityRenderer() = default;
        virtual bool render(bgfx::Encoder& encoder, Entity& entity, Registry& registry) = 0;
    };

    class BX_NO_VTABLE ISceneLogicUpdater
    {
    public:
        virtual ~ISceneLogicUpdater() = default;
        virtual void updateLogic(Registry& registry) = 0;
    };

    class Scene final
    {
    public:
        Scene(const bgfx::ProgramHandle& program);
        ~Scene();
        Entity createEntity();

        template<typename T, typename... A>
        T& addComponent(const Entity entity, A&&... args)
        {
            return getRegistry().emplace<T, A...>(entity, std::forward<A>(args)...);
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

        void updateLogic(float dt);
        void render(bgfx::ViewId viewId);

        void addRenderer(std::unique_ptr<IEntityRenderer>&& renderer);
        void addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& renderer);

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

    class Program final
    {
    public:
        Program(const bgfx::ProgramHandle& handle);
        const bgfx::ProgramHandle& getHandle() const;
        void setHandle(const bgfx::ProgramHandle& handle);

    private:
        bgfx::ProgramHandle _handle;
    };

    class Blend final
    {
    public:
        Blend(uint64_t src, uint64_t dst);
        uint64_t getState() const;
        uint64_t getSource() const;
        uint64_t getDestination() const;
        void setSource(uint64_t src);
        void setDestination(uint64_t dst);
    private:
        uint64_t _src;
        uint64_t _dst;
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
        void render(bgfx::ViewId viewId) const; 
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
        SceneAppComponent(const bgfx::ProgramHandle& program);
        Scene& getScene();
        const Scene& getScene() const;
        void render(bgfx::ViewId viewId) override;
        void updateLogic(float dt) override;
    private:
        bgfx::ProgramHandle _program;
        Scene _scene;
    };
}

