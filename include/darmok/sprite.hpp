#pragma once

#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <glm/glm.hpp>
#include <memory>

namespace darmok
{
    struct SpriteVertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
        Color color;
    };

    class TextureAtlas;
    class TextureAtlasElement;

    class SpriteData final
    {
    public:
        SpriteData(const bgfx::TextureHandle& texture, std::vector<SpriteVertex>&& vertex, std::vector<VertexIndex>&& indices) noexcept;
        ~SpriteData() noexcept;
        SpriteData(SpriteData&& other) noexcept;
        SpriteData& operator=(SpriteData&& other) noexcept;

        static std::shared_ptr<SpriteData> fromAtlas(const TextureAtlas& atlas, const TextureAtlasElement& element, const Color& color = Colors::white);
        static std::shared_ptr<SpriteData> fromTexture(const bgfx::TextureHandle& texture, const glm::vec2& size, const Color& color = Colors::white);

        const bgfx::TextureHandle& getTexture() const;
        const bgfx::VertexBufferHandle& getVertexBuffer() const;
        const bgfx::IndexBufferHandle& getIndexBuffer() const;
    private:
        SpriteData(const SpriteData& other) = delete;
        SpriteData& operator=(const SpriteData& other) = delete;
        void resetBuffers();

        bgfx::TextureHandle _texture;
        bgfx::VertexBufferHandle _vertexBuffer = { bgfx::kInvalidHandle };
        bgfx::IndexBufferHandle  _indexBuffer = { bgfx::kInvalidHandle };
        std::vector<SpriteVertex> _vertices;
        std::vector<VertexIndex> _indices;

        static const bgfx::VertexLayout& getVertexLayout();
    };

    class Sprite final
    {
    public:
        Sprite(const std::shared_ptr<SpriteData>& data = nullptr);
        const std::shared_ptr<SpriteData>& getData() const;
        void setData(const std::shared_ptr<SpriteData>& data);
    private:
        std::shared_ptr<SpriteData> _data;
    };

    class SpriteRenderer final : public ISceneRenderer
    {
    public:
        SpriteRenderer(bgfx::ProgramHandle program = { bgfx::kInvalidHandle });
        ~SpriteRenderer();
        void init();
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry) override;
    private:
        bgfx::ProgramHandle _program;
        bgfx::UniformHandle _texColorUniforn;
    };

}

