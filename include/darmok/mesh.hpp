#pragma once

#include <darmok/export.h>
#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/vertex_fwd.hpp>
#include <darmok/glm.hpp>
#include <darmok/varying.hpp>
#include <darmok/loader.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/mesh.pb.h>

#include <vector>
#include <memory>
#include <optional>
#include <variant>
#include <limits>

#include <bgfx/bgfx.h>
#include <bx/bx.h>

namespace darmok
{
    struct DARMOK_EXPORT MeshConfig final
    {
        bool index32 = false;

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

    class DARMOK_EXPORT BX_NO_VTABLE IMeshDefinitionLoader : public ILoader<protobuf::Mesh>
    {
    };

    using MeshDefinitionLoader = ProtobufLoader<IMeshDefinitionLoader>;

    class DARMOK_EXPORT BX_NO_VTABLE IMesh
    {
    public:
        using Config = MeshConfig;
        using RenderConfig = MeshRenderConfig;
        using Definition = protobuf::Mesh;
        using MeshType = protobuf::MeshType;
        using Source = protobuf::MeshSource;

        virtual ~IMesh() = default;
        [[nodiscard]] virtual std::string toString() const noexcept = 0;
        virtual bool render(bgfx::Encoder& encoder, RenderConfig config = {}) const = 0;
        [[nodiscard]] virtual const bgfx::VertexLayout& getVertexLayout() const noexcept = 0;

        [[nodiscard]] static std::unique_ptr<IMesh> create(const Definition& def);
        [[nodiscard]] static std::unique_ptr<IMesh> create(MeshType::Enum type, const bgfx::VertexLayout& layout, DataView vertices, Config config = {});
        [[nodiscard]] static std::unique_ptr<IMesh> create(MeshType::Enum type, const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config = {});
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
        glm::vec3 position{};
        glm::vec2 texCoord{};
        glm::vec3 normal = glm::vec3(0, 1, 0);
        glm::vec3 tangent = glm::vec3(0, 0, 0);
        Color color = Colors::white();

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(position),
                CEREAL_NVP(texCoord),
                CEREAL_NVP(normal),
                CEREAL_NVP(tangent),
                CEREAL_NVP(color)
            );
        }
    };

    class Texture;
    struct Cube;
    struct Sphere;
    struct Rectangle;
    struct Capsule;
    struct Ray;
    struct Line;
    struct Plane;
    struct Triangle;
    struct Polygon;
    struct Frustum;
    struct BoundingBox;
    struct Grid;
    struct MeshSource;


    struct DARMOK_EXPORT MeshData final
    {
        using Vertex = MeshDataVertex;
        using Index = VertexIndex;
        using MeshType = protobuf::MeshType;
        using RectangleMeshType = protobuf::RectangleMeshType;
        using LineMeshType = protobuf::LineMeshType;

        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        MeshType::Enum type = MeshType::Static;

        MeshData(MeshType::Enum type = MeshType::Static) noexcept;
        MeshData(const Cube& Cube, RectangleMeshType::Enum type = RectangleMeshType::Full) noexcept;
        MeshData(const Sphere& sphere, unsigned int lod = 32) noexcept;
        MeshData(const Capsule& capsule, unsigned int lod = 32) noexcept;
        MeshData(const Rectangle& rect, RectangleMeshType::Enum type = RectangleMeshType::Full) noexcept;
        MeshData(const Plane& plane, RectangleMeshType::Enum type = RectangleMeshType::Full, float scale = 100.F) noexcept;
        MeshData(const Ray& ray) noexcept;
        MeshData(const Line& line, LineMeshType::Enum type = LineMeshType::Line) noexcept;
        MeshData(const Triangle& tri) noexcept;
        MeshData(const Polygon& poly) noexcept;
        MeshData(const Frustum& frust, RectangleMeshType::Enum type = RectangleMeshType::Outline) noexcept;
        MeshData(const Grid& grid) noexcept;

        MeshData& operator+=(const MeshData& other) noexcept;
        MeshData operator+(const MeshData& other) const noexcept;

        MeshData& operator*=(const glm::mat4& trans) noexcept;
        MeshData& operator*=(const Color& color) noexcept;

        MeshData& scalePositions(const glm::vec3& scale) noexcept;
        MeshData& translatePositions(const glm::vec3& pos) noexcept;
        MeshData& scaleTexCoords(const glm::vec2& scale) noexcept;
        MeshData& translateTexCoords(const glm::vec2& pos) noexcept;
        MeshData& setColor(const Color& color) noexcept;

        MeshData& createIndices() noexcept;
        MeshData& shiftIndices(Index offset) noexcept;
        MeshData& calcNormals() noexcept;
        MeshData& calcTangents() noexcept;

        using Face = std::array<Index, 3>;
        [[nodiscard]] std::vector<Face> getFaces() const noexcept;

        BoundingBox getBounds() const noexcept;

        bool empty() const noexcept;
        void clear() noexcept;

        void exportData(const bgfx::VertexLayout& vertexLayout, Data& vertexData, Data& indexData) const noexcept;
        [[nodiscard]] Mesh::Definition createMeshDefinition(const bgfx::VertexLayout& vertexLayout, const IMesh::Config& config = {}) const;
        [[nodiscard]] std::unique_ptr<IMesh> createMesh(const bgfx::VertexLayout& vertexLayout, const IMesh::Config& config = {}) const;
        [[nodiscard]] static const bgfx::VertexLayout& getDefaultVertexLayout() noexcept;

        MeshData& convertQuadIndicesToLine() noexcept;
        MeshData& subdivide(size_t amount = 1) noexcept;
        size_t subdivideDensity(float maxDistance) noexcept;

        static Vertex mix(const Vertex& v1, const Vertex& v2, float f) noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(vertices),
                CEREAL_NVP(indices),
                CEREAL_NVP(type)
            );
        }
    private:
        static const std::vector<Index> _cuboidTriangleIndices;

        bool doSubdivide(size_t i, float maxDistance = std::numeric_limits<float>::infinity()) noexcept;
        static void doCreateIndices(std::vector<Index>& indices, size_t size) noexcept;
        void setupBasicRectangle() noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IMeshLoader : public IFromDefinitionLoader<IMesh, Mesh::Definition>
    {
    };

    class DARMOK_EXPORT MeshLoader final : public FromDefinitionLoader<IMeshLoader, IMeshDefinitionLoader>
    {
    public:
        MeshLoader(IMeshDefinitionLoader& defLoader) noexcept;
    protected:

        Result create(const std::shared_ptr<Definition>& def) override;
    };
}