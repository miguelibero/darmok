#pragma once

#include <darmok/mesh_core.hpp>
#include <optional>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class DARMOK_EXPORT MeshDefinitionFromSourceLoader final : public FromDefinitionLoader<IMeshDefinitionFromSourceLoader, IMeshSourceLoader>
    {
    public:
        MeshDefinitionFromSourceLoader(IMeshSourceLoader& srcLoader) noexcept;
    private:
        Result create(std::shared_ptr<Mesh::Source> src) noexcept override;
    };
}
