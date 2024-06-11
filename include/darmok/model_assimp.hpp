#pragma once

#include <darmok/export.h>
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

    class DARMOK_EXPORT AssimpModelLoader final : public IModelLoader
    {
    public:
        AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader = nullptr) noexcept;
        ~AssimpModelLoader() noexcept;
        void setVertexLayout(const bgfx::VertexLayout& vertexLayout) noexcept;
		result_type operator()(std::string_view name) override;
    private:
        std::unique_ptr<AssimpModelLoaderImpl> _impl;
    };
}