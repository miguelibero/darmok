#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/data.hpp>
#include <darmok/protobuf/mesh.pb.h>
#include <darmok/protobuf/assimp.pb.h>

#include <memory>
#include <string>

struct aiMesh;

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class AssimpMeshSourceConverterImpl;

    class DARMOK_EXPORT AssimpMeshSourceConverter final
    {
    public:
        using Definition = protobuf::DataMeshSource;

        AssimpMeshSourceConverter(DataView data, std::string_view format, Definition& def) noexcept;
        ~AssimpMeshSourceConverter();

		std::vector<std::string> getMeshNames() const noexcept;
        expected<void, std::string> operator()(std::string_view name) noexcept;

    private:
        std::unique_ptr<AssimpMeshSourceConverterImpl> _impl;
    };

    class AssimpMeshDefinitionConverterImpl;

    class DARMOK_EXPORT AssimpMeshDefinitionConverter final
    {
    public:
        using Definition = protobuf::Mesh;
		using VertexLayout = protobuf::VertexLayout;

        AssimpMeshDefinitionConverter(const aiMesh& assimpMesh, const VertexLayout& layout, Definition& meshDef, OptionalRef<bx::AllocatorI> alloc = std::nullopt) noexcept;
        ~AssimpMeshDefinitionConverter();
        expected<void, std::string> operator()() noexcept;

    private:
		std::unique_ptr<AssimpMeshDefinitionConverterImpl> _impl;
    };
}
