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
                glm::vec2(vtx.position.x, element.originalSize.y - vtx.position.y),
                glm::vec2(vtx.texCoord) / atlasSize,
                color
            });
        }
        // TODO: check rotated
        return std::make_shared<SpriteData>(atlas.texture, std::move(vertices), std::vector<VertexIndex>(element.indices));
    }

    std::shared_ptr<SpriteData> SpriteData::fromTexture(const bgfx::TextureHandle& texture, const glm::vec2& size, const Color& color)
    {
        return std::make_shared<SpriteData>(texture, std::vector<SpriteVertex>{
                SpriteVertex{ {0.f, 0.f},       {0.f, 1.f}, color },
                SpriteVertex{ {size.x, 0.f},    {1.f, 1.f}, color },
                SpriteVertex{ size,             {1.f, 0.f}, color },
                SpriteVertex{ {0.f, size.y},    {0.f, 0.f}, color },
            }, std::vector<VertexIndex>{ 0, 1, 2, 2, 3, 0 });
    }

    SpriteData::SpriteData(const bgfx::TextureHandle& texture, std::vector<SpriteVertex>&& vertices, std::vector<VertexIndex>&& indices) noexcept
        : _texture(texture)
        , _vertices(std::move(vertices))
        , _indices(std::move(indices))

    {
        _vertexBuffer = bgfx::createVertexBuffer(makeVectorRef(_vertices), getVertexLayout());
        _indexBuffer = bgfx::createIndexBuffer(makeVectorRef(_indices));
    }

    SpriteData::~SpriteData() noexcept
    {
        resetBuffers();
    }

    void SpriteData::resetBuffers()
    {
        bgfx::destroy(_vertexBuffer);
        bgfx::destroy(_indexBuffer);
    }

    SpriteData::SpriteData(SpriteData&& other) noexcept
        : _texture(other._texture)
        , _vertices(std::move(other._vertices))
        , _indices(std::move(other._indices))
        , _vertexBuffer(other._vertexBuffer)
        , _indexBuffer(other._indexBuffer)
    {
        other._vertexBuffer = { bgfx::kInvalidHandle };
        other._indexBuffer = { bgfx::kInvalidHandle };
    }
    SpriteData& SpriteData::operator=(SpriteData&& other) noexcept
    {
        resetBuffers();
        _vertices = std::move(other._vertices);
        _indices = std::move(other._indices);
        _vertexBuffer = other._vertexBuffer;
        _indexBuffer = other._indexBuffer;

        other._vertexBuffer = { bgfx::kInvalidHandle };
        other._indexBuffer = { bgfx::kInvalidHandle };
        return *this;
    }

    const bgfx::TextureHandle& SpriteData::getTexture() const
    {
        return _texture;
    }
    const bgfx::VertexBufferHandle& SpriteData::getVertexBuffer() const
    {
        return _vertexBuffer;
    }

    const bgfx::IndexBufferHandle& SpriteData::getIndexBuffer() const
    {
        return _indexBuffer;
    }

    Sprite::Sprite(const std::shared_ptr<SpriteData>& data)
        : _data(data)
    {
    }

    const SpriteData& Sprite::getData() const
    {
        return *_data;
    }

    SpriteRenderer::SpriteRenderer(bgfx::ProgramHandle program)
        : _program(program)
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

    void SpriteRenderer::init()
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
        auto sprites = registry.view<const Sprite>();
        const auto textureUnit = 0;
        const auto vertexStream = 0;
        for (auto [entity, sprite] : sprites.each())
        {
            Transform::bgfxConfig(entity, encoder, registry);
            auto& data = sprite.getData();
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
    }
}