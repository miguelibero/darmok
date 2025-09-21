#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>

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

    struct DARMOK_EXPORT FileTypeImporterInput final
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

    using FileTypeImportDependencies = std::unordered_set<std::filesystem::path>;

    class DARMOK_EXPORT BX_NO_VTABLE IFileTypeImporter
    {
    public:
        using Input = FileTypeImporterInput;
        using Dependencies = FileTypeImportDependencies;
        using Outputs = std::vector<std::filesystem::path>;

        virtual ~IFileTypeImporter() = default;
        virtual void setLogOutput(OptionalRef<std::ostream> log) noexcept {};

        virtual bool startImport(const Input& input, bool dry = false) { return true; };

        virtual Outputs getOutputs(const Input& input) = 0;
        virtual Dependencies getDependencies(const Input& input) { return {}; };

        // outputIndex is the index of the element in the vector returned by getOutputs
        virtual std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& outputPath)
        {
            return std::ofstream(outputPath, std::ios::binary);
        }

        virtual void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) = 0;
        virtual void endImport(const Input& input) { };
        virtual const std::string& getName() const noexcept = 0;
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
        BaseCommandLineFileImporter() noexcept;
        virtual ~BaseCommandLineFileImporter() noexcept;
        int operator()(const CmdArgs& args) noexcept;
        static void setup(CLI::App& cli, Config& cfg, bool required = false) noexcept;


    protected:
        virtual std::vector<std::filesystem::path> getOutputs(const Config& config) const = 0;
        virtual void import(const Config& config, std::ostream& log) const = 0;
    private:
        friend class CommandLineFileImporterImpl;
        std::unique_ptr<CommandLineFileImporterImpl> _impl;
    };

    class FileImporterImpl;

    class DARMOK_EXPORT FileImporter final
    {
    public:
        FileImporter(const std::filesystem::path& inputPath);
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

        std::vector<std::filesystem::path> getOutputs() const noexcept;
		void operator()(std::ostream& log) const;
	private:
		std::unique_ptr<FileImporterImpl> _impl;
    };

    class ProgramFileImporter;

    class DARMOK_EXPORT DarmokCoreAssetFileImporter final
    {
    public:
        DarmokCoreAssetFileImporter(const CommandLineFileImporterConfig& config);
        DarmokCoreAssetFileImporter(const std::filesystem::path& inputPath);
        DarmokCoreAssetFileImporter& setCachePath(const std::filesystem::path& cachePath) noexcept;
        DarmokCoreAssetFileImporter& setOutputPath(const std::filesystem::path& outputPath) noexcept;
        DarmokCoreAssetFileImporter& setShadercPath(const std::filesystem::path& path) noexcept;
        DarmokCoreAssetFileImporter& addShaderIncludePath(const std::filesystem::path& path) noexcept;
        std::vector<std::filesystem::path> getOutputs() const;
        void operator()(std::ostream& log) const;
    private:
        FileImporter _importer;
        ProgramFileImporter& _progImporter;
    };

    class DARMOK_EXPORT CopyFileImporter final : public IFileTypeImporter
    {
    public:
        CopyFileImporter(size_t bufferSize = 4096) noexcept;

        std::vector<std::filesystem::path> getOutputs(const Input& input) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        const std::string& getName() const noexcept override;
    private:
        size_t _bufferSize;
    };
}