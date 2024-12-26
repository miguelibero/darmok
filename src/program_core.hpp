#pragma once

#include <darmok/asset_core.hpp>
#include <darmok/varying.hpp>
#include <regex>
#include <unordered_set>

namespace darmok
{
    using ShaderDefines = std::unordered_set<std::string>;

    enum class ShaderType
    {
        Unknown,
        Vertex,
        Fragment,
        Compute,
    };

    struct ShaderCompilerConfig final
    {
        ProgramCompilerConfig programConfig;
        std::filesystem::path path;
        std::filesystem::path varyingPath;
        ShaderType type;
    };

    struct ShaderCompilerOperation final
    {
        std::string profile;
        ShaderDefines defines;
        std::filesystem::path outputPath;
    };

    class ShaderParser final
    {
    public:
        using IncludePaths = std::unordered_set<std::filesystem::path>;
        using Dependencies = FileTypeImportDependencies;
        using Defines = ShaderDefines;
        using CompilerConfig = ShaderCompilerConfig;
        using CompilerOperation = ShaderCompilerOperation;

        ShaderParser(const IncludePaths& includePaths) noexcept;

        size_t getDependencies(std::istream& in, Dependencies& deps) const noexcept;
        size_t getDefines(std::istream& in, Defines& defines) const noexcept;
        static std::string getDefinesArgument(const Defines& defines) noexcept;

        static ShaderType getType(const std::string& name) noexcept;
        static ShaderType getType(const std::filesystem::path& path) noexcept;
        static std::string getTypeName(ShaderType type);

        static std::filesystem::path getDefaultOutputFile(const CompilerConfig& config, const CompilerOperation& op) noexcept;
        std::vector<CompilerOperation> prepareCompilerOperations(const CompilerConfig& config, const DataView& shader, const std::filesystem::path& baseOutputPath = "") const noexcept;
        static std::vector<CompilerOperation> getCompilerOperations(const CompilerConfig& config, const Defines& defines, const std::filesystem::path& baseOutputPath = "") noexcept;
    private:
        IncludePaths _includePaths;

        static const std::regex _includeRegex;
        static const std::regex _ifdefRegex;
        static const std::string _definePrefix;
        static const std::unordered_map<bgfx::RendererType::Enum, std::string> _rendererExtensions;
        static const std::string _binExt;
        static const std::string _enableDefineSuffix;

        size_t getDefines(std::istream& in, Defines& defines, std::unordered_set<std::filesystem::path>& checkedPaths) const noexcept;
        static std::optional<std::string> readDefine(const std::string& line) noexcept;
        size_t getDependencies(std::istream& in, Dependencies& deps, std::unordered_set<std::filesystem::path>& checkedPaths) const noexcept;
        std::optional<std::filesystem::path> readDependency(const std::string& line) const noexcept;
        static const std::vector<bgfx::RendererType::Enum>& getSupportedRenderers() noexcept;

        std::filesystem::path getCompilerProfileOutputPath(const CompilerConfig& config, const std::filesystem::path& basePath, const std::string& profileExt) const noexcept;
    };

    class ShaderCompiler final
    {
    public:
        using Config = ShaderCompilerConfig;
        using Operation = ShaderCompilerOperation;

        ShaderCompiler(const Config& config) noexcept;
        void operator()(const Operation& op) const;

    private:
        Config _config;
    };

    class ProgramCompilerImpl final
    {
    public:
        using Config = ProgramCompilerConfig;
        ProgramCompilerImpl(const Config& config) noexcept;
        void setLogOutput(OptionalRef<std::ostream> log) noexcept;
        ProgramDefinition operator()(const ProgramSource& src);
    private:
        Config _config;
        OptionalRef<std::ostream> _log;
    };

    class ProgramFileImporterImpl final
    {
    public:
        using Input = FileTypeImporterInput;
        using Dependencies = FileTypeImportDependencies;
        using Config = ProgramCompilerConfig;
        using Source = ProgramSource;
        ProgramFileImporterImpl(size_t defaultBufferSize = 4096) noexcept;

        void setShadercPath(const std::filesystem::path& path) noexcept;
        void addIncludePath(const std::filesystem::path& path) noexcept;

        void setLogOutput(OptionalRef<std::ostream> log) noexcept;
        bool startImport(const Input& input, bool dry = false);
        std::vector<std::filesystem::path> getOutputs(const Input& input);
        Dependencies getDependencies(const Input& input);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        void endImport(const Input& input);

        const std::string& getName() const noexcept;
    private:
        Config _defaultConfig;
        OptionalRef<std::ostream> _log;

        std::optional<Config> _config;
        std::optional<Source> _src;
        std::filesystem::path _outputPath;

        void writeDefinition(const ProgramDefinition& def, std::ostream& out);
        ProgramSource readSource(const Input& input);
    };
}