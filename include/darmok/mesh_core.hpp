#pragma once

#include <darmok/export.h>
#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/vertex_fwd.hpp>
#include <darmok/glm.hpp>
#include <darmok/varying.hpp>
#include <darmok/loader.hpp>
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
    struct DARMOK_EXPORT MeshRenderConfig final
    {
        uint8_t vertexStream = 0;
        uint32_t startVertex = 0;
        uint32_t numVertices = 0;
        uint32_t startIndex = 0;
        uint32_t numIndices = 0;

        void fix(uint32_t maxVertices, uint32_t maxIndices) noexcept;
    };

    struct DARMOK_EXPORT MeshConfig final
    {
        using Definition = protobuf::Mesh;
        using Type = Definition::Type;
		Type type = Definition::Static;
        bool index32 = false;

        uint16_t getFlags() const noexcept;
        size_t getIndexSize() const noexcept;

		static MeshConfig fromDefinition(const Definition& def) noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IMeshDefinitionLoader : public ILoader<protobuf::Mesh>
    {
    };

    using DataMeshDefinitionLoader = DataProtobufLoader<IMeshDefinitionLoader>;

    class DARMOK_EXPORT BX_NO_VTABLE IMeshSourceLoader : public ILoader<protobuf::MeshSource>
    {
    };

    class DARMOK_EXPORT Mesh final
    {
    public:
        using Config = MeshConfig;
        using RenderConfig = MeshRenderConfig;
        using Definition = protobuf::Mesh;
        using Type = protobuf::Mesh::Type;
        using Source = protobuf::MeshSource;
        
        Mesh(Mesh&& other) = default;
        Mesh& operator=(Mesh&& other) = default;
        Mesh(const Mesh& other) = delete;
        Mesh& operator=(const Mesh& other) = delete;

        expected<void, std::string> render(bgfx::Encoder& encoder, RenderConfig config = {}) const noexcept;

        [[nodiscard]] std::string toString() const noexcept;        
        [[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept;
        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] uint16_t getVertexHandleIndex() const noexcept;

        // only if dynamic
        expected<void, std::string> updateVertices(DataView data, uint32_t offset = 0) noexcept;
        expected<void, std::string> updateIndices(DataView data, uint32_t offset = 0) noexcept;

        [[nodiscard]] static Source createSource() noexcept;
        [[nodiscard]] static Definition createDefinition() noexcept;
        [[nodiscard]] static expected<Mesh, std::string> load(const bgfx::VertexLayout& layout, DataView vertices, Config config = {}) noexcept;
        [[nodiscard]] static expected<Mesh, std::string> load(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config = {}) noexcept;
        [[nodiscard]] static expected<Mesh, std::string> load(const Definition& def) noexcept;
    private:
        struct StaticMode final
        {
            bgfx::VertexBufferHandle vertexBuffer{ bgfx::kInvalidHandle };
            bgfx::IndexBufferHandle indexBuffer{ bgfx::kInvalidHandle };

            StaticMode() = default;
            ~StaticMode() noexcept;
            StaticMode(const StaticMode& other) = delete;
            StaticMode& operator=(const StaticMode& other) = delete;
            StaticMode(StaticMode&& other) noexcept;
            StaticMode& operator=(StaticMode&& other) noexcept;

            static expected<StaticMode, std::string> create(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config) noexcept;
            expected<void, std::string> render(bgfx::Encoder& encoder, RenderConfig config = {}) const noexcept;
        private:
			void clear() noexcept;
        };

        struct DynamicMode final
        {
            bgfx::DynamicVertexBufferHandle vertexBuffer{ bgfx::kInvalidHandle };
            bgfx::DynamicIndexBufferHandle indexBuffer{ bgfx::kInvalidHandle };

            DynamicMode() = default;
            ~DynamicMode() noexcept;
            DynamicMode(const DynamicMode& other) = delete;
            DynamicMode& operator=(const DynamicMode& other) = delete;
            DynamicMode(DynamicMode&& other) noexcept;
            DynamicMode& operator=(DynamicMode&& other) noexcept;

            static expected<DynamicMode, std::string> create(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config) noexcept;
            expected<void, std::string> render(bgfx::Encoder& encoder, RenderConfig config = {}) const noexcept;
        private:
            void clear() noexcept;
        };

        struct TransientMode final
        {
            bgfx::TransientVertexBuffer vertexBuffer;
            bgfx::TransientIndexBuffer indexBuffer;

            TransientMode() = default;
            TransientMode(const TransientMode& other) = delete;
            TransientMode& operator=(const TransientMode& other) = delete;
            TransientMode(TransientMode&& other) = default;
            TransientMode& operator=(TransientMode&& other) = default;
            
            static expected<TransientMode, std::string> create(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config) noexcept;
            expected<void, std::string> render(bgfx::Encoder& encoder, RenderConfig config = {}) const noexcept;
        };

        using Mode = std::variant<StaticMode, DynamicMode, TransientMode>;

        Mesh(Type type, Mode mode, const bgfx::VertexLayout& layout, size_t vertNum, size_t idxNum) noexcept;
        static expected<Mode, std::string> createMode(Type type, const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config) noexcept;

        Type _type;
        Mode _mode;
        bgfx::VertexLayout _layout;
        size_t _vertNum;
        size_t _idxNum;
    };

    struct DARMOK_EXPORT MeshDataWeight final
    {
        size_t boneIndex = 0;
        float value = 0.F;
    };

    struct DARMOK_EXPORT MeshDataVertex final
    {
        glm::vec3 position{};
        glm::vec2 texCoord{};
        glm::vec3 normal = glm::vec3(0, 1, 0);
        glm::vec3 tangent = glm::vec3(0, 0, 0);
        Color color = Colors::white();
        std::vector<MeshDataWeight> weights;
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
        using Weight = MeshDataWeight;
        using MeshType = protobuf::Mesh::Type;
        using RectangleType = protobuf::Mesh::RectangleType;
        using LineType = protobuf::Mesh::LineType;
        using Definition = protobuf::MeshSource;
		using DataDefinition = protobuf::DataMeshSource;
        using CubeDefinition = protobuf::CubeMeshSource;
        using SphereDefinition = protobuf::SphereMeshSource;
        using CapsuleDefinition = protobuf::CapsuleMeshSource;
        using RectangleDefinition = protobuf::RectangleMeshSource;

        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        MeshType type = protobuf::Mesh::Static;

        MeshData(MeshType type = protobuf::Mesh::Static) noexcept;
        MeshData(const Cube& Cube, RectangleType type = Mesh::Definition::FullRectangle) noexcept;
        MeshData(const Sphere& sphere, unsigned int lod = 32) noexcept;
        MeshData(const Capsule& capsule, unsigned int lod = 32) noexcept;
        MeshData(const Rectangle& rect, RectangleType type = Mesh::Definition::FullRectangle) noexcept;
        MeshData(const Plane& plane, RectangleType type = Mesh::Definition::FullRectangle, float scale = 100.F) noexcept;
        MeshData(const Ray& ray) noexcept;
        MeshData(const Line& line, LineType type = Mesh::Definition::Line) noexcept;
        MeshData(const Triangle& tri) noexcept;
        MeshData(const Polygon& poly) noexcept;
        MeshData(const Frustum& frust, RectangleType type = Mesh::Definition::OutlineRectangle) noexcept;
        MeshData(const Grid& grid) noexcept;

        MeshData(const Definition& def) noexcept;
        MeshData(const DataDefinition& def) noexcept;
        MeshData(const CubeDefinition& def) noexcept;
        MeshData(const SphereDefinition& def) noexcept;
        MeshData(const CapsuleDefinition& def) noexcept;
        MeshData(const RectangleDefinition& def) noexcept;

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

        MeshData& setName(std::string_view name) noexcept;
		[[nodiscard]] const std::string& getName() const noexcept;

        void exportData(const bgfx::VertexLayout& vertexLayout, Data& vertexData, Data& indexData) const noexcept;
        [[nodiscard]] Mesh::Definition createDefinition(const bgfx::VertexLayout& vertexLayout, const Mesh::Config& config = {}) const noexcept;
        [[nodiscard]] std::shared_ptr<Mesh::Definition> createSharedDefinition(const bgfx::VertexLayout& vertexLayout, const Mesh::Config& config = {}) const noexcept;
        [[nodiscard]] expected<Mesh, std::string> createMesh(const bgfx::VertexLayout& vertexLayout, const Mesh::Config& config = {}) const noexcept;
        [[nodiscard]] expected<std::shared_ptr<Mesh>, std::string> createSharedMesh(const bgfx::VertexLayout& vertexLayout, const Mesh::Config& config = {}) const noexcept;
        [[nodiscard]] static const bgfx::VertexLayout& getDefaultVertexLayout() noexcept;

        MeshData& convertQuadIndicesToLine() noexcept;
        MeshData& subdivide(size_t amount = 1) noexcept;
        size_t subdivideDensity(float maxDistance) noexcept;

        static Vertex mix(const Vertex& v1, const Vertex& v2, float f) noexcept;

    private:
        static const std::vector<Index> _cuboidTriangleIndices;
        std::string _name;

        bool doSubdivide(size_t i, float maxDistance = std::numeric_limits<float>::infinity()) noexcept;
        static void doCreateIndices(std::vector<Index>& indices, size_t size) noexcept;
        void setupBasicRectangle() noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IMeshLoader : public ILoader<Mesh>{};
    class DARMOK_EXPORT BX_NO_VTABLE IMeshFromDefinitionLoader : public IFromDefinitionLoader<IMeshLoader, Mesh::Definition>{};

    using MeshLoader = FromDefinitionLoader<IMeshFromDefinitionLoader, IMeshDefinitionLoader>;

    class DARMOK_EXPORT BX_NO_VTABLE IMeshDefinitionFromSourceLoader : public IFromDefinitionLoader<IMeshDefinitionLoader, Mesh::Source>{};
}