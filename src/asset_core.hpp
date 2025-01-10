#pragma once

#include <darmok/asset_core.hpp>
#include <darmok/optional_ref.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <regex>

#include <nlohmann/json.hpp>

namespace darmok
{
    class FileImporterImpl final
    {
    public:
        using Input = FileTypeImporterInput;
        using Dependencies = FileTypeImportDependencies;

        FileImporterImpl(const std::filesystem::path& inputPath);
        void setCachePath(const std::filesystem::path& cachePath) noexcept;
        void setOutputPath(const std::filesystem::path& outputPath) noexcept;
        std::vector<std::filesystem::path> getOutputs() const;
        void addTypeImporter(std::unique_ptr<IFileTypeImporter>&& importer) noexcept;
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
        mutable std::unordered_map<std::filesystem::path, Dependencies> _fileDependencies;

        struct HeaderConfig final
        {
            bool produceHeaders;
            std::string varPrefix;
            std::filesystem::path includeDir;

            bool load(const nlohmann::json& json) noexcept;
        private:
            static const std::string _headerVarPrefixKey;
            static const std::string _produceHeadersKey;
            static const std::string _headerIncludeDirKey;
        };

        struct Operation final
        {
            IFileTypeImporter& importer;
            Input input;
            HeaderConfig headerConfig;
            std::filesystem::path outputPath;
        };

        struct DirConfig final
        {
            std::filesystem::path path;
            nlohmann::json importers;
            std::unordered_map<std::filesystem::path, nlohmann::json> files;
            std::unordered_map<std::filesystem::path, std::vector<std::string>> fileMatches;
            std::optional<HeaderConfig> header;
            std::optional<std::filesystem::path> outputPath;

            bool load(const std::filesystem::path& inputPath, const std::vector<std::filesystem::path>& filePaths);
            static std::filesystem::path getPath(const std::filesystem::path& path) noexcept;
            static bool isPath(const std::filesystem::path& path) noexcept;

            void updateOperation(Operation& op, const std::string& importerName) const;

        private:
            static const std::string _configFileName;
            static const std::string _filesKey;
            static const std::string _importersKey;
            static const std::string _includesKey;
            static const std::string _outputPathKey;

            void loadFile(const std::string& key, const nlohmann::json& config, const std::filesystem::path& basePath, const std::vector<std::filesystem::path>& filePaths) noexcept;
        };
        
        std::unordered_map<std::filesystem::path, DirConfig> _dirs;


        struct FileConfig final
        {
            std::filesystem::path path;
            nlohmann::json importers;
            bool load(const std::filesystem::path& inputPath);
            void load(const nlohmann::json& json);
            static nlohmann::json fix(const nlohmann::json& json) noexcept;
            static bool replaceIncludes(nlohmann::json& json, const nlohmann::json& includes) noexcept;
            static std::filesystem::path getPath(const std::filesystem::path& path) noexcept;
            static bool isPath(const std::filesystem::path& path) noexcept;
        private:
            static const std::string _configFileSuffix;
            static const std::string _importersKey;
            static const std::string _includesKey;
            static const std::regex _includePattern;
            static const std::string _includePatternToken;
        };

        std::unordered_map<std::filesystem::path, FileConfig> _files;
        std::unordered_map<std::string, std::unique_ptr<IFileTypeImporter>> _importers;

        std::vector<Operation> getOperations() const;
        static void mergeConfig(nlohmann::json& json, const nlohmann::json& other);
        void loadDependencies(const std::vector<Operation>& ops) const;
        void getDependencies(const std::filesystem::path& path, const std::vector<Operation>& ops, Dependencies& deps) const;
        std::filesystem::path fixOutput(const std::filesystem::path& path, const Operation& op) const noexcept;
        std::vector<std::filesystem::path> getOutputs(const Operation& op) const;

        struct FileImportResult final
        {
            std::vector<std::filesystem::path> outputs;
            bool inputCached;
            std::vector<std::filesystem::path> updatedOutputs;
        };

        using DirConfigs = std::vector<OptionalRef<const DirConfig>>;
        DirConfigs getDirConfigs(const std::filesystem::path& path) const noexcept;
        bool addFileCachePath(const std::filesystem::path& path, std::time_t cacheTime = 0) const noexcept;
        FileImportResult importFile(const Operation& op, std::ostream& log) const;
        std::filesystem::path getHeaderPath(const std::filesystem::path& path, const std::string& baseName) const noexcept;
        std::filesystem::path getHeaderPath(const std::filesystem::path& path) const noexcept;
        bool loadInput(const std::filesystem::path& path, const std::vector<std::filesystem::path>& paths);
        using PathGroups = std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>;
        PathGroups getPathGroups(const std::vector<std::filesystem::path>& paths) const noexcept;
        void produceCombinedHeader(const std::filesystem::path& path, const std::vector<std::filesystem::path>& paths, const std::filesystem::path& includeDir) const;
        static std::time_t getUpdateTime(const std::filesystem::path& path);
        
        static std::filesystem::path normalizePath(const std::filesystem::path& path) noexcept;
        bool isCached(const std::filesystem::path& path) const noexcept;
        bool isPathCached(const std::filesystem::path& path) const noexcept;
        bool isCacheUpdated() const noexcept;
        bool writeCache() const;
    };

    class BaseCommandLineFileImporter;

    class CommandLineFileImporterImpl final
    {
    public:
        using Config = CommandLineFileImporterConfig;
        CommandLineFileImporterImpl(BaseCommandLineFileImporter& importer) noexcept;
        int operator()(const CmdArgs& args) noexcept;
        static void setup(CLI::App& cli, Config& cfg, bool required) noexcept;
    private:
        BaseCommandLineFileImporter& _importer;

    };
}