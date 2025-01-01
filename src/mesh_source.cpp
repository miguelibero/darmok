#include <darmok/mesh_source.hpp>

namespace darmok
{
    std::shared_ptr<MeshDefinition> MeshSource::createDefinition(const bgfx::VertexLayout& layout)
    {
        MeshData data;
        auto def = std::make_shared<MeshDefinition>(data.createMeshDefinition(layout, config));
        def->name = name;
        return def;
    }
}