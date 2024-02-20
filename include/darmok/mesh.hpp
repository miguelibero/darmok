#pragma once

#include <darmok/scene.hpp>
#include <darmok/data.hpp>
#include <darmok/material.hpp>

namespace darmok
{
    class Mesh final
    {
    public:
        Mesh(const std::shared_ptr<Material>& material, std::vector<float>&& vertices, bgfx::VertexLayout layout, std::vector<VertexIndex>&& indices) noexcept;

        const std::shared_ptr<Material>& getMaterial() const;
        const bgfx::VertexBufferHandle& getVertexBuffer() const;
        const bgfx::IndexBufferHandle& getIndexBuffer() const;
    private:
        std::shared_ptr<Material> _material;
        std::vector<float> _vertices;
        std::vector<VertexIndex> _indices;
        VertexBuffer _vertexBuffer;
        IndexBuffer _indexBuffer;

        Mesh(const Mesh& other) noexcept = delete;
        Mesh& operator=(const Mesh& other) noexcept = delete;
    };

    class MeshComponent final
    {
    public:
        MeshComponent(const std::vector<std::shared_ptr<Mesh>>& meshes = {});
        const std::vector<std::shared_ptr<Mesh>>& getMeshes() const;
        void setMeshes(const std::vector<std::shared_ptr<Mesh>>& data);
    private:
        std::vector<std::shared_ptr<Mesh>> _meshes;
    };

    class MeshRenderer final : public ISceneRenderer
    {
    public:
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry) override;
    private:

        void renderMesh(const Mesh& mesh, bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry);
    };
}