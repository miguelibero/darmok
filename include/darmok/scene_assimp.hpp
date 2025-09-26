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
    class ITextureSourceLoader;

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

        AssimpSceneDefinitionLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<ITextureSourceLoader> texLoader = nullptr) noexcept;
        ~AssimpSceneDefinitionLoader() noexcept;
        AssimpSceneDefinitionLoader& setConfig(const Config& config) noexcept;
        bool supports(const std::filesystem::path& path) const noexcept;
		Result operator()(std::filesystem::path path) noexcept override;
    private:
        std::unique_ptr<AssimpSceneDefinitionLoaderImpl> _impl;
    };

    class AssimpSceneFileImporterImpl;

    class DARMOK_EXPORT AssimpSceneFileImporter final : public IFileTypeImporter
    {
    public:
        using Config = FileImportConfig;
        AssimpSceneFileImporter(bx::AllocatorI& alloc);
        ~AssimpSceneFileImporter();
        
        const std::string& getName() const noexcept override;
        expected<Effect, std::string> prepare(const Input& input) noexcept override;
        expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;

        AssimpSceneFileImporter& setShadercPath(const std::filesystem::path& path) noexcept;
        AssimpSceneFileImporter& addIncludePath(const std::filesystem::path& path) noexcept;
    private:
        std::unique_ptr<AssimpSceneFileImporterImpl> _impl;
    };
}