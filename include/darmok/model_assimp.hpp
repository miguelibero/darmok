#pragma once

#include <darmok/model.hpp>
#include <darmok/optional_ref.hpp>
#include <memory>
#include <string>

namespace bx
{
    struct AllocatorI;
}

namespace bgfx
{
    struct VertexLayout;
}

namespace darmok
{
    class IDataLoader;
    class IImageLoader;
    class AssimpModelLoaderImpl;

    class AssimpModelLoader final : public IModelLoader
    {
    public:
        DLLEXPORT AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader = nullptr) noexcept;
        DLLEXPORT ~AssimpModelLoader() noexcept;
        DLLEXPORT void setVertexLayout(const bgfx::VertexLayout& vertexLayout) noexcept;
		DLLEXPORT result_type operator()(std::string_view name) override;
    private:
        std::unique_ptr<AssimpModelLoaderImpl> _impl;
    };
}