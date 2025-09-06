#pragma once

#include <darmok/mesh_core.hpp>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class IProgramDefinitionLoader;

    class DARMOK_EXPORT MeshDefinitionFromSourceLoader final : public FromDefinitionLoader<IMeshDefinitionFromSourceLoader, IMeshSourceLoader>
    {
    public:
        MeshDefinitionFromSourceLoader(IMeshSourceLoader& srcLoader, IProgramDefinitionLoader& progDefLoader, bx::AllocatorI& allocator) noexcept;
    private:
        IProgramDefinitionLoader& _progDefLoader;
        bx::AllocatorI& _allocator;
        Result create(const std::shared_ptr<Mesh::Source>& src) override;
        Result create(const protobuf::ExternalMeshSource& external);
    };
}