#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/expected.hpp>

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_set>

#include <bx/bx.h>
#include <nlohmann/json.hpp>

namespace CLI
{
    class App;
}

namespace darmok
{
    using CmdArgs = std::span<const char*>;

    struct DARMOK_EXPORT FileImportInput final
    {
        std::filesystem::path path;
        std::filesystem::path basePath;

        nlohmann::json config;
        nlohmann::json dirConfig;

        std::vector<std::string> pathMatches;

        std::filesystem::path getRelativePath() const noexcept;
        std::filesystem::path getOutputPath(std::string_view defaultExt = "") const noexcept;
        std::optional<const nlohmann::json> getConfigField(std::string_view key) const noexcept;
    };

    struct DARMOK_EXPORT FileImportOutput final
    {
        std::filesystem::path path;
        bool binary = true;
    };

    using FileImportDependencies = std::unordered_set<std::filesystem::path>;

    struct DARMOK_EXPORT FileImportEffect final
    {
        std::vector<FileImportOutput> outputs;
        FileImportDependencies dependencies;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFileImportContext
    {
    };

    struct DARMOK_EXPORT FileImportConfig final
    {
        // empty element if output should be skipped
        std::vector<std::unique_ptr<std::ostream>> outputStreams;
        const IFileImportContext& context;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFileTypeImporter
    {
    public:
        using Input = FileImportInput;
        using Effect = FileImportEffect;
        using Config = FileImportConfig;

        virtual ~IFileTypeImporter() = default;
        virtual const std::string& getName() const noexcept = 0;
        virtual void setLogOutput(OptionalRef<std::ostream> log) noexcept;

        virtual expected<Effect, std::string> prepare(const Input& input) noexcept;
        virtual expected<void, std::string> operator()(const Input& input, Config& config) noexcept;
    };

    class CommandLineFileImporterImpl;

    struct CommandLineFileImporterConfig
    {
        std::filesystem::path inputPath;
        std::filesystem::path outputPath;
        std::filesystem::path cachePath;
        bool dry = false;
        std::filesystem::path shadercPath;
        std::vector<std::filesystem::path> shaderIncludePaths;

        static const std::string defaultInputPath;
        static const std::string defaultOutputPath;
        static const std::string defaultCachePath;

        void fix(const CLI::App& cli) noexcept;
    };

    class DARMOK_EXPORT BaseCommandLineFileImporter
    {
    public:
        using Config = CommandLineFileImporterConfig;
        using Paths = std::vector<std::filesystem::path>;
        BaseCommandLineFileImporter() noexcept;
        virtual ~BaseCommandLineFileImporter() noexcept;
        int operator()(const CmdArgs& args) noexcept;
        static void setup(CLI::App& cli, Config& cfg, bool required = false) noexcept;


    protected:
        virtual expected<Paths, std::string> getOutputPaths(const Config& config) const noexcept = 0;
        virtual bool import(const Config& config, std::ostream& log) const noexcept = 0;
    private:
        friend class CommandLineFileImporterImpl;
        std::unique_ptr<CommandLineFileImporterImpl> _impl;
    };

    class FileImporterImpl;

    class DARMOK_EXPORT FileImporter final
    {
    public:
        using Paths = std::vector<std::filesystem::path>;

        FileImporter(const std::filesystem::path& inputPath) noexcept;
		~FileImporter() noexcept;
        FileImporter& setCachePath(const std::filesystem::path& cachePath) noexcept;
        FileImporter& setOutputPath(const std::filesystem::path& outputPath) noexcept;
        FileImporter& addTypeImporter(std::unique_ptr<IFileTypeImporter>&& importer) noexcept;

        template<typename T, typename... A>
        T& addTypeImporter(A&&... args) noexcept
        {
            auto ptr = new T(std::forward<A>(args)...);
            addTypeImporter(std::unique_ptr<IFileTypeImporter>(ptr));
            return *ptr;
        }

        expected<Paths, std::string> getOutputPaths() const noexcept;
		bool operator()(std::ostream& log) const noexcept;
	private:
		std::unique_ptr<FileImporterImpl> _impl;
    };

    class ProgramFileImporter;

    class DARMOK_EXPORT DarmokCoreAssetFileImporter final
    {
    public:
        using Paths = std::vector<std::filesystem::path>;
        DarmokCoreAssetFileImporter(const CommandLineFileImporterConfig& config);
        DarmokCoreAssetFileImporter(const std::filesystem::path& inputPath);
        DarmokCoreAssetFileImporter& setCachePath(const std::filesystem::path& cachePath) noexcept;
        DarmokCoreAssetFileImporter& setOutputPath(const std::filesystem::path& outputPath) noexcept;
        DarmokCoreAssetFileImporter& setShadercPath(const std::filesystem::path& path) noexcept;
        DarmokCoreAssetFileImporter& addShaderIncludePath(const std::filesystem::path& path) noexcept;
        expected<Paths, std::string> getOutputPaths() const noexcept;
        bool operator()(std::ostream& log) const noexcept;
    private:
        FileImporter _importer;
        ProgramFileImporter& _progImporter;
    };

    class DARMOK_EXPORT CopyFileImporter final : public IFileTypeImporter
    {
    public:
        CopyFileImporter(size_t bufferSize = 4096) noexcept;

        const std::string& getName() const noexcept override;

        expected<Effect, std::string> prepare(const Input& input) noexcept override;
        expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;
    private:
        size_t _bufferSize;
    };
}