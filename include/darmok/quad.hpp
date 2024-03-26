#pragma once

#include <darmok/scene.hpp>
#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/material.hpp>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    class QuadComponent final
    {
    public:
        QuadComponent(const std::shared_ptr<Material>& material = nullptr, const glm::vec2& size = { 1, 1 }, const glm::vec2& offset = {}) noexcept;
        ~QuadComponent() noexcept;
        const glm::vec2& getSize() const noexcept;
        QuadComponent& setSize(const glm::vec2& size) noexcept;
        const glm::vec2& getOffset() const noexcept;
        QuadComponent& setOffset(const glm::vec2& offset) noexcept;
        const std::shared_ptr<Material>& getMaterial() const noexcept;
        QuadComponent& setMaterial(const std::shared_ptr<Material>& material) noexcept;

        void update() noexcept;
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId, uint8_t vertexStream = 0) const;
    private:
        std::shared_ptr<Material> _material;
        glm::vec2 _size;
        glm::vec2 _offset;
        bool _changed;
        bgfx::DynamicVertexBufferHandle _vertexBuffer;
        Data _vertices;
    };

    class QuadUpdater final : public ISceneLogicUpdater
    {
    public:
        void init(Scene& scene, App& app) noexcept override;
        void update(float deltaTime) noexcept override;
    private:
        OptionalRef<Scene> _scene;
    };


    class QuadRenderer final : public CameraSceneRenderer
    {
    public:
        QuadRenderer() noexcept;
        void init(Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
    private:
        bgfx::IndexBufferHandle _lineIndexBuffer;
        bgfx::IndexBufferHandle _triIndexBuffer;

        bgfx::IndexBufferHandle getIndexBuffer(MaterialPrimitiveType mode) const;

        bgfx::ViewId render(const Camera& cam, bgfx::Encoder& encoder, bgfx::ViewId viewId) noexcept override;
    };
}