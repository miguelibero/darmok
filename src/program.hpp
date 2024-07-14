#pragma once

#include <darmok/asset_core.hpp>
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
        ShaderCompiler() noexcept;
        ShaderCompiler& setShadercPath(const std::filesystem::path& path) noexcept;
        ShaderCompiler& setIncludePaths(const std::vector<std::filesystem::path>& paths) noexcept;
        ShaderCompiler& addIncludePath(const std::filesystem::path& path) noexcept;
        ShaderCompiler& setVaryingDef(const std::filesystem::path& path) noexcept;
        ShaderCompiler& setShaderType(ShaderType type) noexcept;
        ShaderCompiler& setShaderType(const std::string& type);
        ShaderCompiler& setLogOutput(OptionalRef<std::ostream> log) noexcept;
        ShaderCompiler& setDefines(const Defines& defines) noexcept;

        int operator()(const std::filesystem::path& input, const Output& output) const;
        std::vector<std::filesystem::path> getDependencies(std::istream& in) const noexcept;
        std::vector<Output> getOutputs(const std::filesystem::path& basePath) const noexcept;
    private:
        std::filesystem::path _shadercPath;
        std::filesystem::path _varyingDef;
        std::vector<std::filesystem::path> _includes;
        Defines _defines;
        static const std::regex _includeRegex;
        OptionalRef<std::ostream> _log;
        ShaderType _shaderType;
        
        static const std::vector<std::string> _profiles;
        static const std::unordered_map<std::string, std::string> _profileExtensions;
        static const std::string _binExt;
        static const std::string _enableDefineSuffix;


        std::filesystem::path getOutputPath(const std::filesystem::path& path, const std::string& profileExt, const Defines& defines) const noexcept;
        static std::string fixPathArgument(const std::filesystem::path& path) noexcept;
        static ShaderType getShaderType(const std::string& name) noexcept;
        ShaderType getShaderType(const std::filesystem::path& path) const noexcept;
        static std::string getShaderTypeName(ShaderType type);
    };

    class ShaderImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        using Defines = ShaderDefines;
        ShaderImporterImpl(size_t bufferSize = 4096) noexcept;
        void setShadercPath(const std::filesystem::path& path) noexcept;
        void addIncludePath(const std::filesystem::path& path) noexcept;
        void setLogOutput(OptionalRef<std::ostream> log) noexcept;
        bool startImport(const Input& input, bool dry = false);
        std::vector<std::filesystem::path> getOutputs(const Input& input);
        std::vector<std::filesystem::path> getDependencies(const Input& input);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        const std::string& getName() const noexcept;
        void endImport(const Input& input);

    private:
        ShaderCompiler _compiler;
        size_t _bufferSize;
        std::vector<std::filesystem::path> _includes;
        std::filesystem::path _basePath;
        std::vector<ShaderCompilerOutput> _outputs;
        
        static const std::string _configTypeKey;
        static const std::string _configVaryingDefKey;
        static const std::string _configIncludeDirsKey;
        static const std::string _manifestFileSuffix;
    };

    struct ShaderImportDefinition final
    {
        std::filesystem::path path;
        ShaderDefines defines;

        void read(const nlohmann::json& json, std::filesystem::path basePath);
    };

    struct ProgramImportDefinition final
    {
        ShaderImportDefinition vertexShader;
        ShaderImportDefinition fragmentShader;
        std::filesystem::path varyingDefPath;
        std::filesystem::path vertexLayoutPath;
        bgfx::VertexLayout vertexLayout;

        void loadDefault(const std::filesystem::path& path);
        void read(const nlohmann::ordered_json& json, std::filesystem::path basePath);
    };

    class ProgramImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;

        bool startImport(const Input& input, bool dry = false);
        std::vector<std::filesystem::path> getOutputs(const Input& input);
        std::vector<std::filesystem::path> getDependencies(const Input& input);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        void endImport(const Input& input);

        const std::string& getName() const noexcept;
    private:
        std::optional<ProgramImportDefinition> _def;
    };
}