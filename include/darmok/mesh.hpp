#pragma once

#include <darmok/mesh_core.hpp>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class IProgramSourceLoader;

    class DARMOK_EXPORT MeshDefinitionFromSourceLoader final : public FromDefinitionLoader<IMeshDefinitionFromSourceLoader, IMeshSourceLoader>
    {
    public:
        MeshDefinitionFromSourceLoader(IMeshSourceLoader& srcLoader, IProgramSourceLoader& progLoader) noexcept;
    private:
        IProgramSourceLoader& _progLoader;
        Result create(std::shared_ptr<Mesh::Source> src) noexcept override;
    };
}