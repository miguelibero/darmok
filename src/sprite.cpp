#include "scene.hpp"
#include <darmok/sprite.hpp>
#include <bgfx/bgfx.h>


namespace darmok
{
    bgfx::VertexLayout Sprite::createVertexLayout()
    {
        bgfx::VertexLayout layout;
        layout
            .begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
        return layout;
    }

    bgfx::VertexLayout Sprite::_layout = Sprite::createVertexLayout();

    Sprite::Sprite(const bgfx::TextureHandle& texture, const glm::vec2& size, const Color& color)
        : _texture(texture)
        , _textureUnit(0)
        , _vertexStream(0)
    {
        setVertices({
            SpriteVertex{ {0.f, 0.f},       {0.f, 1.f}, color },
            SpriteVertex{ {size.x, 0.f},    {1.f, 1.f}, color },
            SpriteVertex{ size,             {1.f, 0.f}, color },
            SpriteVertex{ {0.f, size.y},    {0.f, 0.f}, color },
        });
        setIndices({ 0, 1, 2, 2, 3, 0 });
    }

    Sprite::Sprite(const bgfx::TextureHandle& texture, std::vector<SpriteVertex>&& vertices, std::vector<VertexIndex>&& idx)
        : _texture(texture)
        , _textureUnit(0)
        , _vertexStream(0)
    {
        setVertices(std::move(vertices));
        setIndices(std::move(idx));
    }

    Sprite::~Sprite()
    {
        resetVertexBuffer();
        resetIndexBuffer();
    }

    void Sprite::resetVertexBuffer()
    {
        if (bgfx::isValid(_vertexBuffer))
        {
            bgfx::destroy(_vertexBuffer);
            _vertexBuffer = { bgfx::kInvalidHandle };
        }
    }

    void Sprite::resetIndexBuffer()
    {
        if (bgfx::isValid(_indexBuffer))
        {
            bgfx::destroy(_indexBuffer);
            _indexBuffer.idx = { bgfx::kInvalidHandle };
        }
    }

    template<typename T>
    static const bgfx::Memory* makeVectorRef(const std::vector<T>& v)
    {
        return bgfx::makeRef(&v.front(), v.size() * sizeof(T));
    }

    void Sprite::setVertices(std::vector<SpriteVertex>&& vertices)
    {
        _vertices = std::move(vertices);
        resetVertexBuffer();
        auto mem = makeVectorRef(_vertices);
        _vertexBuffer = bgfx::createVertexBuffer(mem, _layout);
    }

    void Sprite::setIndices(std::vector<VertexIndex>&& idx)
    {
        _indices = std::move(idx);
        resetIndexBuffer();
        _indexBuffer = bgfx::createIndexBuffer(makeVectorRef(_indices));
    }

    void Sprite::render(bgfx::Encoder& encoder) const
    {
        encoder.setTexture(_textureUnit, SceneImpl::getTexColorUniform(), _texture);
        encoder.setVertexBuffer(_vertexStream, _vertexBuffer);
        encoder.setIndexBuffer(_indexBuffer);
    }

    bool SpriteRenderer::render(bgfx::Encoder& encoder, Entity& entity, Registry& registry)
    {
        auto sprite = registry.try_get<const Sprite>(entity);
        if (sprite == nullptr)
        {
            return false;
        }
        sprite->render(encoder);
        return true;
    }
}