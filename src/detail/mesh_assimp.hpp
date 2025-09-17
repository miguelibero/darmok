#pragma once

#include <darmok/mesh_assimp.hpp>
#include <darmok/optional_ref.hpp>
#include <mikktspace.h>
#include <glm/glm.hpp>

#include <vector>

struct aiScene;

namespace darmok
{
    class VertexDataWriter;

    class AssimpMeshSourceConverterImpl final
    {
    public:
        using Definition = AssimpMeshSourceConverter::Definition;

        AssimpMeshSourceConverterImpl(const aiMesh& assimpMesh, Definition& def) noexcept;

        expected<void, std::string> operator()() noexcept;
    private:
        const aiMesh& _assimpMesh;
        Definition& _def;
    };
}