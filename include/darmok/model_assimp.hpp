#pragma once

#include <darmok/export.h>
#include <darmok/model.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/optional_ref.hpp>
#include <memory>
#include <string>
#include <filesystem>

namespace bx
{
    struct AllocatorI;
}

namespace bgfx
{
    struct VertexLayout;
}

namespace darmok
{
    class IDataLoader;
    class IImageLoader;
    class AssimpModelLoaderImpl;

    class DARMOK_EXPORT AssimpModelLoader final : public IModelLoader
    {
    public:
        AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader = nullptr) noexcept;
        ~AssimpModelLoader() noexcept;
        void setVertexLayout(const bgfx::VertexLayout& vertexLayout) noexcept;
		result_type operator()(std::string_view name) override;
    private:
        std::unique_ptr<AssimpModelLoaderImpl> _impl;
    };

    enum class AssimpModelProcessorOutputFormat
    {
        Binary,
        Json,
        Xml
    };

    struct AssimpModelProcessorConfig final
    {
        bgfx::VertexLayout vertexLayout;
        bool embedTextures;

        AssimpModelProcessorConfig() noexcept;
        bool loadForModel(const std::filesystem::path& path);
        void load(const nlohmann::ordered_json& config);

    private:
        static const char* _vertexLayoutJsonKey;
        static const char* _embedTexturesJsonKey;

        static bgfx::VertexLayout loadVertexLayout(const nlohmann::ordered_json& json);
    };

    class DARMOK_EXPORT AssimpModelProcessor final : public IAssetTypeProcessor
    {
    public:
        using Config = AssimpModelProcessorConfig;
        using OutputFormat = AssimpModelProcessorOutputFormat;
        AssimpModelProcessor();
        std::shared_ptr<Model> read(const std::filesystem::path& input) const;
        bool getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) const override;
        std::ofstream createOutputStream(size_t outputIndex, const std::filesystem::path& path) const override;
        void writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) const override;
        std::string getName() const noexcept override;
    private:

        OutputFormat _outputFormat;

        bgfx::VertexLayout loadVertexLayout(const nlohmann::ordered_json& json);
        static std::filesystem::path getFilename(const std::filesystem::path& path, OutputFormat format) noexcept;
    };
}