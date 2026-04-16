#pragma once

#include <darmok/slang.hpp>
#include <slang.h>
#include <slang-com-ptr.h> 

namespace darmok
{
    class SlangProgramCompilerImpl final
    {
    public:
        using Config = SlangProgramCompilerConfig;
        using Source = protobuf::SlangProgramSource;

        SlangProgramCompilerImpl(Slang::ComPtr<slang::IGlobalSession> globalSession, const Config& config) noexcept;
        static expected<SlangProgramCompilerImpl, std::string> create(const Config& config) noexcept;
        expected<protobuf::Program, std::string> operator()(const Source& src) noexcept;
    private:
        Slang::ComPtr<slang::IGlobalSession> _globalSession;
        Config _config;
        slang::SessionDesc _sessionDesc;
        std::vector<std::string> _searchPathStrings;
        std::vector<const char*> _searchPathChars;
        std::vector<slang::TargetDesc> _targetDescs;

        using TargetProfileMap = std::unordered_map<SlangCompileTarget, std::string>;
		static const TargetProfileMap _targetProfileMap;
		static const std::vector<SlangCompileTarget> _supportedTargets;
        using TargetRendererMap = std::unordered_map<SlangCompileTarget, bgfx::RendererType::Enum>;
        static const TargetRendererMap _targetRenderers;

		static expected<slang::TypeLayoutReflection*, std::string> getStructParamLayout(slang::EntryPointReflection& entryPoint) noexcept;
		static expected<protobuf::VertexLayout, std::string> createVertexLayout(slang::EntryPointReflection& entryPoint) noexcept;
        static expected<protobuf::FragmentLayout, std::string> createFragmentLayout(slang::EntryPointReflection& entryPoint) noexcept;
        static void updateShader(protobuf::Shader& shader, const std::unordered_set<std::string>& defines, slang::IBlob& data) noexcept;
        static std::optional<protobuf::Bgfx::Attrib> getBgfxAttrib(std::string_view semanticName, size_t semanticIndex) noexcept;
        static std::optional<protobuf::Bgfx::AttribType> getBgfxAttribType(slang::TypeReflection::ScalarType scalarType) noexcept;
        static std::string getDiagnosticsString(slang::IBlob* diagnostics) noexcept;
        expected<slang::IComponentType*, std::string> compileProgram(const Source& src, const std::unordered_set<std::string>& defines, OptionalRef<std::ostream> log = nullptr) noexcept;
    };

    class SlangProgramFileImporterImpl final
    {
    public:
        using Effect = FileImportEffect;
        using Input = FileImportInput;
        using CompileConfig = SlangProgramCompilerConfig;
        using ImportConfig = FileImportConfig;
        using Source = protobuf::SlangProgramSource;

        void addIncludePath(const std::filesystem::path& path) noexcept;

        expected<void, std::string> init(OptionalRef<std::ostream> log = nullptr) noexcept;
        expected<Effect, std::string> prepare(const Input& input) noexcept;
        expected<void, std::string> operator()(const Input& input, ImportConfig& config) noexcept;

        const std::string& getName() const noexcept;
    private:
        CompileConfig _defaultConfig;
        OptionalRef<std::ostream> _log;
        std::optional<CompileConfig> _config;
        std::optional<Source> _src;
    };
}