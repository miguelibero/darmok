#pragma once

#include <darmok/asset_core.hpp>
#include <regex>

namespace darmok
{
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

        using Defines = std::vector<std::string>;
        struct OutputConfig final
        {
            std::filesystem::path path;
            std::string profile;
            Defines defines;
        };
        std::vector<OutputConfig> _outputConfigs;

        static const std::string _binExt;
        static const std::vector<std::string> _profiles;
        static const std::unordered_map<std::string, std::string> _profileExtensions;
        static const std::string _configTypeKey;
        static const std::string _configVaryingDefKey;
        static const std::string _configIncludeDirsKey;
        static const std::regex _includeRegex;
        static const std::string _enableDefineSuffix;
        static const std::string _manifestFileSuffix;

        std::vector<std::filesystem::path> getIncludes(const Input& input) const noexcept;
        static std::filesystem::path getOutputPath(const std::filesystem::path& path, const std::string& profileExt, const Defines& defines) noexcept;
        static std::string fixPathArgument(const std::filesystem::path& path) noexcept;
        std::vector<Defines> getDefineCombinations(const Input& input) const noexcept;
    };
}