#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/varying.hpp>
#include <darmok/program.hpp>
#include <darmok/loader.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/model.pb.h>
#include <darmok/protobuf/scene.pb.h>

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
    class ITextureDefinitionLoader;

    class DARMOK_EXPORT AssimpModelImporter final
    {
    public:
        using Model = protobuf::Scene;
        using Source = protobuf::AssimpModelSource;
        AssimpModelImporter(bx::AllocatorI& alloc, OptionalRef<ITextureDefinitionLoader> texLoader = nullptr) noexcept;
        Model operator()(const Source& src);
    private:
        bx::AllocatorI& _alloc;
        OptionalRef<ITextureDefinitionLoader> _texLoader;
    };

    class IDataLoader;
    class AssimpModelLoaderImpl;

    class DARMOK_EXPORT AssimpModelLoader final : public ILoader<protobuf::Scene>
    {
    public:
        using Config = protobuf::AssimpModelImportConfig;
        using Model = protobuf::Scene;

        AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<ITextureDefinitionLoader> texLoader = nullptr) noexcept;
        ~AssimpModelLoader() noexcept;
        AssimpModelLoader& setConfig(const Config& config) noexcept;
        bool supports(const std::filesystem::path& path) const noexcept;
		Result operator()(std::filesystem::path path) override;
    private:
        std::unique_ptr<AssimpModelLoaderImpl> _impl;
    };

    class AssimpModelFileImporterImpl;

    class DARMOK_EXPORT AssimpModelFileImporter final : public IFileTypeImporter
    {
    public:
        using Config = protobuf::AssimpModelImportConfig;
        AssimpModelFileImporter(bx::AllocatorI& alloc);
        ~AssimpModelFileImporter();
        AssimpModelFileImporter& setProgramVertexLayoutSuffix(const std::string& suffix);
        bool startImport(const Input& input, bool dry = false) override;
        std::vector<std::filesystem::path> getOutputs(const Input& input) override;
        Dependencies getDependencies(const Input& input) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        void endImport(const Input& input) override;
        const std::string& getName() const noexcept override;
    private:
        std::unique_ptr<AssimpModelFileImporterImpl> _impl;
    };
}