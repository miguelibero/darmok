#include "scene.hpp"
#include <darmok/sprite.hpp>
#include <darmok/asset.hpp>
#include <darmok/utils.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    const bgfx::VertexLayout& Sprite::getVertexLayout()
    {
        static bgfx::VertexLayout layout;
        if (layout.m_hash == 0)
        {
            layout
                .begin()
                .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .end();
        }

        return layout;
    }

    std::shared_ptr<Sprite> Sprite::fromAtlas(const TextureAtlas& atlas, const TextureAtlasElement& element, const Color& color)
    {
        std::vector<SpriteVertex> vertices;
        
        glm::vec2 atlasSize(atlas.size);
        for (auto& vtx : element.vertices)
        {
            vertices.push_back({
                vtx.position,
                glm::vec2(vtx.texCoord) / atlasSize,
                color
            });
        }
        return std::make_shared<Sprite>(atlas.texture, std::move(vertices), std::vector<VertexIndex>(element.indices));
    }

    std::vector<std::shared_ptr<Sprite>> Sprite::fromAtlas(const TextureAtlas& atlas, const std::string& namePrefix, const Color& color)
    {
        std::vector<std::shared_ptr<Sprite>> frames;
        for (auto& elm : atlas.elements)
        {
            if (elm.name.starts_with(namePrefix))
            {
                frames.push_back(Sprite::fromAtlas(atlas, elm, color));
            }
        }
        return frames;
    }

    std::shared_ptr<Sprite> Sprite::fromTexture(const std::shared_ptr<Texture>& texture, const glm::vec2& size, const Color& color)
    {
        return std::make_shared<Sprite>(texture, std::vector<SpriteVertex>{
                SpriteVertex{ {0.f, 0.f},       {0.f, 0.f}, color },
                SpriteVertex{ {size.x, 0.f},    {1.f, 0.f}, color },
                SpriteVertex{ size,             {1.f, 1.f}, color },
                SpriteVertex{ {0.f, size.y},    {0.f, 1.f}, color },
            }, std::vector<VertexIndex>{ 0, 1, 2, 2, 3, 0 });
    }

    std::shared_ptr<Material> createMaterial(const std::shared_ptr<Texture>& texture)
    {
        auto prog = AssetContext::get().getEmbeddedProgramLoader()(EmbeddedProgramType::Sprite);
        auto mat = std::make_shared<Material>(prog);
        mat->addTexture(texture);
        return mat;
    }

    Sprite::Sprite(const std::shared_ptr<Texture>& texture, std::vector<SpriteVertex>&& vertices, std::vector<VertexIndex>&& indices) noexcept
        : _material(createMaterial(texture))
        , _vertices(std::move(vertices))
        , _indices(std::move(indices))
        , _vertexBuffer(_vertices, getVertexLayout())
        , _indexBuffer(_indices)
    {
    }

    Sprite::Sprite(const std::shared_ptr<Material>& material, std::vector<SpriteVertex>&& vertices, std::vector<VertexIndex>&& indices) noexcept
        : _material(material)
        , _vertices(std::move(vertices))
        , _indices(std::move(indices))
        , _vertexBuffer(_vertices, getVertexLayout())
        , _indexBuffer(_indices)
    {

    }

    const std::shared_ptr<Material>& Sprite::getMaterial() const
    {
        return _material;
    }
    const bgfx::VertexBufferHandle& Sprite::getVertexBuffer() const
    {
        return _vertexBuffer.getHandle();
    }

    const bgfx::IndexBufferHandle& Sprite::getIndexBuffer() const
    {
        return _indexBuffer.getHandle();
    }

    SpriteAnimationComponent::SpriteAnimationComponent(const std::vector<std::shared_ptr<Sprite>>& frames, int fps)
        : _frames(frames)
        , _frameDuration(1.f / fps)
        , _currentFrame(0)
        , _timeSinceLastFrame(0.f)
    {
    }

    std::shared_ptr<const Sprite> SpriteAnimationComponent::getSprite() const
    {
        if (_currentFrame < 0 || _currentFrame >= _frames.size())
        {
            return nullptr;
        }
        return _frames[_currentFrame];
    }

    std::shared_ptr<Sprite> SpriteAnimationComponent::getSprite()
    {
        if (_currentFrame < 0 || _currentFrame >= _frames.size())
        {
            return nullptr;
        }
        return _frames[_currentFrame];
    }


    bool SpriteAnimationComponent::empty() const
    {
        return _frames.empty();
    }

    void SpriteAnimationComponent::update(float dt)
    {
        _timeSinceLastFrame += dt;
        if (_timeSinceLastFrame > _frameDuration)
        {
            int frames = _timeSinceLastFrame / _frameDuration;
            _timeSinceLastFrame -= frames * _frameDuration;
            if (!empty())
            {
                _currentFrame = (_currentFrame + frames) % _frames.size();
            }
        }
    }

    SpriteComponent::SpriteComponent(const std::shared_ptr<Sprite>& data)
        : _data(data)
    {
    }

    std::shared_ptr<const Sprite> SpriteComponent::getSprite() const
    {
        return _data;
    }

    std::shared_ptr<Sprite> SpriteComponent::getSprite()
    {
        return _data;
    }

    void SpriteRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
    {
        // TODO: sort by Z
        auto sprites = registry.view<const SpriteComponent>();
        for (auto [entity, sprite] : sprites.each())
        {
            auto data = sprite.getSprite();
            if (data == nullptr)
            {
                continue;
            }
            Transform::bgfxConfig(entity, encoder, registry);
            renderSprite(*data, encoder, viewId, registry);
        }
        auto anims = registry.view<const SpriteAnimationComponent>();
        for (auto [entity, anim] : anims.each())
        {
            auto data = anim.getSprite();
            if (data == nullptr)
            {
                continue;
            }
            Transform::bgfxConfig(entity, encoder, registry);
            renderSprite(*data, encoder, viewId, registry);
        }
    }

    void SpriteRenderer::renderSprite(const Sprite& sprite, bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
    {
        const auto vertexStream = 0;
        encoder.setVertexBuffer(vertexStream, sprite.getVertexBuffer());
        encoder.setIndexBuffer(sprite.getIndexBuffer());
        sprite.getMaterial()->submit(encoder, viewId);
    }

    void SpriteAnimationUpdater::updateLogic(float dt, Registry& registry)
    {
        auto anims = registry.view<SpriteAnimationComponent>();
        for (auto [entity, anim] : anims.each())
        {
            anim.update(dt);
        }
    }
}