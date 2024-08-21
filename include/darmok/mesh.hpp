#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <variant>

#include <darmok/export.h>
#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/vertex_fwd.hpp>
#include <darmok/mesh_fwd.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <darmok/glm.hpp>

namespace darmok
{
    struct DARMOK_EXPORT MeshConfig final
    {
        bool index32 = false;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(index32);
        }

        uint16_t getFlags() const noexcept;
        size_t getIndexSize() const noexcept;
    };

    struct DARMOK_EXPORT MeshRenderConfig final
    {
        uint8_t vertexStream = 0;
        uint32_t startVertex = 0;
        uint32_t numVertices = 0;
        uint32_t startIndex = 0;
        uint32_t numIndices = 0;

        void fix(uint32_t maxVertices, uint32_t maxIndices) noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IMesh
    {
    public:
        using Config = MeshConfig;
        using RenderConfig = MeshRenderConfig;

        virtual ~IMesh() = default;
         virtual [[nodiscard]] std::string toString() const noexcept = 0;
         virtual bool render(bgfx::Encoder& encoder, RenderConfig config = {}) const = 0;
         virtual [[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept = 0;

         static [[nodiscard]] std::unique_ptr<IMesh> create(MeshType type, const bgfx::VertexLayout& layout, DataView vertices, Config config = {});
         static [[nodiscard]] std::unique_ptr<IMesh> create(MeshType type, const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config = {});
    };

    class DARMOK_EXPORT Mesh final : public IMesh
    {
    public:
        using Config = MeshConfig;
         Mesh(const bgfx::VertexLayout& layout, DataView vertices, Config config = {}) noexcept;
         Mesh(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config = {}) noexcept;
         ~Mesh() noexcept;
         Mesh(Mesh&& other) noexcept;
         Mesh& operator=(Mesh&& other) noexcept;

        Mesh(const Mesh& other) = delete;
        Mesh& operator=(const Mesh& other) = delete;

         [[nodiscard]] bgfx::VertexBufferHandle getVertexHandle() const noexcept;
         [[nodiscard]] bgfx::IndexBufferHandle getIndexHandle() const noexcept;

         [[nodiscard]] std::string toString() const noexcept override;
         bool render(bgfx::Encoder& encoder, RenderConfig config = {}) const noexcept override;
         [[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept override;
    private:
        bgfx::VertexLayout _layout;
        bgfx::VertexBufferHandle _vertexBuffer;
        bgfx::IndexBufferHandle _indexBuffer;
        size_t _vertNum;
        size_t _idxNum;
    };

    class DARMOK_EXPORT DynamicMesh final : public IMesh
    {
    public:
        using Config = MeshConfig;
        DynamicMesh(const bgfx::VertexLayout& layout, DataView vertices = DataView(), Config config = {}) noexcept;
        DynamicMesh(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config = {}) noexcept;
        ~DynamicMesh() noexcept;
        DynamicMesh(DynamicMesh&& other) noexcept;
        DynamicMesh& operator=(DynamicMesh&& other) noexcept;

        DynamicMesh(const DynamicMesh& other) = delete;
        Mesh& operator=(const DynamicMesh& other) = delete;

        [[nodiscard]] bgfx::DynamicVertexBufferHandle getVertexHandle() const noexcept;
        [[nodiscard]] bgfx::DynamicIndexBufferHandle getIndexHandle() const noexcept;

        bool empty() const noexcept;

        void updateVertices(DataView data, uint32_t offset = 0) noexcept;
        void updateIndices(DataView data, uint32_t offset = 0) noexcept;

        [[nodiscard]] std::string toString() const noexcept override;
        bool render(bgfx::Encoder& encoder, RenderConfig config = {}) const noexcept override;
        [[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept override;
    private:
        bgfx::VertexLayout _layout;
        bgfx::DynamicVertexBufferHandle _vertexBuffer;
        bgfx::DynamicIndexBufferHandle _indexBuffer;
        size_t _idxSize;
        size_t _vertNum;
        size_t _idxNum;
    };

    class DARMOK_EXPORT TransientMesh final : public IMesh
    {
    public:
         TransientMesh(const bgfx::VertexLayout& layout, DataView vertices, bool index32 = false);
         TransientMesh(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, bool index32 = false);
         TransientMesh(TransientMesh&& other) noexcept;
         TransientMesh& operator=(TransientMesh&& other) noexcept;

         TransientMesh(const Mesh& other) = delete;
         TransientMesh& operator=(const TransientMesh& other) = delete;

         [[nodiscard]] std::string toString() const noexcept override;
         bool render(bgfx::Encoder& encoder, RenderConfig config = {}) const noexcept override;
         [[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept override;
    private:
        bgfx::VertexLayout _layout;
        bgfx::TransientVertexBuffer _vertexBuffer;
        bgfx::TransientIndexBuffer _indexBuffer;
        uint32_t _vertNum;
        uint32_t _idxNum;
    };

    struct DARMOK_EXPORT MeshDataVertex final
    {
        glm::vec3 position;
        glm::vec2 texCoord;
        glm::vec3 normal = glm::vec3(0, 1, 0);
        glm::vec3 tangent = glm::vec3(0, 0, 0);
        Color color = Colors::white();
    };

    class Texture;
    struct Cube;
    struct Sphere;
    struct Rectangle;
    struct Capsule;
    struct Ray;
    struct Line;
    struct Triangle;
    struct Polygon;
    struct Frustum;

    enum class RectangleMeshType
    {
        Full,
        Outline
    };

    enum class LineMeshType
    {
        Line,
        Diamond
    };

    struct DARMOK_EXPORT MeshData final
    {
        using Vertex = MeshDataVertex;
        using Index = VertexIndex;

        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        MeshType type = MeshType::Static;

        MeshData() = default;
        MeshData(const Cube& Cube, RectangleMeshType type = RectangleMeshType::Full) noexcept;
        MeshData(const Sphere& sphere, unsigned int lod = 32) noexcept;
        MeshData(const Capsule& capsule, unsigned int lod = 32) noexcept;
        MeshData(const Rectangle& rect, RectangleMeshType type = RectangleMeshType::Full) noexcept;
        MeshData(const Ray& ray) noexcept;
        MeshData(const Line& line, LineMeshType type = LineMeshType::Line) noexcept;
        MeshData(const Triangle& tri) noexcept;
        MeshData(const Polygon& poly) noexcept;
        MeshData(const Frustum& frust, RectangleMeshType type = RectangleMeshType::Outline) noexcept;

        MeshData& operator+=(const MeshData& other) noexcept;
        MeshData operator+(const MeshData& other) const noexcept;

        MeshData& operator*=(const glm::mat4& trans) noexcept;
        MeshData& operator*=(const glm::mat2& trans) noexcept;

        MeshData& operator+=(const glm::vec3& offset) noexcept;
        MeshData& operator*=(const glm::vec3& scale) noexcept;
        MeshData& operator*=(const glm::quat& rot) noexcept;
        MeshData& operator*=(const Color& color) noexcept;
        MeshData& operator*=(const glm::uvec2& textureScale) noexcept;
        MeshData& operator+=(const glm::uvec2& textureOffset) noexcept;

        MeshData& shiftIndices(Index offset) noexcept;
        MeshData& calcNormals() noexcept;
        MeshData& calcTangents() noexcept;

        using Face = std::array<Index, 3>;
        [[nodiscard]] std::vector<Face> getFaces() const noexcept;

        bool empty() const noexcept;
        void clear() noexcept;

        [[nodiscard]] void exportData(const bgfx::VertexLayout& vertexLayout, Data& vertexData, Data& indexData) const noexcept;
        [[nodiscard]] std::unique_ptr<IMesh> createMesh(const bgfx::VertexLayout& vertexLayout, const IMesh::Config& config = {}) const;
        [[nodiscard]] static const bgfx::VertexLayout& getDefaultVertexLayout() noexcept;

        MeshData& convertQuadIndicesToLine() noexcept;
    private:
        static const std::vector<Index> _cuboidTriangleIndices;
    };
}