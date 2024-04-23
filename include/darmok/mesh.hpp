#pragma once

#include <darmok/scene.hpp>
#include <darmok/data.hpp>
#include <darmok/material.hpp>
#include <darmok/math.hpp>
#include <vector>
#include <bgfx/bgfx.h>

namespace darmok
{
    using VertexIndex = uint16_t;

    class Mesh final
    {
    public:
        Mesh(const bgfx::VertexLayout& layout, Data&& vertices = Data(), const std::shared_ptr<Material>& material = nullptr) noexcept;
        Mesh(const bgfx::VertexLayout& layout, Data&& vertices, Data&& indices, const std::shared_ptr<Material>& material = nullptr) noexcept;
        ~Mesh();

        std::string to_string() const noexcept;

        Mesh(const Mesh& other) noexcept;
        Mesh& operator=(const Mesh& other) noexcept;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        const std::shared_ptr<Material>& getMaterial() const noexcept;
        void setMaterial(const std::shared_ptr<Material>& material) noexcept;

        const Data& getVertexData() const noexcept;
        const Data& getIndexData() const noexcept;

        void bgfxConfig(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const;
        
    private:
        bgfx::VertexLayout _layout;
        Data _vertices;
        Data _indices;
        std::shared_ptr<Material> _material;
        bgfx::VertexBufferHandle _vertexBuffer;
        bgfx::IndexBufferHandle _indexBuffer;

        void destroyHandles();
    };

    struct MeshCreationConfig final
    {
        glm::vec3 scale = glm::vec3(1);
        glm::vec3 offset = glm::vec3(0);
        glm::vec2 textureScale = glm::vec2(1);
        glm::vec2 textureOffset = glm::vec3(0);
        Color color = Colors::white();
    };

    struct MeshData
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;
        std::vector<VertexIndex> indices;
    };

    class Texture;

    struct MeshCreator final
    {
        using Config = MeshCreationConfig;
        Config config;
        bgfx::VertexLayout layout;

        MeshCreator(const bgfx::VertexLayout& layout) noexcept;
        
        std::shared_ptr<Mesh> createMesh(const MeshData& meshData) noexcept;
        std::shared_ptr<Mesh> createCube(const Cube& cube = Cube::standard) noexcept;
        std::shared_ptr<Mesh> createSphere(const Sphere& sphere = Sphere::standard, int lod = 32) noexcept;
        std::shared_ptr<Mesh> createQuad(const Quad& quad = Quad::standard) noexcept;
        std::shared_ptr<Mesh> createLineQuad(const Quad& quad = Quad::standard) noexcept;
        std::shared_ptr<Mesh> createSprite(const std::shared_ptr<Texture>& texture) noexcept;
        std::shared_ptr<Mesh> createRay(const Ray& ray) noexcept;
        std::shared_ptr<Mesh> createLine(const Line& line) noexcept;
        std::shared_ptr<Mesh> createLines(const std::vector<Line>& lines) noexcept;

    private:
        std::shared_ptr<Mesh> createMesh(const MeshData& meshData, const Config& cfg) noexcept;
        std::shared_ptr<Mesh> createQuadMesh(const bgfx::VertexLayout& layout, const MeshData& data, const Quad& quad) noexcept;
    };

    class MeshComponent final
    {
    public:
        MeshComponent(const std::shared_ptr<Mesh>& mesh) noexcept;
        MeshComponent(const std::vector<std::shared_ptr<Mesh>>& meshes = {}) noexcept;
        const std::vector<std::shared_ptr<Mesh>>& getMeshes() const noexcept;
        std::shared_ptr<Mesh> getMesh() const noexcept;
        MeshComponent& setMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes) noexcept;
        MeshComponent& setMesh(const std::shared_ptr<Mesh>& mesh) noexcept;
        MeshComponent& addMesh(const std::shared_ptr<Mesh>& mesh) noexcept;
    private:
        std::vector<std::shared_ptr<Mesh>> _meshes;
    };
}