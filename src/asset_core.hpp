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
        nlohmann::json _importersConfig;
        std::filesystem::path _outputPath;
        std::string _headerVarPrefix;
        bool _produceHeaders;
        std::filesystem::path _headerIncludeDir;
        std::unordered_map<std::string, std::unique_ptr<IAssetTypeImporter>> _importers;
        static const std::string _globalConfigFileSuffix;
        static const std::string _inputConfigFileSuffix;
        static const std::string _globalConfigFilesKey;
        static const std::string _globalConfigImportersKey;
        static const std::string _globalConfigHeaderVarPrefixKey;
        static const std::string _globalConfigProduceHeadersKey;
        static const std::string _globalConfigHeaderIncludeDirKey;
        static const std::string _globalConfigOutputPathKey;

        using ImporterInputs = std::vector<std::pair<OptionalRef<IAssetTypeImporter>, Input>>;
        ImporterInputs getImporterInputs() const;
        size_t getOutputs(IAssetTypeImporter& importer, const Input& input, std::vector<std::filesystem::path>& outputs) const;
        std::vector<std::filesystem::path> importFile(IAssetTypeImporter& importer, const Input& input, std::ostream& log) const;
        std::filesystem::path getHeaderPath(const std::filesystem::path& path, const std::string& baseName) const noexcept;
        std::filesystem::path getHeaderPath(const std::filesystem::path& path) const noexcept;
        bool loadInput(const std::filesystem::path& path);
        bool loadGlobalConfig(const std::filesystem::path& path);
        const nlohmann::json& loadInputConfig(const std::filesystem::path& path, const nlohmann::json& json) noexcept;
        static nlohmann::json fixInputConfig(const nlohmann::json& json) noexcept;
        static void mergeJsonObjects(nlohmann::json& json1, const nlohmann::json& json2) noexcept;
        using PathGroups = std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>;
        PathGroups getPathGroups(const std::vector<std::filesystem::path>& paths) const noexcept;
        void produceCombinedHeader(const std::filesystem::path& path, const std::vector<std::filesystem::path>& paths) const;
    };

    class ShaderImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        ShaderImporterImpl(size_t bufferSize = 4096) noexcept;
        void setShadercPath(const std::filesystem::path& path) noexcept;
        void addIncludePath(const std::filesystem::path& path) noexcept;
        void setLogOutput(OptionalRef<std::ostream> log) noexcept;
        bool getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs);
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        std::string getName() const noexcept;

    private:
        size_t _bufferSize;
        std::filesystem::path _shadercPath;
        OptionalRef<std::ostream> _log;
        std::vector<std::filesystem::path> _includes;
        static const std::string _binExt;
        static const std::vector<std::string> _profiles;
        static const std::unordered_map<std::string, std::string> _profileExtensions;
        static const std::string _configTypeKey;
        static const std::string _configVaryingDefKey;
        static const std::string _configIncludeDirsKey;


        std::filesystem::path getOutputPath(const Input& input, const std::string& ext) noexcept;
        static std::string fixPathArgument(const std::filesystem::path& path) noexcept;

    };
}