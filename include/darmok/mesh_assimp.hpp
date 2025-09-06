#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/protobuf/mesh.pb.h>
#include <darmok/protobuf/assimp.pb.h>

#include <memory>

struct aiMesh;

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class AssimpMeshDefinitionConverterImpl;

    class DARMOK_EXPORT AssimpMeshDefinitionConverter final
    {
    public:
        using Definition = protobuf::Mesh;
		using VertexLayout = protobuf::VertexLayout;

        AssimpMeshDefinitionConverter(const aiMesh& assimpMesh, Definition& meshDef, const VertexLayout& layout, bx::AllocatorI& alloc) noexcept;
        ~AssimpMeshDefinitionConverter();
        expected<void, std::string> operator()() noexcept;
    private:
		std::unique_ptr<AssimpMeshDefinitionConverterImpl> _impl;
    };
}
