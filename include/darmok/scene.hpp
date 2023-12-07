#pragma once

#include <memory>
#include <cstdint>
#include <darmok/app.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <entt/entt.hpp>

namespace darmok
{
    class SceneImpl;

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
        static const uint8_t MaxValue;
        static const Color black;
        static const Color white;
        static const Color red;
        static const Color green;
        static const Color blue;
    };

    struct SpriteVertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
        Color color;
    };

    typedef glm::vec<2, uint16_t> TextureSize;
    struct TextureWithInfo;


    typedef const std::vector<uint16_t> VertexIndexes;

    class Sprite final
    {
    public:
        Sprite(const TextureWithInfo& info, const Color& color = Colors::white);
        Sprite(const bgfx::TextureHandle& texture, const TextureSize& size, const Color& color = Colors::white);
        Sprite(const bgfx::TextureHandle& texture, const std::vector<SpriteVertex>& vertex, const VertexIndexes& idx);
        ~Sprite();

        void render(bgfx::Encoder* encoder, uint8_t textureUnit = 0, uint8_t vertexStream = 0);
        const bgfx::TextureHandle& getTexture() const;
    private:
        bgfx::TextureHandle _texture;
        bgfx::VertexBufferHandle _vertexBuffer = { bgfx::kInvalidHandle };
        bgfx::IndexBufferHandle  _indexBuffer = { bgfx::kInvalidHandle };

        static bgfx::VertexLayout  _layout;
        static bgfx::VertexLayout createVertexLayout();
        void resetVertexBuffer();
        void resetIndexBuffer();
        void setVertices(const std::vector<SpriteVertex>& vertices);
        void setIndices(const VertexIndexes& idx);
    };

    class Program final
    {
    public:
        Program(const bgfx::ProgramHandle& handle);
        const bgfx::ProgramHandle& getHandle();
        void setHandle(const bgfx::ProgramHandle& handle);

    private:
        bgfx::ProgramHandle _handle;
    };

    struct Blend
    {
        uint64_t src;
        uint64_t dst;
    };

    class Camera final
    {
    public:
        Camera(const glm::mat4x4& v = glm::mat4x4());
        
        void setMatrix(const glm::mat4x4& v);
        const glm::mat4x4& getMatrix() const;

    private:
        glm::mat4x4 _matrix;
    };

    typedef uint32_t Entity;
    typedef entt::basic_registry<Entity> Registry;

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

        void render(bgfx::ViewId viewId);

    private:

        Registry& getRegistry();
        const Registry& getRegistry() const;

        std::unique_ptr<SceneImpl> _impl;
    };

    class SceneAppComponent final : public AppComponent
    {
    public:
        SceneAppComponent(const bgfx::ProgramHandle& program);
        Scene& getScene();
        const Scene& getScene() const;
        void update(const InputState& input, bgfx::ViewId viewId, const WindowHandle& window) override;
    private:
        bgfx::ProgramHandle _program;
        Scene _scene;
    };
}

