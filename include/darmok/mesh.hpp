#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <variant>

#include <darmok/export.h>
#include <darmok/color.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/vertex_fwd.hpp>
#include <darmok/mesh_fwd.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <darmok/glm.hpp>

namespace darmok
{
    class DataView;

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

    class DARMOK_EXPORT BX_NO_VTABLE IMesh
    {
    public:
        using Config = MeshConfig;

        virtual ~IMesh() = default;
         virtual [[nodiscard]] std::string to_string() const noexcept = 0;
         virtual void render(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const = 0;
         virtual [[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept = 0;

         static [[nodiscard]] std::unique_ptr<IMesh> create(MeshType type, const bgfx::VertexLayout& layout, const DataView& vertices, Config config = {}) noexcept;
         static [[nodiscard]] std::unique_ptr<IMesh> create(MeshType type, const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, Config config = {}) noexcept;
    };

    class DARMOK_EXPORT Mesh final : public IMesh
    {
    public:
        using Config = MeshConfig;
         Mesh(const bgfx::VertexLayout& layout, const DataView& vertices, Config config = {}) noexcept;
         Mesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, Config config = {}) noexcept;
         ~Mesh() noexcept;
         Mesh(Mesh&& other) noexcept;
         Mesh& operator=(Mesh&& other) noexcept;

        Mesh(const Mesh& other) = delete;
        Mesh& operator=(const Mesh& other) = delete;

         [[nodiscard]] bgfx::VertexBufferHandle getVertexHandle() const noexcept;
         [[nodiscard]] bgfx::IndexBufferHandle getIndexHandle() const noexcept;

         [[nodiscard]] std::string to_string() const noexcept override;
         void render(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const override;
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
         DynamicMesh(const bgfx::VertexLayout& layout, const DataView& vertices, Config config = {}) noexcept;
         DynamicMesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, Config config = {}) noexcept;
         ~DynamicMesh() noexcept;
         DynamicMesh(DynamicMesh&& other) noexcept;
         DynamicMesh& operator=(DynamicMesh&& other) noexcept;

        DynamicMesh(const DynamicMesh& other) = delete;
        Mesh& operator=(const DynamicMesh& other) = delete;

         [[nodiscard]] bgfx::DynamicVertexBufferHandle getVertexHandle() const noexcept;
         [[nodiscard]] bgfx::DynamicIndexBufferHandle getIndexHandle() const noexcept;

         void updateVertices(const DataView& data, uint32_t offset = 0) noexcept;
         void updateIndices(const DataView& data, uint32_t offset = 0) noexcept;

         [[nodiscard]] std::string to_string() const noexcept override;
         void render(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const override;
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
         TransientMesh(const bgfx::VertexLayout& layout, const DataView& vertices, bool index32 = false);
         TransientMesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, bool index32 = false);
         TransientMesh(TransientMesh&& other) noexcept;
         TransientMesh& operator=(TransientMesh&& other) noexcept;

        TransientMesh(const Mesh& other) = delete;
        TransientMesh& operator=(const TransientMesh& other) = delete;

         [[nodiscard]] std::string to_string() const noexcept override;
         void render(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const override;
         [[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept override;
    private:
        bgfx::VertexLayout _layout;
        bgfx::TransientVertexBuffer _vertexBuffer;
        bgfx::TransientIndexBuffer _indexBuffer;
        uint32_t _vertNum;
        uint32_t _idxNum;
    };

    struct DARMOK_EXPORT MeshCreationConfig final
    {
        glm::vec3 scale = glm::vec3(1);
        glm::vec3 offset = glm::vec3(0);
        glm::vec2 textureScale = glm::vec2(1);
        glm::vec2 textureOffset = glm::vec3(0);
        Color color = Colors::white();
        MeshType type = MeshType::Static;
    };

    struct DARMOK_EXPORT MeshVertexData
    {
        glm::vec3 position;
        glm::vec3 normal = glm::vec3(0, 1, 0);
        glm::vec2 texCoord;
        Color color = Colors::white();
    };

    struct DARMOK_EXPORT MeshData
    {
        std::vector<MeshVertexData> vertices;
        std::vector<VertexIndex> indices;
    };

    class Texture;
    struct Cuboid;
    struct Sphere;
    struct Rectangle;
    struct Capsule;
    struct Ray;
    struct Line;
    struct Triangle;

    struct DARMOK_EXPORT MeshCreator final
    {
        using Config = MeshCreationConfig;
        Config config;
        std::optional<bgfx::VertexLayout> vertexLayout;
        using Shape = std::variant<Cuboid, Sphere, Capsule, Rectangle, Ray, Line, Triangle, MeshData>;
        using MeshPtr = std::unique_ptr<IMesh>;
        MeshCreator(std::optional<bgfx::VertexLayout> vertexLauout = std::nullopt) noexcept;
        [[nodiscard]] MeshPtr createMesh(const MeshData& meshData) noexcept;
        [[nodiscard]] MeshPtr createCuboid() noexcept;
        [[nodiscard]] MeshPtr createCuboid(const Cuboid& cuboid) noexcept;
        [[nodiscard]] MeshPtr createSphere(const Sphere& sphere, int lod = 32) noexcept;
        [[nodiscard]] MeshPtr createSphere(int lod = 32) noexcept;
        [[nodiscard]] MeshPtr createCapsule(const Capsule& capsule, int lod = 32) noexcept;
        [[nodiscard]] MeshPtr createCapsule(int lod = 32) noexcept;
        [[nodiscard]] MeshPtr createRectangle() noexcept;
        [[nodiscard]] MeshPtr createRectangle(const Rectangle& rect) noexcept;
        [[nodiscard]] MeshPtr createLineRectangle() noexcept;
        [[nodiscard]] MeshPtr createLineRectangle(const Rectangle& rect) noexcept;
        [[nodiscard]] MeshPtr createRay(const Ray& ray) noexcept;
        [[nodiscard]] MeshPtr createLine(const Line& line) noexcept;
        [[nodiscard]] MeshPtr createLines(const std::vector<Line>& lines) noexcept;
        [[nodiscard]] MeshPtr createTriangle(const Triangle& tri) noexcept;
        [[nodiscard]] MeshPtr createTriangles(const std::vector<Triangle>& tris) noexcept;
        [[nodiscard]] MeshPtr createBone() noexcept;
        [[nodiscard]] MeshPtr createShape(const Shape& shape) noexcept;

    private:
        static const MeshData& getRectangleMeshData() noexcept;

        MeshPtr createMesh(const MeshData& meshData, const Config& cfg) noexcept;
        MeshPtr createRectangleMesh(const MeshData& data, const Rectangle& quad) noexcept;
        static bgfx::VertexLayout getDefaultVertexLayout(const MeshData& meshData) noexcept;
    };
}