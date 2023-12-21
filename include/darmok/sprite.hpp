#pragma once

#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
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
        SpriteData(const std::shared_ptr<Texture>& texture, std::vector<SpriteVertex>&& vertex, std::vector<VertexIndex>&& indices) noexcept;
        SpriteData(SpriteData&& other) noexcept = default;
        SpriteData& operator=(SpriteData&& other) noexcept = default;

        static std::shared_ptr<SpriteData> fromAtlas(const TextureAtlas& atlas, const TextureAtlasElement& element, const Color& color = Colors::white);
        static std::shared_ptr<SpriteData> fromTexture(const std::shared_ptr<Texture>& texture, const glm::vec2& size, const Color& color = Colors::white);

        const std::shared_ptr<Texture>& getTexture() const;
        const bgfx::VertexBufferHandle& getVertexBuffer() const;
        const bgfx::IndexBufferHandle& getIndexBuffer() const;
    private:
        SpriteData(const SpriteData& other) = delete;
        SpriteData& operator=(const SpriteData& other) = delete;

        std::shared_ptr<Texture> _texture;
        std::vector<SpriteVertex> _vertices;
        std::vector<VertexIndex> _indices;
        VertexBuffer _vertexBuffer;
        IndexBuffer _indexBuffer;

        static const bgfx::VertexLayout& getVertexLayout();
    };

    class SpriteAnimation final
    {
    public:
        SpriteAnimation(const std::vector<std::shared_ptr<SpriteData>>& frames = {}, int fps = 15);
        SpriteAnimation(const SpriteAnimation& anim) noexcept = default;
        SpriteAnimation& operator=(const SpriteAnimation& anim) noexcept = default;
        SpriteAnimation(SpriteAnimation&& anim) noexcept = default;
        SpriteAnimation& operator=(SpriteAnimation&& anim) noexcept = default;

        static SpriteAnimation fromAtlas(const TextureAtlas& atlas, const std::string& namePrefix, int fps = 15, const Color& color = Colors::white);
        
        bool empty() const;
        const std::shared_ptr<SpriteData>& getData() const;
        void update(float dt);
    private:
        std::vector<std::shared_ptr<SpriteData>> _frames;
        float _frameDuration;
        size_t _currentFrame;
        float _timeSinceLastFrame;
    };

    class Sprite final
    {
    public:
        Sprite(const std::shared_ptr<SpriteData>& data = nullptr);
        Sprite(const SpriteAnimation& anim);
        const std::shared_ptr<SpriteData>& getData() const;
        const SpriteAnimation& getAnimation() const;
        SpriteAnimation& getAnimation();
        void setData(const std::shared_ptr<SpriteData>& data);
        void setAnimation(const SpriteAnimation& data);
    private:
        std::shared_ptr<SpriteData> _data;
        SpriteAnimation _anim;
    };

    class SpriteRenderer final : public ISceneRenderer
    {
    public:
        SpriteRenderer(bgfx::ProgramHandle program = { bgfx::kInvalidHandle });
        ~SpriteRenderer();
        void init(Registry& registry) override;
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry) override;
    private:

        void renderData(const SpriteData& data, bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry);

        bgfx::ProgramHandle _program;
        bgfx::UniformHandle _texColorUniforn;
    };

    class SpriteAnimationUpdater final : public ISceneLogicUpdater
    {
    public:
        SpriteAnimationUpdater() = default;
        void updateLogic(float dt, Registry& registry) override;
    };

}

