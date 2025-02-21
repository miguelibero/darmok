#pragma once

#include <darmok/export.h>
#include <darmok/model.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/varying.hpp>
#include <memory>
#include <string>
#include <filesystem>
#include <regex>
#include <vector>
#include <unordered_set>

#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/string.hpp>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    using ProgramDefines = std::unordered_set<std::string>;

    struct DARMOK_EXPORT AssimpModelImportConfig final
    {
        using Program = std::variant<StandardProgramType, std::shared_ptr<ProgramDefinition>>;
        Program program = StandardProgramType::Unlit;
        ProgramDefines programDefines;
        std::vector<std::regex> skipMeshes;
        std::vector<std::regex> skipNodes;
        bgfx::VertexLayout vertexLayout;
        bool embedTextures = true;
        std::string defaultTexture;
        std::optional<std::regex> rootMesh;
        std::optional<OpacityType> opacity;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(program),
                CEREAL_NVP(programDefines),
                CEREAL_NVP(skipMeshes),
                CEREAL_NVP(skipNodes),
                CEREAL_NVP(vertexLayout),
                CEREAL_NVP(embedTextures),
                CEREAL_NVP(defaultTexture),
                CEREAL_NVP(rootMesh),
                CEREAL_NVP(opacity)
            );
        }
    };

    struct DARMOK_EXPORT AssimpModelSource final
    {
        using Config = AssimpModelImportConfig;
        std::string name;
        Data data;
        Config config;
        std::unordered_map<std::string, TextureSource> textures;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(name),
                CEREAL_NVP(data),
                CEREAL_NVP(config)
            );
        }
    };

    class ITextureDefinitionLoader;

    class DARMOK_EXPORT AssimpModelImporter final
    {
    public:
        AssimpModelImporter(bx::AllocatorI& alloc, OptionalRef<ITextureDefinitionLoader> texLoader = nullptr) noexcept;
        Model operator()(const AssimpModelSource& src);
    private:
        bx::AllocatorI& _alloc;
        OptionalRef<ITextureDefinitionLoader> _texLoader;
    };

    class IDataLoader;
    class IImageLoader;
    class AssimpModelLoaderImpl;

    class DARMOK_EXPORT AssimpModelLoader final : public IModelLoader
    {
    public:
        using Config = AssimpModelImportConfig;
        AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<ITextureDefinitionLoader> texLoader = nullptr) noexcept;
        ~AssimpModelLoader() noexcept;
        AssimpModelLoader& setConfig(const Config& config) noexcept;
        bool supports(const std::filesystem::path& path) const noexcept;
		std::shared_ptr<Model> operator()(std::filesystem::path path) override;
    private:
        std::unique_ptr<AssimpModelLoaderImpl> _impl;
    };

    class AssimpModelFileImporterImpl;

    class DARMOK_EXPORT AssimpModelFileImporter final : public IFileTypeImporter
    {
    public:
        using Config = AssimpModelImportConfig;
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