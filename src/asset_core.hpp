#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <darmok/vertex_layout.hpp>
#include <darmok/asset_core.hpp>

namespace darmok
{
    class AssetImporterImpl final
    {
    public:
        AssetImporterImpl(const std::filesystem::path& inputPath);
        void setOutputPath(const std::filesystem::path& outputPath) noexcept;
        void setHeaderVarPrefix(const std::string& prefix) noexcept;
        void setProduceHeaders(bool headers) noexcept;
        std::vector<std::filesystem::path> getOutputs() const;
        void addTypeImporter(std::unique_ptr<IAssetTypeImporter>&& importer) noexcept;
        void operator()(std::ostream& log) const;
	private:
        std::filesystem::path _inputPath;
        std::filesystem::path _outputPath;
        std::string _headerVarPrefix;
        bool _produceHeaders;
        std::vector< std::unique_ptr<IAssetTypeImporter>> _importers;

        std::vector<std::filesystem::path> getInputs() const noexcept;
        size_t getOutputs(IAssetTypeImporter& processor, const std::filesystem::path& path, std::vector<std::filesystem::path>& outputs) const;
        size_t processFile(IAssetTypeImporter& processor, const std::filesystem::path& path, std::ostream& log) const;
        std::filesystem::path getHeaderPath(const std::filesystem::path& path, const std::string& baseName) const noexcept;
        std::filesystem::path getHeaderPath(const std::filesystem::path& path) const noexcept;
    };

    class ShaderAssetImporterImpl final
    {
    public:
        ShaderAssetImporterImpl();
        bool getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) const;
        std::ofstream createOutputStream(size_t outputIndex, const std::filesystem::path& path) const;
        void writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) const;
        std::string getName() const noexcept;
    private:
    };
}