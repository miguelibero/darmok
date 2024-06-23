#pragma once

#include <darmok/export.h>
#include <darmok/model.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/program_standard.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/model_assimp_fwd.hpp>
#include <memory>
#include <string>
#include <filesystem>
#include <bgfx/bgfx.h>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    struct DARMOK_EXPORT AssimpModelLoadConfig final
    {
        bgfx::VertexLayout vertexLayout;
        StandardProgramType standardProgram = StandardProgramType::ForwardPhong;
        std::string programName;
        bool embedTextures = true;
    };

    class IDataLoader;
    class IImageLoader;
    class AssimpModelLoaderImpl;

    class DARMOK_EXPORT AssimpModelLoader final : public IModelLoader
    {
    public:
        using Config = AssimpModelLoadConfig;
        AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader = nullptr) noexcept;
        ~AssimpModelLoader() noexcept;
        AssimpModelLoader& setConfig(const Config& config) noexcept;
		result_type operator()(std::string_view name) override;
    private:
        std::unique_ptr<AssimpModelLoaderImpl> _impl;
    };

    class AssimpModelImporterImpl;

    class DARMOK_EXPORT AssimpModelImporter final : public IAssetTypeImporter
    {
    public:
        using LoadConfig = AssimpModelLoadConfig;
        AssimpModelImporter();
        ~AssimpModelImporter();
        AssimpModelImporter& setProgramVertexLayoutSuffix(const std::string& suffix);
        size_t getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        const std::string& getName() const noexcept override;
    private:
        std::unique_ptr<AssimpModelImporterImpl> _impl;
    };
}