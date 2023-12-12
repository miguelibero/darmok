#pragma once

#include <darmok/scene.hpp>
#include <glm/glm.hpp>

namespace darmok
{
    struct SpriteVertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
        Color color;
    };

    class Sprite final
    {
    public:
        Sprite(const bgfx::TextureHandle& texture, const glm::vec2& size, const Color& color = Colors::white);
        Sprite(const bgfx::TextureHandle& texture, std::vector<SpriteVertex>&& vertex, std::vector<VertexIndex>&& indices);
        ~Sprite();

        void render(bgfx::Encoder& encoder) const;
    private:
        bgfx::TextureHandle _texture;
        bgfx::VertexBufferHandle _vertexBuffer = { bgfx::kInvalidHandle };
        bgfx::IndexBufferHandle  _indexBuffer = { bgfx::kInvalidHandle };
        std::vector<SpriteVertex> _vertices;
        std::vector<VertexIndex> _indices;
        uint8_t _textureUnit;
        uint8_t _vertexStream;

        static bgfx::VertexLayout  _layout;
        static bgfx::VertexLayout createVertexLayout();
        void resetVertexBuffer();
        void resetIndexBuffer();
        void setVertices(std::vector<SpriteVertex>&& vertices);
        void setIndices(std::vector<VertexIndex>&& idx);
    };

    class SpriteRenderer final : public IEntityRenderer
    {
    public:
        bool render(bgfx::Encoder& encoder, Entity& entity, Registry& registry) override;
    };

}

