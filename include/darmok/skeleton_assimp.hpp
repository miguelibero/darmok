#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/skeleton.hpp>

struct aiMesh;

namespace darmok
{
    class AssimpSkeletonLoaderImpl;

    class DARMOK_EXPORT AssimpSkeletonLoader final : public ISkeletonLoader
    {
    public:
        AssimpSkeletonLoader(IDataLoader& dataLoader) noexcept;
        ~AssimpSkeletonLoader() noexcept;
        Result operator()(std::filesystem::path path) noexcept override;
    private:
        std::unique_ptr<AssimpSkeletonLoaderImpl> _impl;
    };

    class AssimpSkeletonFileImporterImpl;

    class DARMOK_EXPORT AssimpSkeletonFileImporter final : public IFileTypeImporter
    {
    public:
        AssimpSkeletonFileImporter() noexcept;
        ~AssimpSkeletonFileImporter() noexcept;
        const std::string& getName() const noexcept override;

        expected<Effect, std::string> prepare(const Input& input) noexcept override;
        expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;
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
        const std::string& getName() const noexcept override;

        expected<Effect, std::string> prepare(const Input& input) noexcept override;
        expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;
    private:
        std::unique_ptr<AssimpSkeletalAnimationFileImporterImpl> _impl;
    };

    class DARMOK_EXPORT AssimpArmatureDefinitionConverter final
    {
    public:
        using Definition = protobuf::Armature;

        AssimpArmatureDefinitionConverter(const aiMesh& assimpMesh, Definition& def) noexcept;
        AssimpArmatureDefinitionConverter& setBoneNames(const std::vector<std::string>& names) noexcept;
        AssimpArmatureDefinitionConverter& setBoneNames(const std::unordered_map<std::string, std::string>& names) noexcept;
        expected<void, std::string> operator()() noexcept;
    private:
        const aiMesh& _assimpMesh;
        Definition& _def;
        std::unordered_map<std::string, std::string> _boneNames;
    };
}