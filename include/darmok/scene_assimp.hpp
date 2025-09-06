#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/varying.hpp>
#include <darmok/program.hpp>
#include <darmok/scene_serialize.hpp>

#include <memory>
#include <string>
#include <filesystem>
#include <regex>
#include <vector>
#include <unordered_set>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class IDataLoader;
    class AssimpSceneDefinitionLoaderImpl;
    class ITextureDefinitionLoader;

    namespace protobuf
    {
		class AssimpSceneImportConfig;
		class Scene;
    }

    class DARMOK_EXPORT AssimpSceneDefinitionLoader final : public ISceneDefinitionLoader
    {
    public:
        using Config = protobuf::AssimpSceneImportConfig;
        using Model = protobuf::Scene;

        AssimpSceneDefinitionLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<ITextureDefinitionLoader> texLoader = nullptr) noexcept;
        ~AssimpSceneDefinitionLoader() noexcept;
        AssimpSceneDefinitionLoader& setConfig(const Config& config) noexcept;
        bool supports(const std::filesystem::path& path) const noexcept;
		Result operator()(std::filesystem::path path) override;
    private:
        std::unique_ptr<AssimpSceneDefinitionLoaderImpl> _impl;
    };

    class AssimpSceneFileImporterImpl;

    class DARMOK_EXPORT AssimpSceneFileImporter final : public IFileTypeImporter
    {
    public:
        using Config = protobuf::AssimpSceneImportConfig;
        AssimpSceneFileImporter(bx::AllocatorI& alloc);
        ~AssimpSceneFileImporter();
        bool startImport(const Input& input, bool dry = false) override;
        std::vector<std::filesystem::path> getOutputs(const Input& input) override;
        Dependencies getDependencies(const Input& input) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        void endImport(const Input& input) override;
        const std::string& getName() const noexcept override;
    private:
        std::unique_ptr<AssimpSceneFileImporterImpl> _impl;
    };
}