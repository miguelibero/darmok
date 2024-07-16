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

    struct DARMOK_EXPORT MeshDataConfig final
    {
        glm::vec3 scale = glm::vec3(1);
        glm::vec3 offset = glm::vec3(0);
        glm::vec2 textureScale = glm::vec2(1);
        glm::vec2 textureOffset = glm::vec3(0);
        Color color = Colors::white();
        MeshType type = MeshType::Static;
        bool index32 = false;
        int16_t indexOffset = 0;
        glm::mat4 transform = glm::mat4(1);
    };

    struct DARMOK_EXPORT MeshDataVertex final
    {
        glm::vec3 position;
        glm::vec3 normal = glm::vec3(0, 1, 0);
        glm::vec2 texCoord;
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
        using Config = MeshDataConfig;

        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        Config config;

        MeshData() = default;
        MeshData(const Cube& Cube) noexcept;
        MeshData(const Sphere& sphere, int lod = 32) noexcept;
        MeshData(const Capsule& capsule, int lod = 32) noexcept;
        MeshData(const Rectangle& rect, RectangleMeshType type = RectangleMeshType::Full) noexcept;
        MeshData(const Ray& ray) noexcept;
        MeshData(const Line& line, LineMeshType type = LineMeshType::Line) noexcept;
        MeshData(const Triangle& tri) noexcept;

        [[nodiscard]] MeshData operator+(const MeshData& other) noexcept;
        MeshData& operator+=(const MeshData& other) noexcept;
        void normalize() noexcept;
        void denormalize(const Config& config) noexcept;
        bool empty() const noexcept;
        void clear() noexcept;

        [[nodiscard]] void exportData(const bgfx::VertexLayout& vertexLayout, Data& vertexData, Data& indexData) const noexcept;
        [[nodiscard]] std::unique_ptr<IMesh> createMesh(const bgfx::VertexLayout& vertexLayout, const IMesh::Config& config = {}) const;
        [[nodiscard]] static const bgfx::VertexLayout& getDefaultVertexLayout() noexcept;
    };
}