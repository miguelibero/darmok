#pragma once

#include <string>
#include <vector>
#include <vector>
#include <iostream>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <darmok/vertex_layout.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/optional_ref.hpp>
#include <nlohmann/json.hpp>

namespace darmok
{
    class AssetImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;

        AssetImporterImpl(const std::filesystem::path& inputPath);
        void setOutputPath(const std::filesystem::path& outputPath) noexcept;
        std::vector<std::filesystem::path> getOutputs() const;
        void addTypeImporter(std::unique_ptr<IAssetTypeImporter>&& importer) noexcept;
        void operator()(std::ostream& log) const;
	private:
        std::filesystem::path _inputPath;
        std::unordered_map<std::filesystem::path, nlohmann::json> _inputs;
        std::filesystem::path _outputPath;
        std::string _headerVarPrefix;
        bool _produceHeaders;
        std::unordered_map<std::string, std::unique_ptr<IAssetTypeImporter>> _importers;
        static const std::string _globalConfigFileSuffix;
        static const std::string _inputConfigFileSuffix;
        static const std::string _globalConfigFilesKey;
        static const std::string _globalConfigHeaderVarPrefixKey;
        static const std::string _globalConfigHeadersKey;

        using ImporterInputs = std::vector<std::pair<OptionalRef<IAssetTypeImporter>, Input>>;
        ImporterInputs getImporterInputs() const;
        size_t getOutputs(IAssetTypeImporter& importer, const Input& input, std::vector<std::filesystem::path>& outputs) const;
        size_t importFile(IAssetTypeImporter& importer, const Input& input, std::ostream& log) const;
        std::filesystem::path getHeaderPath(const std::filesystem::path& path, const std::string& baseName) const noexcept;
        std::filesystem::path getHeaderPath(const std::filesystem::path& path) const noexcept;
        bool loadInput(const std::filesystem::path& path);
        bool loadGlobalConfig(const std::filesystem::path& path);
        const nlohmann::json& loadInputConfig(const std::filesystem::path& path, const nlohmann::json& json) noexcept;
    };

    class ShaderAssetImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        ShaderAssetImporterImpl();
        bool getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs);
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        std::string getName() const noexcept;
    private:
    };
}