#include <darmok/mesh_source.hpp>

namespace darmok
{
    std::shared_ptr<MeshDefinition> MeshSource::createDefinition(const bgfx::VertexLayout& layout)
    {
        MeshData data;
        if (auto cubeContent = std::get_if<CubeMeshSource>(&content))
        {
            data += MeshData(cubeContent->cube, cubeContent->type);
        }
        else if (auto sphereContent = std::get_if<SphereMeshSource>(&content))
        {
            data += MeshData(sphereContent->sphere, sphereContent->lod);
        }
        else if (auto capsuleContent = std::get_if<CapsuleMeshSource>(&content))
        {
            data += MeshData(capsuleContent->capsule, capsuleContent->lod);
        }
        else if (auto rectContent = std::get_if<RectangleMeshSource>(&content))
        {
            data += MeshData(rectContent->rectangle, rectContent->type);
        }
        else
        {
            return nullptr;
        }
        auto def = std::make_shared<MeshDefinition>(data.createMeshDefinition(layout, config));
        def->name = name;
        return def;
    }
}