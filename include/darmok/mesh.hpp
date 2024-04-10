#pragma once

#include <darmok/scene.hpp>
#include <darmok/data.hpp>
#include <darmok/material.hpp>

namespace darmok
{
    using VertexIndex = uint16_t;

    class Mesh final
    {
    public:
        Mesh(const bgfx::VertexLayout& layout, Data&& vertices, const std::shared_ptr<Material>& material = nullptr) noexcept;
        Mesh(const bgfx::VertexLayout& layout, Data&& vertices, Data&& indices, const std::shared_ptr<Material>& material = nullptr) noexcept;
        ~Mesh();

        Mesh(const Mesh& other) noexcept;
        Mesh& operator=(const Mesh& other) noexcept;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        const std::shared_ptr<Material>& getMaterial() const noexcept;
        void setMaterial(const std::shared_ptr<Material>& material) noexcept;

        const Data& getVertexData() const noexcept;
        const Data& getIndexData() const noexcept;

        void bgfxConfig(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const;
        static const std::shared_ptr<Mesh> createCube(const bgfx::VertexLayout& layout) noexcept;
        static const std::shared_ptr<Mesh> createQuad(const bgfx::VertexLayout& layout, const glm::uvec2& size = {1, 1}) noexcept;
        static const std::shared_ptr<Mesh> createLineQuad(const bgfx::VertexLayout& layout, const glm::uvec2& size = { 1, 1 }) noexcept;
        static const std::shared_ptr<Mesh> createSphere(const bgfx::VertexLayout& layout, float radius = 1.F, int lod = 32) noexcept;
        static const std::shared_ptr<Mesh> createSprite(const std::shared_ptr<Texture>& texture, const bgfx::VertexLayout& layout, float scale = 1.f, const Color& color = Colors::white) noexcept;

    private:
        bgfx::VertexLayout _layout;
        Data _vertices;
        Data _indices;
        std::shared_ptr<Material> _material;
        bgfx::VertexBufferHandle _vertexBuffer;
        bgfx::IndexBufferHandle _indexBuffer;

        void destroyHandles();
    };

    class MeshComponent final
    {
    public:
        MeshComponent(const std::shared_ptr<Mesh>& mesh) noexcept;
        MeshComponent(const std::vector<std::shared_ptr<Mesh>>& meshes = {}) noexcept;
        const std::vector<std::shared_ptr<Mesh>>& getMeshes() const noexcept;
        MeshComponent& setMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes) noexcept;
        MeshComponent& setMesh(const std::shared_ptr<Mesh>& mesh) noexcept;
        MeshComponent& addMesh(const std::shared_ptr<Mesh>& mesh) noexcept;
    private:
        std::vector<std::shared_ptr<Mesh>> _meshes;
    };
}