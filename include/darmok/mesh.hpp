#pragma once

#include <vector>
#include <memory>
#include <optional>

#include <darmok/color.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/vertex_fwd.hpp>
#include <darmok/mesh_fwd.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <glm/glm.hpp>

namespace darmok
{
    class DataView;

    struct MeshConfig final
    {
        bool index32 = false;

        uint64_t getFlags() const noexcept;
        size_t getIndexSize() const noexcept;
    };

    class BX_NO_VTABLE IMesh
    {
    public:
        using Config = MeshConfig;

        virtual ~IMesh() = default;
        DLLEXPORT virtual std::string to_string() const noexcept = 0;
        DLLEXPORT virtual void render(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const = 0;
        DLLEXPORT virtual const bgfx::VertexLayout& getVertexLayout() const noexcept = 0;

        static std::shared_ptr<IMesh> create(MeshType type, const bgfx::VertexLayout& layout, const DataView& vertices, Config config = {}) noexcept;
        static std::shared_ptr<IMesh> create(MeshType type, const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, Config config = {}) noexcept;
    };

    class Mesh final : public IMesh
    {
    public:
        using Config = MeshConfig;
        DLLEXPORT Mesh(const bgfx::VertexLayout& layout, const DataView& vertices, Config config = {}) noexcept;
        DLLEXPORT Mesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, Config config = {}) noexcept;
        DLLEXPORT ~Mesh() noexcept;
        DLLEXPORT Mesh(Mesh&& other) noexcept;
        DLLEXPORT Mesh& operator=(Mesh&& other) noexcept;

        Mesh(const Mesh& other) = delete;
        Mesh& operator=(const Mesh& other) = delete;

        bgfx::VertexBufferHandle getVertexHandle() const noexcept;
        bgfx::IndexBufferHandle getIndexHandle() const noexcept;

        DLLEXPORT std::string to_string() const noexcept override;
        DLLEXPORT void render(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const override;
        DLLEXPORT const bgfx::VertexLayout& getVertexLayout() const noexcept override;
    private:
        bgfx::VertexLayout _layout;
        bgfx::VertexBufferHandle _vertexBuffer;
        bgfx::IndexBufferHandle _indexBuffer;
        size_t _vertNum;
        size_t _idxNum;
    };

    class DynamicMesh final : public IMesh
    {
    public:
        using Config = MeshConfig;
        DLLEXPORT DynamicMesh(const bgfx::VertexLayout& layout, const DataView& vertices, Config config = {}) noexcept;
        DLLEXPORT DynamicMesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, Config config = {}) noexcept;
        DLLEXPORT ~DynamicMesh() noexcept;
        DLLEXPORT DynamicMesh(DynamicMesh&& other) noexcept;
        DLLEXPORT DynamicMesh& operator=(DynamicMesh&& other) noexcept;

        DynamicMesh(const DynamicMesh& other) = delete;
        Mesh& operator=(const DynamicMesh& other) = delete;

        bgfx::DynamicVertexBufferHandle getVertexHandle() const noexcept;
        bgfx::DynamicIndexBufferHandle getIndexHandle() const noexcept;

        DLLEXPORT void updateVertices(const DataView& data, size_t offset = 0) noexcept;
        DLLEXPORT void updateIndices(const DataView& data, size_t offset = 0) noexcept;

        DLLEXPORT std::string to_string() const noexcept override;
        DLLEXPORT void render(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const override;
        DLLEXPORT const bgfx::VertexLayout& getVertexLayout() const noexcept override;
    private:
        bgfx::VertexLayout _layout;
        bgfx::DynamicVertexBufferHandle _vertexBuffer;
        bgfx::DynamicIndexBufferHandle _indexBuffer;
        size_t _idxSize;
        size_t _vertNum;
        size_t _idxNum;
    };

    class TransientMesh final : public IMesh
    {
    public:
        DLLEXPORT TransientMesh(const bgfx::VertexLayout& layout, const DataView& vertices, bool index32 = false);
        DLLEXPORT TransientMesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, bool index32 = false);
        DLLEXPORT TransientMesh(TransientMesh&& other) noexcept;
        DLLEXPORT TransientMesh& operator=(TransientMesh&& other) noexcept;

        TransientMesh(const Mesh& other) = delete;
        TransientMesh& operator=(const TransientMesh& other) = delete;

        DLLEXPORT std::string to_string() const noexcept override;
        DLLEXPORT void render(bgfx::Encoder& encoder, uint8_t vertexStream = 0) const override;
        DLLEXPORT const bgfx::VertexLayout& getVertexLayout() const noexcept override;
    private:
        bgfx::VertexLayout _layout;
        bgfx::TransientVertexBuffer _vertexBuffer;
        bgfx::TransientIndexBuffer _indexBuffer;
        size_t _vertNum;
        size_t _idxNum;
    };

    struct MeshCreationConfig final
    {
        glm::vec3 scale = glm::vec3(1);
        glm::vec3 offset = glm::vec3(0);
        glm::vec2 textureScale = glm::vec2(1);
        glm::vec2 textureOffset = glm::vec3(0);
        Color color = Colors::white();
        MeshType type = MeshType::Static;
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
    struct Rectangle;
    struct Ray;
    struct Line;

    struct MeshCreator final
    {
        using Config = MeshCreationConfig;
        Config config;
        std::optional<bgfx::VertexLayout> vertexLayout;

        DLLEXPORT MeshCreator(std::optional<bgfx::VertexLayout> vertexLauout = std::nullopt) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createMesh(const MeshData& meshData) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createCube() noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createCube(const Cube& cube) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createSphere(const Sphere& sphere, int lod = 32) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createSphere(int lod = 32) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createRectangle() noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createRectangle(const Rectangle& rect) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createLineRectangle() noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createLineRectangle(const Rectangle& rect) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createRay(const Ray& ray) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createLine(const Line& line) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createLines(const std::vector<Line>& lines) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> createBone() noexcept;

    private:
        const static MeshData _rectangleMeshData;

        std::shared_ptr<IMesh> createMesh(const MeshData& meshData, const Config& cfg) noexcept;
        std::shared_ptr<IMesh> createRectangleMesh(const MeshData& data, const Rectangle& quad) noexcept;
        static bgfx::VertexLayout getDefaultVertexLayout(const MeshData& meshData) noexcept;
    };
}