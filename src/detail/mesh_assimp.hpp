#pragma once

#include <darmok/mesh_assimp.hpp>
#include <darmok/optional_ref.hpp>
#include <mikktspace.h>
#include <glm/glm.hpp>

#include <vector>

struct aiBone;
struct aiScene;

namespace darmok
{
    class VertexDataWriter;

    class AssimpMeshSourceConverterImpl final
    {
    public:
        using Definition = AssimpMeshSourceConverter::Definition;

        AssimpMeshSourceConverterImpl(DataView data, std::string_view format, Definition& def) noexcept;

        std::vector<std::string> getMeshNames() const noexcept;
        expected<void, std::string> operator()(std::string_view name) noexcept;
    private:
        DataView _data;
		std::string _format;
        Definition& _def;
        mutable std::shared_ptr<aiScene> _scene;

        expected<void, std::string> loadScene() const noexcept;
    };

    class AssimpMeshDefinitionConverterImpl final
    {
    public:
        using Definition = AssimpMeshDefinitionConverter::Definition;
		using VertexLayout = AssimpMeshDefinitionConverter::VertexLayout;

        AssimpMeshDefinitionConverterImpl(const aiMesh& assimpMesh, const VertexLayout& layout, Definition& meshDef, OptionalRef<bx::AllocatorI> = std::nullopt) noexcept;
        expected<void, std::string> operator()() noexcept;
    private:
        const aiMesh& _assimpMesh;
        VertexLayout _vertexLayout;
        Definition& _meshDef;
        OptionalRef<bx::AllocatorI> _allocator;

        bool updateBoneData(const std::vector<aiBone*>& bones, VertexDataWriter& writer) const noexcept;
        std::string createVertexData(const std::vector<aiBone*>& bones) const noexcept;
        std::string createIndexData() const noexcept;
    };

    struct AssimpCalcTangentsOperation final
    {
    public:
        AssimpCalcTangentsOperation() noexcept;
        std::vector<glm::vec3> operator()(const aiMesh& mesh) noexcept;

    private:
        SMikkTSpaceInterface _iface{};
        SMikkTSpaceContext _context{};
        OptionalRef<const aiMesh> _mesh;
        std::vector<glm::vec3> _tangents;

        static const aiMesh& getMeshFromContext(const SMikkTSpaceContext* context) noexcept;
        static int getVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert) noexcept;
        static int getNumFaces(const SMikkTSpaceContext* context) noexcept;
        static int getNumFaceVertices(const SMikkTSpaceContext* context, int iFace) noexcept;
        static void getPosition(const SMikkTSpaceContext* context, float outpos[], int iFace, int iVert) noexcept;
        static void getNormal(const SMikkTSpaceContext* context, float outnormal[], int iFace, int iVert) noexcept;
        static void getTexCoords(const SMikkTSpaceContext* context, float outuv[], int iFace, int iVert) noexcept;
        static void setTangent(const SMikkTSpaceContext* context, const float tangentu[], float fSign, int iFace, int iVert) noexcept;
    };

}