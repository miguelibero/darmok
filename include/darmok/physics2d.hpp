#pragma once

#include <darmok/scene.hpp>
#include <darmok/color.hpp>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    class BoxCollider2D final
    {
    public:
        BoxCollider2D(const glm::vec2& size = { 1, 1 }, const glm::vec2& offset = {});
        const glm::vec2& getSize() const;
        BoxCollider2D& setSize(const glm::vec2& size);
        const glm::vec2& getOffset() const;
        BoxCollider2D& setOffset(const glm::vec2& offset);
    private:
        glm::vec2 _size;
        glm::vec2 _offset;
    };

    class Physics2DDebugRenderer final : public CameraSceneRenderer
    {
    public:
        Physics2DDebugRenderer(const Color& color = Color::red);
        ~Physics2DDebugRenderer();

        void init(Scene& scene, App& app) override;
    private:
        bgfx::IndexBufferHandle _boxIndexBuffer;
        bgfx::ProgramHandle _program;
        bgfx::UniformHandle _colorUniform;
        bgfx::VertexLayout _vertexLayout;
        Color _color;

        bgfx::ViewId render(EntityRuntimeView& entities, bgfx::Encoder& encoder, bgfx::ViewId viewId) override;
    };
}