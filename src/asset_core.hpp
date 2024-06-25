#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <regex>

#include <darmok/vertex_layout.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/optional_ref.hpp>
#include <nlohmann/json.hpp>

namespace bx
{
    class CommandLine;
}

namespace darmok
{
    class AssetImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;

        AssetImporterImpl(const std::filesystem::path& inputPath);
        void setCachePath(const std::filesystem::path& cachePath) noexcept;
        void setOutputPath(const std::filesystem::path& outputPath) noexcept;
        std::vector<std::filesystem::path> getOutputs() const;
        void addTypeImporter(std::unique_ptr<IAssetTypeImporter>&& importer) noexcept;
        void operator()(std::ostream& log) const;
	private:
        std::filesystem::path _inputPath;
        std::filesystem::path _outputPath;
        std::filesystem::path _cachePath;

        struct FileCacheData final
        {
            std::time_t updateTime;
            std::time_t cacheTime;
        };

        mutable std::unordered_map<std::filesystem::path, FileCacheData> _fileCache;
        mutable std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>> _fileDependencies;
        std::unordered_map<std::filesystem::path, nlohmann::json> _inputs;
        nlohmann::json _importersConfig;
        std::string _headerVarPrefix;
        bool _produceHeaders;
        std::filesystem::path _headerIncludeDir;
        std::unordered_map<std::string, std::unique_ptr<IAssetTypeImporter>> _importers;
        static const std::string _globalConfigFile;
        static const std::string _inputConfigFileSuffix;
        static const std::string _globalConfigFilesKey;
        static const std::string _globalConfigImportersKey;
        static const std::string _globalConfigHeaderVarPrefixKey;
        static const std::string _globalConfigProduceHeadersKey;
        static const std::string _globalConfigHeaderIncludeDirKey;
        static const std::string _globalConfigOutputPathKey;

        using ImporterInputs = std::vector<std::pair<OptionalRef<IAssetTypeImporter>, Input>>;
        ImporterInputs getImporterInputs() const;
        void loadDependencies(const ImporterInputs& importerInputs) const;
        void getDependencies(const std::filesystem::path& path, const ImporterInputs& importerInputs, std::vector<std::filesystem::path>& deps) const;
        std::vector<std::filesystem::path> getOutputs(IAssetTypeImporter& importer, const Input& input) const;

        struct FileImportResult final
        {
            std::vector<std::filesystem::path> outputs;
            bool inputCached;
            std::vector<std::filesystem::path> updatedOutputs;
        };

        std::filesystem::path getGlobalConfigPath() const noexcept;
        std::filesystem::path getInputConfigPath(const std::filesystem::path& path) const noexcept;
        bool addFileCachePath(const std::filesystem::path& path, std::time_t cacheTime = 0) const noexcept;
        FileImportResult importFile(IAssetTypeImporter& importer, const Input& input, std::ostream& log) const;
        std::filesystem::path getHeaderPath(const std::filesystem::path& path, const std::string& baseName) const noexcept;
        std::filesystem::path getHeaderPath(const std::filesystem::path& path) const noexcept;
        bool loadInput(const std::filesystem::path& path);
        bool loadGlobalConfig(const std::filesystem::path& path, const std::vector<std::filesystem::path>& inputPaths);
        const nlohmann::json& loadInputConfig(const std::filesystem::path& path, const nlohmann::json& json) noexcept;
        static nlohmann::json fixInputConfig(const nlohmann::json& json) noexcept;
        static void mergeJsonObjects(nlohmann::json& json1, const nlohmann::json& json2) noexcept;
        using PathGroups = std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>;
        PathGroups getPathGroups(const std::vector<std::filesystem::path>& paths) const noexcept;
        void produceCombinedHeader(const std::filesystem::path& path, const std::vector<std::filesystem::path>& paths) const;
        static std::time_t getUpdateTime(const std::filesystem::path& path);
        
        static std::filesystem::path normalizePath(const std::filesystem::path& path) noexcept;
        bool isCached(const std::filesystem::path& path) const noexcept;
        bool isPathCached(const std::filesystem::path& path) const noexcept;
        bool isCacheUpdated() const noexcept;
        bool writeCache() const;
    };

    class BaseCommandLineAssetImporter;

    class CommandLineAssetImporterImpl final
    {
    public:
        using Config = CommandLineAssetImporterConfig;
        CommandLineAssetImporterImpl(BaseCommandLineAssetImporter& importer) noexcept;
        int operator()(int argc, const char* argv[]) noexcept;
    private:
        BaseCommandLineAssetImporter& _importer;
        void version(const std::string& name) noexcept;
        void help(const std::string& name, const char* error = nullptr) noexcept;
        int run(const std::string& name, const bx::CommandLine cmdLine);
    };

    class ShaderImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        ShaderImporterImpl(size_t bufferSize = 4096) noexcept;
        void setShadercPath(const std::filesystem::path& path) noexcept;
        void addIncludePath(const std::filesystem::path& path) noexcept;
        void setLogOutput(OptionalRef<std::ostream> log) noexcept;
        std::vector<std::filesystem::path> getOutputs(const Input& input);
        std::vector<std::filesystem::path> getDependencies(const Input& input);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        const std::string& getName() const noexcept;

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
        static const std::regex _includeRegex;

        std::vector<std::filesystem::path> getIncludes(const Input& input) const noexcept;
        static std::filesystem::path getOutputPath(const std::filesystem::path& path, const std::string& ext) noexcept;
        static std::string fixPathArgument(const std::filesystem::path& path) noexcept;
    };
}