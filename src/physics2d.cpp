
#include <darmok/physics2d.hpp>
#include <darmok/utils.hpp>
#include <bgfx/embedded_shader.h>
#include "generated/shaders/debug_vertex.h"
#include "generated/shaders/debug_fragment.h"

namespace darmok
{
    BoxCollider2D::BoxCollider2D(const glm::vec2& size)
        : _size(size)
    {
    }

    const glm::vec2& BoxCollider2D::getSize() const
    {
        return _size;
    }

    void BoxCollider2D::setSize(const glm::vec2& size)
    {
        _size = size;
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

    void Physics2DDebugRenderer::init()
    {
        _boxIndexBuffer = bgfx::createIndexBuffer(makeVectorRef(_physics2dDebugBoxIndices));

        auto type = bgfx::getRendererType();
        _program = bgfx::createProgram(
            bgfx::createEmbeddedShader(_physics2dEmbeddedShaders, type, "debug_vertex"),
            bgfx::createEmbeddedShader(_physics2dEmbeddedShaders, type, "debug_fragment"),
            true
        );
        _colorUniform = bgfx::createUniform("s_color", bgfx::UniformType::Vec4);;
        _vertexLayout
            .begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .end();
    }

    void Physics2DDebugRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
    {
        uint64_t state = BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_MSAA
            | BGFX_STATE_PT_LINES
            ;

        auto boxes = registry.view<const BoxCollider2D>();
        for (auto [entity, box] : boxes.each())
        {
            Transform::bgfxConfig(entity, encoder, registry);

            auto& size = box.getSize();
            std::vector<glm::vec2> vertices{
                { 0, 0 }, { size.x, 0 }, { size.x, size.y }, { 0, size.y }
            };
            bgfx::TransientVertexBuffer vertexBuffer;
            bgfx::allocTransientVertexBuffer(&vertexBuffer, 4, _vertexLayout);
            bx::memCopy((glm::vec2*)vertexBuffer.data, &vertices.front(), vertices.size() * sizeof(glm::vec2));

            encoder.setUniform(_colorUniform, &_color);
            encoder.setVertexBuffer(0, &vertexBuffer);
            encoder.setIndexBuffer(_boxIndexBuffer);
            encoder.setState(state);
            encoder.submit(viewId, _program);
        }
    }
}