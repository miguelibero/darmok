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

        AssimpMeshSourceConverter(const aiMesh& assimpMesh, Definition& def) noexcept;
        ~AssimpMeshSourceConverter();

        expected<void, std::string> operator()() noexcept;

    private:
        std::unique_ptr<AssimpMeshSourceConverterImpl> _impl;
    };
}
