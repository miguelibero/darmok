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

    struct ShaderCompilerOutput
    {
        std::filesystem::path path;
        std::string profile;
        ShaderDefines defines;
    };

    class ShaderCompiler final
    {
    public:
        using Defines = ShaderDefines;
        using Output = ShaderCompilerOutput;
        using Dependencies = AssetImportDependencies;
        ShaderCompiler() noexcept;
        void reset() noexcept;
        ShaderCompiler& setShadercPath(const std::filesystem::path& path) noexcept;
        ShaderCompiler& setIncludePaths(const std::vector<std::filesystem::path>& paths) noexcept;
        ShaderCompiler& addIncludePath(const std::filesystem::path& path) noexcept;
        ShaderCompiler& setVaryingDef(const std::filesystem::path& path) noexcept;
        ShaderCompiler& setShaderType(ShaderType type) noexcept;
        ShaderCompiler& setShaderType(const std::string& type);
        ShaderCompiler& setLogOutput(OptionalRef<std::ostream> log) noexcept;
        ShaderCompiler& setDefines(const Defines& defines) noexcept;
        ShaderCompiler& setDefines(const std::filesystem::path& shaderPath) noexcept;

        void operator()(const std::filesystem::path& input, const Output& output) const;
        size_t getDependencies(std::istream& in, Dependencies& deps) const noexcept;
        size_t getDefines(std::istream& in, Defines& defines) const noexcept;
        std::vector<Output> getOutputs(const std::filesystem::path& basePath = "") const noexcept;
    private:
        std::filesystem::path _shadercPath;
        std::filesystem::path _varyingDef;
        std::vector<std::filesystem::path> _includes;
        Defines _defines;
        OptionalRef<std::ostream> _log;
        ShaderType _shaderType;
        
        static const std::regex _includeRegex;
        static const std::regex _ifdefRegex;
        static const std::string _definePrefix;
        static const std::unordered_map<bgfx::RendererType::Enum, std::string> _rendererExtensions;
        static const std::string _binExt;
        static const std::string _enableDefineSuffix;

        std::filesystem::path getOutputPath(const std::filesystem::path& path, const std::string& profileExt, const Defines& defines) const noexcept;
        static ShaderType getShaderType(const std::string& name) noexcept;
        ShaderType getShaderType(const std::filesystem::path& path) const noexcept;
        static std::string getShaderTypeName(ShaderType type);
        static const std::vector<bgfx::RendererType::Enum>& getSupportedRenderers() noexcept;

        size_t getDefines(std::istream& in, Defines& defines, std::unordered_set<std::filesystem::path>& checkedPaths) const noexcept;
        std::optional<std::string> readDefine(const std::string& line) const noexcept;
        size_t getDependencies(std::istream& in, Dependencies& deps, std::unordered_set<std::filesystem::path>& checkedPaths) const noexcept;
        std::optional<std::filesystem::path> readDependency(const std::string& line) const noexcept;
    };

    struct ProgramImportConfig final
    {
        std::string name;
        std::filesystem::path vertexShader;
        std::filesystem::path fragmentShader;
        VaryingDefinition varying;

        void load(const AssetTypeImporterInput& input);
    private:
        void read(const nlohmann::ordered_json& json, std::filesystem::path basePath);
    };

    class ProgramImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        using Dependencies = AssetImportDependencies;
        using Config = ProgramImportConfig;
        ProgramImporterImpl(size_t bufferSize = 4096) noexcept;
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
        size_t _bufferSize;
        std::optional<Config> _config;
        std::filesystem::path _outputPath;
        ShaderCompiler _compiler;
        std::vector<std::filesystem::path> _includes;

        static const std::string _configIncludeDirsKey;
        static const std::string _shadercPathKey;
        void addIncludes(const nlohmann::json& json, const std::filesystem::path& basePath) noexcept;
    };
}