#pragma once

#include <darmok/scene.hpp>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    class BoxCollider2D final
    {
    public:
        BoxCollider2D(const glm::vec2& size = {});
        const glm::vec2& getSize() const;
        void setSize(const glm::vec2& size);
    private:
        glm::vec2 _size;
    };

    class Physics2DDebugRenderer final : public ISceneRenderer
    {
    public:
        Physics2DDebugRenderer(const Color& color = Colors::red);
        ~Physics2DDebugRenderer();

        void init() override;
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry) override;
    private:
        bgfx::IndexBufferHandle _boxIndexBuffer;
        bgfx::ProgramHandle _program;
        bgfx::UniformHandle _colorUniform;
        bgfx::VertexLayout _vertexLayout;
        Color _color;
    };
}