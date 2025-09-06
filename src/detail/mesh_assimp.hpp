#pragma once

#include <darmok/mesh_assimp.hpp>
#include <darmok/optional_ref.hpp>
#include <mikktspace.h>
#include <glm/glm.hpp>

#include <vector>

struct aiBone;

namespace darmok
{
    class VertexDataWriter;

    class AssimpMeshDefinitionConverterImpl final
    {
    public:
        using Definition = AssimpMeshDefinitionConverter::Definition;
		using ImportConfig = AssimpMeshDefinitionConverter::ImportConfig;

        AssimpMeshDefinitionConverterImpl(const aiMesh& assimpMesh, Definition& meshDef, bx::AllocatorI& allocator, const ImportConfig& config = {}) noexcept;
        expected<void, std::string> operator()() noexcept;
    private:
        const aiMesh& _assimpMesh;
        Definition& _meshDef;
        bx::AllocatorI& _allocator;
        ImportConfig _config;

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