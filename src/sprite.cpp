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
    const bgfx::VertexLayout& SpriteData::getVertexLayout()
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

    std::shared_ptr<SpriteData> SpriteData::fromAtlas(const TextureAtlas& atlas, const TextureAtlasElement& element, const Color& color)
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
        return std::make_shared<SpriteData>(atlas.texture, std::move(vertices), std::vector<VertexIndex>(element.indices));
    }

    std::shared_ptr<SpriteData> SpriteData::fromTexture(const bgfx::TextureHandle& texture, const glm::vec2& size, const Color& color)
    {
        return std::make_shared<SpriteData>(texture, std::vector<SpriteVertex>{
                SpriteVertex{ {0.f, 0.f},       {0.f, 0.f}, color },
                SpriteVertex{ {size.x, 0.f},    {1.f, 0.f}, color },
                SpriteVertex{ size,             {1.f, 1.f}, color },
                SpriteVertex{ {0.f, size.y},    {0.f, 1.f}, color },
            }, std::vector<VertexIndex>{ 0, 1, 2, 2, 3, 0 });
    }

    SpriteData::SpriteData(const bgfx::TextureHandle& texture, std::vector<SpriteVertex>&& vertices, std::vector<VertexIndex>&& indices) noexcept
        : _texture(texture)
        , _vertices(std::move(vertices))
        , _indices(std::move(indices))
        , _vertexBuffer(_vertices, getVertexLayout())
        , _indexBuffer(_indices)
    {
    }

    const bgfx::TextureHandle& SpriteData::getTexture() const
    {
        return _texture;
    }
    const bgfx::VertexBufferHandle& SpriteData::getVertexBuffer() const
    {
        return _vertexBuffer.getHandle();
    }

    const bgfx::IndexBufferHandle& SpriteData::getIndexBuffer() const
    {
        return _indexBuffer.getHandle();
    }

    SpriteAnimation::SpriteAnimation(const std::vector<std::shared_ptr<SpriteData>>& frames, int fps)
        : _frames(frames)
        , _frameDuration(1.f / fps)
        , _currentFrame(0)
        , _timeSinceLastFrame(0.f)
    {
    }

    SpriteAnimation SpriteAnimation::fromAtlas(const TextureAtlas& atlas, const std::string& namePrefix, int fps, const Color& color)
    {
        std::vector<std::shared_ptr<SpriteData>> frames;
        for (auto& elm : atlas.elements)
        {
            if (elm.name.starts_with(namePrefix))
            {
                frames.push_back(SpriteData::fromAtlas(atlas, elm, color));
            }
        }
        return std::move(SpriteAnimation(frames, fps));
    }

    const std::shared_ptr<SpriteData>& SpriteAnimation::getData() const
    {
        if (_currentFrame >= _frames.size())
        {
            return nullptr;
        }
        return _frames[_currentFrame];
    }

    bool SpriteAnimation::empty() const
    {
        return _frames.empty();
    }

    void SpriteAnimation::update(float dt)
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

    Sprite::Sprite(const std::shared_ptr<SpriteData>& data)
        : _data(data)
    {
    }

    Sprite::Sprite(const SpriteAnimation& anim)
        : _anim(anim)
    {
    }

    const SpriteAnimation& Sprite::getAnimation() const
    {
        return _anim;
    }

    SpriteAnimation& Sprite::getAnimation()
    {
        return _anim;
    }

    void Sprite::setAnimation(const SpriteAnimation& anim)
    {
        _data = nullptr;
        _anim = anim;
    }

    const std::shared_ptr<SpriteData>& Sprite::getData() const
    {
        auto& data = _anim.getData();
        if (data != nullptr)
        {
            return data;
        }
        return _data;
    }

    void Sprite::setData(const std::shared_ptr<SpriteData>& data)
    {
        _data = data;
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
        auto view = registry.view<const Sprite>();
        for (auto [entity, sprite] : view.each())
        {
            auto& data = sprite.getData();
            if (data == nullptr)
            {
                continue;
            }
            Transform::bgfxConfig(entity, encoder, registry);
            renderData(*data, encoder, viewId, registry);
        }
    }

    void SpriteRenderer::renderData(const SpriteData& data, bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
    {
        const auto textureUnit = 0;
        const auto vertexStream = 0;

        encoder.setTexture(textureUnit, _texColorUniforn, data.getTexture());
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
        auto sprites = registry.view<Sprite>();
        for (auto [entity, sprite] : sprites.each())
        {
            sprite.getAnimation().update(dt);
        }
    }
}