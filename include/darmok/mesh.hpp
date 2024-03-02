#pragma once

#include <darmok/scene.hpp>
#include <darmok/data.hpp>
#include <darmok/material.hpp>

namespace darmok
{
    class Mesh final
    {
    public:
        Mesh(const std::shared_ptr<Material>& material, const bgfx::VertexLayout& layout, Data&& vertices) noexcept;
        Mesh(const std::shared_ptr<Material>& material, const bgfx::VertexLayout& layout, Data&& vertices, Data&& indices) noexcept;
        ~Mesh();

        Mesh(const Mesh& other) noexcept;
        Mesh& operator=(const Mesh& other) noexcept;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        const std::shared_ptr<Material>& getMaterial() const;
        const Data& getVertexData() const;
        const Data& getIndexData() const;
        void render(RenderContext& ctxt, uint8_t vertexStream = 0) const;


        static const std::shared_ptr<Mesh> createCube(const std::shared_ptr<Material>& material);


    private:
        std::shared_ptr<Material> _material;
        bgfx::VertexLayout _layout;

        Data _vertices;
        Data _indices;

        bgfx::VertexBufferHandle _vertexBuffer;
        bgfx::IndexBufferHandle _indexBuffer;

        void destroyHandles();
    };

    class MeshComponent final
    {
    public:
        MeshComponent(const std::shared_ptr<Mesh>& mesh);
        MeshComponent(const std::vector<std::shared_ptr<Mesh>>& meshes = {});
        const std::vector<std::shared_ptr<Mesh>>& getMeshes() const;
        MeshComponent& setMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes);
        MeshComponent& setMesh(const std::shared_ptr<Mesh>& mesh);
    private:
        std::vector<std::shared_ptr<Mesh>> _meshes;
    };

    class MeshRenderer final : public ISceneRenderer
    {
    public:
        void render(Registry& registry, RenderContext& ctxt) override;
    };
}