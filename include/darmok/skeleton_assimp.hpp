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
        Result operator()(std::filesystem::path path) override;
    private:
        std::unique_ptr<AssimpSkeletonLoaderImpl> _impl;
    };

    class AssimpSkeletonFileImporterImpl;

    class DARMOK_EXPORT AssimpSkeletonFileImporter final : public IFileTypeImporter
    {
    public:
        AssimpSkeletonFileImporter() noexcept;
        ~AssimpSkeletonFileImporter() noexcept;
        std::vector<std::filesystem::path> getOutputs(const Input& input) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        const std::string& getName() const noexcept override;
    private:
        std::unique_ptr<AssimpSkeletonFileImporterImpl> _impl;
    };

    class AssimpSkeletalAnimationFileImporterImpl;

    class DARMOK_EXPORT AssimpSkeletalAnimationFileImporter final : public IFileTypeImporter
    {
    public:
        AssimpSkeletalAnimationFileImporter() noexcept;
        ~AssimpSkeletalAnimationFileImporter() noexcept;
        void setLogOutput(OptionalRef<std::ostream> log) noexcept override;
        bool startImport(const Input& input, bool dry) override;
        std::vector<std::filesystem::path> getOutputs(const Input& input) override;
        void endImport(const Input& input) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        const std::string& getName() const noexcept override;
    private:
        std::unique_ptr<AssimpSkeletalAnimationFileImporterImpl> _impl;
    };
}