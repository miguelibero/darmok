#pragma once

#include <darmok/scene.hpp>
#include <darmok/texture.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <darmok/material.hpp>
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

    class Sprite final
    {
    public:
        Sprite(const std::shared_ptr<Texture>& texture, std::vector<SpriteVertex>&& vertices, std::vector<VertexIndex>&& indices) noexcept;
        Sprite(const std::shared_ptr<Material>& material, std::vector<SpriteVertex>&& vertices, std::vector<VertexIndex>&& indices) noexcept;
        Sprite(Sprite&& other) noexcept = default;
        Sprite& operator=(Sprite&& other) noexcept = default;

        static std::shared_ptr<Sprite> fromAtlas(const TextureAtlas& atlas, const TextureAtlasElement& element, const Color& color = Colors::white);
        static std::vector<std::shared_ptr<Sprite>> fromAtlas(const TextureAtlas& atlas, const std::string& namePrefix, const Color& color = Colors::white);
        static std::shared_ptr<Sprite> fromTexture(const std::shared_ptr<Texture>& texture, const glm::vec2& size, const Color& color = Colors::white);

        const std::shared_ptr<Material>& getMaterial() const;
        const bgfx::VertexBufferHandle& getVertexBuffer() const;
        const bgfx::IndexBufferHandle& getIndexBuffer() const;
    private:
        Sprite(const Sprite& other) = delete;
        Sprite& operator=(const Sprite& other) = delete;

        std::shared_ptr<Material> _material;
        std::vector<SpriteVertex> _vertices;
        std::vector<VertexIndex> _indices;
        VertexBuffer _vertexBuffer;
        IndexBuffer _indexBuffer;

        static const bgfx::VertexLayout& getVertexLayout();
    };

    class BX_NO_VTABLE ISpriteProvider
    {
    public:
        virtual ~ISpriteProvider() = default;
        virtual std::shared_ptr<const Sprite> getSprite() const = 0;
        virtual std::shared_ptr<Sprite> getSprite() = 0;
    };

    class SpriteAnimationComponent final : ISpriteProvider
    {
    public:
        SpriteAnimationComponent(const std::vector<std::shared_ptr<Sprite>>& frames = {}, int fps = 15);
        SpriteAnimationComponent(const SpriteAnimationComponent& anim) noexcept = default;
        SpriteAnimationComponent& operator=(const SpriteAnimationComponent& anim) noexcept = default;
        SpriteAnimationComponent(SpriteAnimationComponent&& anim) noexcept = default;
        SpriteAnimationComponent& operator=(SpriteAnimationComponent&& anim) noexcept = default;

        bool empty() const;
        void update(float dt);

        std::shared_ptr<const Sprite> getSprite() const override;
        std::shared_ptr<Sprite> getSprite()  override;

    private:
        std::vector<std::shared_ptr<Sprite>> _frames;
        float _frameDuration;
        size_t _currentFrame;
        float _timeSinceLastFrame;
    };

    class SpriteComponent final : ISpriteProvider
    {
    public:
        SpriteComponent(const std::shared_ptr<Sprite>& data = nullptr);
        std::shared_ptr<const Sprite> getSprite() const override;
        std::shared_ptr<Sprite> getSprite()  override;
    private:
        std::shared_ptr<Sprite> _data;
    };

    class SpriteRenderer final : public ISceneRenderer
    {
    public:
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry) override;
    private:
        void renderSprite(const Sprite& sprite, bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry);
    };

    class SpriteAnimationUpdater final : public ISceneLogicUpdater
    {
    public:
        SpriteAnimationUpdater() = default;
        void updateLogic(float dt, Registry& registry) override;
    };

}

