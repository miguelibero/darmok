#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/varying.hpp>
#include <darmok/program.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/protobuf/assimp.pb.h>

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

struct aiScene;

namespace darmok
{
    class IDataLoader;
    class ITextureSourceLoader;
    class IProgramSourceLoader;
    class IImageLoader;
    class IProgramLoader;
    class Image;
    class Data;
    class VertexDataWriter;

    namespace protobuf
    {
		class AssimpSceneImportConfig;
		class Scene;
    }

    class AssimpSceneDefinitionConverterImpl;

    class AssimpSceneDefinitionConverter final
    {
    public:
        using ImportConfig = protobuf::AssimpSceneImportConfig;
        using Definition = protobuf::Scene;

        AssimpSceneDefinitionConverter(const aiScene& assimpScene, Definition& sceneDef, const ImportConfig& config,
            bx::AllocatorI& alloc, OptionalRef<ITextureSourceLoader> texLoader = nullptr, OptionalRef<IProgramSourceLoader> progLoader = nullptr) noexcept;
        ~AssimpSceneDefinitionConverter() noexcept;
        static std::vector<std::string> getDependencies(const aiScene& scene) noexcept;
        expected<void, std::string> operator()() noexcept;
    private:
        std::unique_ptr<AssimpSceneDefinitionConverterImpl> _impl;
    };

    class AssimpSceneDefinitionLoaderImpl;

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
        AssimpSceneFileImporter(bx::AllocatorI& alloc) noexcept;
        ~AssimpSceneFileImporter() noexcept;
        
        const std::string& getName() const noexcept override;
        expected<Effect, std::string> prepare(const Input& input) noexcept override;
        expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;

        AssimpSceneFileImporter& setShadercPath(const std::filesystem::path& path) noexcept;
        AssimpSceneFileImporter& addIncludePath(const std::filesystem::path& path) noexcept;
    private:
        std::unique_ptr<AssimpSceneFileImporterImpl> _impl;
    };
}