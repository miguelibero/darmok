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

        expected<Slang::ComPtr<slang::ISession>, std::string> createSession(const std::unordered_set<std::string> &defines) noexcept;
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
        void addBgfxIncludePath(const std::filesystem::path& path) noexcept;
        void setShadercPath(const std::filesystem::path& path) noexcept;

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
