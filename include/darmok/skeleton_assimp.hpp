#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/skeleton.hpp>

namespace darmok
{
    class AssimpSkeletonLoaderImpl;

    class DARMOK_EXPORT AssimpSkeletonLoader final : public ISkeletonLoader
    {
    public:
        AssimpSkeletonLoader(IDataLoader& dataLoader) noexcept;
        ~AssimpSkeletonLoader() noexcept;
        result_type operator()(std::string_view name) override;
    private:
        std::unique_ptr<AssimpSkeletonLoaderImpl> _impl;
    };

    class AssimpSkeletonImporterImpl;

    class DARMOK_EXPORT AssimpSkeletonImporter final : public IAssetTypeImporter
    {
    public:
        AssimpSkeletonImporter() noexcept;
        ~AssimpSkeletonImporter() noexcept;
        size_t getOutputs(const Input& input, const std::filesystem::path& basePath, std::vector<std::filesystem::path>& outputs) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        std::string getName() const noexcept override;
    private:
        std::unique_ptr<AssimpSkeletonImporterImpl> _impl;
    };
}