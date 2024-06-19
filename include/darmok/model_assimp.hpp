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
        AssimpModelLoader& setConfig(const Config& config, bool force = false) noexcept;
		result_type operator()(std::string_view name) override;
    private:
        std::unique_ptr<AssimpModelLoaderImpl> _impl;
    };

    struct DARMOK_EXPORT AssimpModelProcessConfig final
    {
        using OutputFormat = AssimpModelProcessorOutputFormat;

        AssimpModelLoadConfig loadConfig;
        OutputFormat outputFormat = OutputFormat::Binary;

        void load(const nlohmann::ordered_json& config);
    };

    class AssimpModelProcessorImpl;

    class DARMOK_EXPORT AssimpModelProcessor final : public IAssetTypeProcessor
    {
    public:
        using Config = AssimpModelProcessConfig;
        using OutputFormat = AssimpModelProcessorOutputFormat;
        AssimpModelProcessor();
        ~AssimpModelProcessor();

        std::shared_ptr<Model> read(const std::filesystem::path& input) const;
        AssimpModelProcessor& setConfig(Config& config, bool force = false) noexcept;

        bool getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) override;
        std::ofstream createOutputStream(const std::filesystem::path& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) override;
        std::string getName() const noexcept override;
    private:
        std::unique_ptr<AssimpModelProcessorImpl> _impl;
    };
}