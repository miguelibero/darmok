#include "scene.hpp"
#include <darmok/sprite.hpp>
#include <darmok/asset.hpp>
#include <darmok/utils.hpp>
#include <bgfx/bgfx.h>

#include <bgfx/embedded_shader.h>
#include "generated/shaders/sprite_vertex.h"
#include "generated/shaders/sprite_fragment.h"

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

    Sprite::Sprite(const std::shared_ptr<Texture>& texture, std::vector<SpriteVertex>&& vertices, std::vector<VertexIndex>&& indices) noexcept
        : _texture(texture)
        , _vertices(std::move(vertices))
        , _indices(std::move(indices))
        , _vertexBuffer(_vertices, getVertexLayout())
        , _indexBuffer(_indices)
    {
    }

    const std::shared_ptr<Texture>& Sprite::getTexture() const
    {
        return _texture;
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

    SpriteRenderer::SpriteRenderer(bgfx::ProgramHandle program)
        : _program(program)
        , _texColorUniforn{ bgfx::kInvalidHandle }
    {
    }

    SpriteRenderer::~SpriteRenderer()
    {
        bgfx::destroy(_program);
        bgfx::destroy(_texColorUniforn);
    }

    static const bgfx::EmbeddedShader _spriteEmbeddedShaders[] =
    {
        BGFX_EMBEDDED_SHADER(sprite_vertex),
        BGFX_EMBEDDED_SHADER(sprite_fragment),
        BGFX_EMBEDDED_SHADER_END()
    };

    void SpriteRenderer::init(Registry& registry)
    {
        if (!isValid(_program))
        {
            auto type = bgfx::getRendererType();
            _program = bgfx::createProgram(
                bgfx::createEmbeddedShader(_spriteEmbeddedShaders, type, "sprite_vertex"),
                bgfx::createEmbeddedShader(_spriteEmbeddedShaders, type, "sprite_fragment"),
                true
            );
        }
        _texColorUniforn = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
    }

    void SpriteRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
    {
        auto sprites = registry.view<const SpriteComponent>();
        for (auto [entity, sprite] : sprites.each())
        {
            auto data = sprite.getSprite();
            if (data == nullptr)
            {
                continue;
            }
            Transform::bgfxConfig(entity, encoder, registry);
            renderData(*data, encoder, viewId, registry);
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
            renderData(*data, encoder, viewId, registry);
        }
    }

    void SpriteRenderer::renderData(const Sprite& data, bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
    {
        const auto textureUnit = 0;
        const auto vertexStream = 0;

        encoder.setTexture(textureUnit, _texColorUniforn, data.getTexture()->getHandle());
        encoder.setVertexBuffer(vertexStream, data.getVertexBuffer());
        encoder.setIndexBuffer(data.getIndexBuffer());

        // TODO: configure state
        uint64_t state = BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
            ;
        encoder.setState(state);
        encoder.submit(viewId, _program);
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