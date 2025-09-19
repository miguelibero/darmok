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
        MeshDefinitionFromSourceLoader(IMeshSourceLoader& srcLoader, IProgramDefinitionLoader& progDefLoader) noexcept;
    private:
        IProgramDefinitionLoader& _progDefLoader;
        Result create(std::shared_ptr<Mesh::Source> src) noexcept override;
    };
}