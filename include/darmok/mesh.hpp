#pragma once

#include <darmok/scene.hpp>
#include <darmok/data.hpp>
#include <darmok/material.hpp>

namespace darmok
{
    class Mesh final
    {
    public:
        Mesh(const std::shared_ptr<Material>& material, Data&& vertices) noexcept;
        Mesh(const std::shared_ptr<Material>& material, Data&& vertices, Data&& indices) noexcept;
        ~Mesh();

        Mesh(const Mesh& other) noexcept;
        Mesh& operator=(const Mesh& other) noexcept;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        const std::shared_ptr<Material>& getMaterial() const noexcept;

        const Data& getVertexData() const noexcept;
        const Data& getIndexData() const noexcept;

        void bgfxConfig(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const;
        static const std::shared_ptr<Mesh> createCube(const std::shared_ptr<Material>& material) noexcept;
        static const std::shared_ptr<Mesh> createQuad(const std::shared_ptr<Material>& material, const glm::uvec2& size = {1, 1}) noexcept;
        static const std::shared_ptr<Mesh> createSphere(const std::shared_ptr<Material>& material, float radius = 1.F, float lod = 1.F) noexcept;
        static const std::shared_ptr<Mesh> createSprite(const std::shared_ptr<Texture>& texture, const ProgramDefinition& progDef, float scale = 1.f, const Color& color = Colors::white) noexcept;

    private:
        std::shared_ptr<Material> _material;

        Data _vertices;
        Data _indices;

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