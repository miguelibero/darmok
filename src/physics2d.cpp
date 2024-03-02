
#include <darmok/physics2d.hpp>
#include <darmok/utils.hpp>
#include <darmok/data.hpp>
#include <darmok/vertex.hpp>

#include <bgfx/embedded_shader.h>
#include "generated/shaders/debug_vertex.h"
#include "generated/shaders/debug_fragment.h"

namespace darmok
{
    BoxCollider2D::BoxCollider2D(const glm::vec2& size, const glm::vec2& offset)
        : _size(size)
        , _offset(offset)
    {
    }

    const glm::vec2& BoxCollider2D::getSize() const
    {
        return _size;
    }

    BoxCollider2D& BoxCollider2D::setSize(const glm::vec2& size)
    {
        _size = size;
        return *this;
    }

    const glm::vec2& BoxCollider2D::getOffset() const
    {
        return _offset;
    }

    BoxCollider2D& BoxCollider2D::setOffset(const glm::vec2& offset)
    {
        _offset = offset;
        return *this;
    }

     Physics2DDebugRenderer::Physics2DDebugRenderer(const Color& color)
        : _color(color)
        , _boxIndexBuffer{ bgfx::kInvalidHandle }
        , _program{ bgfx::kInvalidHandle }
        , _colorUniform{ bgfx::kInvalidHandle }
    {
    }

     Physics2DDebugRenderer::~Physics2DDebugRenderer()
     {
         bgfx::destroy(_boxIndexBuffer);
         bgfx::destroy(_program);
         bgfx::destroy(_colorUniform);
     }

    static const bgfx::EmbeddedShader _physics2dEmbeddedShaders[] =
    {
        BGFX_EMBEDDED_SHADER(debug_vertex),
        BGFX_EMBEDDED_SHADER(debug_fragment),
        BGFX_EMBEDDED_SHADER_END()
    };

    static const std::vector<VertexIndex> _physics2dDebugBoxIndices{ 0, 1, 1, 2, 2, 3, 3, 0 };

    void Physics2DDebugRenderer::init(Registry& registry)
    {
        _boxIndexBuffer = bgfx::createIndexBuffer(makeVectorRef(_physics2dDebugBoxIndices));

        auto type = bgfx::getRendererType();
        _program = bgfx::createProgram(
            bgfx::createEmbeddedShader(_physics2dEmbeddedShaders, type, "debug_vertex"),
            bgfx::createEmbeddedShader(_physics2dEmbeddedShaders, type, "debug_fragment"),
            true
        );
        _colorUniform = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);;
        _vertexLayout
            .begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .end();
    }

    void Physics2DDebugRenderer::render(Registry& registry, RenderContext& ctxt)
    {
        uint64_t state = BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_MSAA
            | BGFX_STATE_PT_LINES
            ;

        auto boxes = registry.view<const BoxCollider2D>();
        for (auto [entity, box] : boxes.each())
        {
            Transform::bgfxConfig(entity, ctxt.encoder, registry);

            auto& size = box.getSize();
            auto& offset = box.getOffset();
            std::vector<glm::vec2> vertices{
                { 0, 0 }, { size.x, 0 }, { size.x, size.y }, { 0, size.y }
            };
            for (auto i = 0; i < vertices.size(); i++)
            {
                vertices[i] += offset;
            }
            bgfx::TransientVertexBuffer vertexBuffer;
            bgfx::allocTransientVertexBuffer(&vertexBuffer, 4, _vertexLayout);
            bx::memCopy((glm::vec2*)vertexBuffer.data, &vertices.front(), vertices.size() * sizeof(glm::vec2));

            ctxt.encoder.setUniform(_colorUniform, &_color);
            ctxt.encoder.setVertexBuffer(0, &vertexBuffer);
            ctxt.encoder.setIndexBuffer(_boxIndexBuffer);
            ctxt.encoder.setState(state);
            ctxt.encoder.submit(ctxt.viewId, _program, ctxt.depth);
        }
    }
}