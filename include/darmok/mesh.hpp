#pragma once

#include <vector>
#include <memory>

#include <darmok/color.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/vertex_fwd.hpp>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

namespace darmok
{
    class DataView;

    class Mesh final
    {
    public:
        Mesh(const bgfx::VertexLayout& layout, const DataView& vertices, bool dynamic = false) noexcept;
        Mesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, bool dynamic = false) noexcept;
        ~Mesh() noexcept;
        Mesh(const Mesh& other) = delete;
        Mesh& operator=(const Mesh& other) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        void updateVertices(const DataView& data, size_t offset = 0);
        void updateIndices(const DataView& data, size_t offset = 0);
        
        std::string to_string() const noexcept;
        void render(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const;
        const bgfx::VertexLayout& getVertexLayout() const noexcept;
    private:
        bgfx::VertexLayout _layout;
        uint16_t _vertexBuffer;
        uint16_t _indexBuffer;
        size_t _vertexSize;
        size_t _indexSize;
        bool _dynamic;
    };

    struct MeshCreationConfig final
    {
        glm::vec3 scale = glm::vec3(1);
        glm::vec3 offset = glm::vec3(0);
        glm::vec2 textureScale = glm::vec2(1);
        glm::vec2 textureOffset = glm::vec3(0);
        Color color = Colors::white();
        bool dynamic = false;
    };

    struct MeshData
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;
        std::vector<VertexIndex> indices;
    };

    class Texture;
    struct Cube;
    struct Sphere;
    struct Quad;
    struct Ray;
    struct Line;

    struct MeshCreator final
    {
        using Config = MeshCreationConfig;
        Config config;
        bgfx::VertexLayout layout;

        MeshCreator(const bgfx::VertexLayout& layout) noexcept;
        
        std::shared_ptr<Mesh> createMesh(const MeshData& meshData) noexcept;
        std::shared_ptr<Mesh> createCube() noexcept;
        std::shared_ptr<Mesh> createCube(const Cube& cube) noexcept;
        std::shared_ptr<Mesh> createSphere(const Sphere& sphere, int lod = 32) noexcept;
        std::shared_ptr<Mesh> createSphere(int lod = 32) noexcept;
        std::shared_ptr<Mesh> createQuad() noexcept;
        std::shared_ptr<Mesh> createQuad(const Quad& quad) noexcept;
        std::shared_ptr<Mesh> createLineQuad() noexcept;
        std::shared_ptr<Mesh> createLineQuad(const Quad& quad) noexcept;
        std::shared_ptr<Mesh> createSprite(const std::shared_ptr<Texture>& texture) noexcept;
        std::shared_ptr<Mesh> createRay(const Ray& ray) noexcept;
        std::shared_ptr<Mesh> createLine(const Line& line) noexcept;
        std::shared_ptr<Mesh> createLines(const std::vector<Line>& lines) noexcept;

    private:
        std::shared_ptr<Mesh> createMesh(const MeshData& meshData, const Config& cfg) noexcept;
        std::shared_ptr<Mesh> createQuadMesh(const bgfx::VertexLayout& layout, const MeshData& data, const Quad& quad) noexcept;
    };

    class Material;

    class Renderable final
    {
    public:
        Renderable(const std::shared_ptr<Mesh>& mesh = nullptr, const std::shared_ptr<Material>& material = nullptr) noexcept;
        Renderable(const std::shared_ptr<Material>& material) noexcept;
        std::shared_ptr<Mesh> getMesh() const noexcept;
        Renderable& setMesh(const std::shared_ptr<Mesh>& mesh) noexcept;
        std::shared_ptr<Material> getMaterial() const noexcept;
        Renderable& setMaterial(const std::shared_ptr<Material>& material) noexcept;
    private:
        std::shared_ptr<Mesh> _mesh;
        std::shared_ptr<Material> _material;
    };
}