#pragma once

#include <darmok/program_core.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/varying.hpp>

#include <regex>
#include <unordered_set>
#include <filesystem>

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

    enum class ShaderCompilerPlatformType
    {
        Android,
        Emscripten,
        Ios,
        Linux,
        Osx,
        Orbis,
        Windows,
    };

    struct ShaderCompilerConfig final
    {
        ProgramCompilerConfig programConfig;
        std::filesystem::path path;
        std::filesystem::path varyingPath;
        ShaderType type;

        using PlatformType = ShaderCompilerPlatformType;
        std::optional<PlatformType> platform;
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
        using Dependencies = std::unordered_set<std::filesystem::path>;
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
    
        static protobuf::Shader& getShader(protobuf::Program& def, ShaderType shaderType, const CompilerOperation& op);

    private:
        IncludePaths _includePaths;

        static const std::regex _includeRegex;
        static const std::regex _ifdefRegex;
        static const std::string _definePrefix;
        static const std::unordered_map<bgfx::RendererType::Enum, std::string> _rendererExtensions;
        static const std::string _enableDefineSuffix;

        size_t getDefines(std::istream& in, Defines& defines, std::unordered_set<std::filesystem::path>& checkedPaths) const noexcept;
        static std::optional<std::string> readDefine(const std::string& line) noexcept;
        size_t getDependencies(std::istream& in, Dependencies& deps, std::unordered_set<std::filesystem::path>& checkedPaths) const noexcept;
        std::optional<std::filesystem::path> readDependency(const std::string& line) const noexcept;
        static const std::vector<bgfx::RendererType::Enum>& getSupportedRenderers() noexcept;
    };

    class ShaderCompiler final
    {
    public:
        using Config = ShaderCompilerConfig;
        using Operation = ShaderCompilerOperation;
        using PlatformType = ShaderCompilerPlatformType;

        ShaderCompiler(const Config& config) noexcept;
        expected<void, std::string> operator()(const Operation& op) const;

        static std::optional<PlatformType> getDefaultPlatform() noexcept;

    private:
        Config _config;
    };

    class ProgramFileImporterImpl final
    {
    public:
        using Effect = FileImportEffect;
        using Input = FileImportInput;
        using CompileConfig = ProgramCompilerConfig;
        using ImportConfig = FileImportConfig;
        ProgramFileImporterImpl(size_t defaultBufferSize = 4096) noexcept;

        void setShadercPath(const std::filesystem::path& path) noexcept;
        void addIncludePath(const std::filesystem::path& path) noexcept;

        void setLogOutput(OptionalRef<std::ostream> log) noexcept;
        expected<Effect, std::string> prepare(const Input& input) noexcept;
        expected<void, std::string> operator()(const Input& input, ImportConfig& config) noexcept;

        const std::string& getName() const noexcept;
    private:
        CompileConfig _defaultConfig;

        std::optional<CompileConfig> _config;
        std::optional<protobuf::ProgramSource> _src;

        expected<void, std::string> readSource(protobuf::ProgramSource& src, const nlohmann::ordered_json& json, const std::filesystem::path& path);
    };
}