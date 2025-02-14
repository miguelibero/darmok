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

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    using ProgramDefines = std::unordered_set<std::string>;

    struct DARMOK_EXPORT AssimpModelLoadConfig final
    {
        StandardProgramType standardProgram = StandardProgramType::Unlit;
        std::string program;
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
                CEREAL_NVP(standardProgram),
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
        using Config = AssimpModelLoadConfig;
        std::string name;
        Data data;
        Config config;

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
    class AssimpModelImporterImpl;

    class DARMOK_EXPORT AssimpModelImporter final
    {
    public:
        using Config = ProgramCompilerConfig;
        AssimpModelImporter(OptionalRef<ITextureDefinitionLoader> texLoader = nullptr) noexcept;
        ~AssimpModelImporter() noexcept;
        Model operator()(const AssimpModelSource& src);
    private:
        std::unique_ptr<AssimpModelImporterImpl> _impl;
    };

    class IDataLoader;
    class IImageLoader;
    class AssimpModelLoaderImpl;

    class DARMOK_EXPORT AssimpModelLoader final : public IModelLoader
    {
    public:
        using Config = AssimpModelLoadConfig;
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
        using LoadConfig = AssimpModelLoadConfig;
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