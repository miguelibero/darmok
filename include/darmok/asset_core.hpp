#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <bx/bx.h>
#include <nlohmann/json.hpp>

namespace darmok
{
    struct AssetTypeImporterInput final
    {
        std::filesystem::path path;
        std::filesystem::path basePath;

        nlohmann::json config;
        nlohmann::json globalConfig;

        std::filesystem::path getRelativePath() const
        {
            return std::filesystem::relative(path, basePath);
        }
    };

    class DARMOK_EXPORT BX_NO_VTABLE IAssetTypeImporter
    {
    public:
        using Input = AssetTypeImporterInput;

        virtual ~IAssetTypeImporter() = default;
        virtual void setLogOutput(OptionalRef<std::ostream> log) noexcept {};

        virtual bool startImport(const Input& input, bool dry = false) { return true; };

        virtual std::vector<std::filesystem::path> getOutputs(const Input& input) = 0;
        virtual std::vector<std::filesystem::path> getDependencies(const Input& input) { return {}; };

        // outputIndex is the index of the element in the vector returned by getOutputs
        virtual std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& outputPath)
        {
            return std::ofstream(outputPath, std::ios::binary);
        }

        virtual void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) = 0;
        virtual void endImport(const Input& input) { };
        virtual const std::string& getName() const noexcept = 0;
    };

    class CommandLineAssetImporterImpl;

    struct CommandLineAssetImporterConfig
    {
        std::filesystem::path inputPath;
        std::filesystem::path outputPath;
        std::filesystem::path cachePath;
        std::filesystem::path shadercPath;
        std::vector<std::filesystem::path> shaderIncludePaths;
    };

    class DARMOK_EXPORT BaseCommandLineAssetImporter
    {
    public:
        using Config = CommandLineAssetImporterConfig;
        BaseCommandLineAssetImporter() noexcept;
        virtual ~BaseCommandLineAssetImporter() noexcept;
        int operator()(int argc, const char* argv[]) noexcept;
    protected:
        virtual std::vector<std::filesystem::path> getOutputs(const Config& config) const = 0;
        virtual void import(const Config & config, std::ostream& log) const = 0;
    private:
        friend class CommandLineAssetImporterImpl;
        std::unique_ptr<CommandLineAssetImporterImpl> _impl;
    };

    class AssetImporterImpl;

    class DARMOK_EXPORT AssetImporter final
    {
    public:
    	public:
		AssetImporter(const std::filesystem::path& inputPath);
		~AssetImporter() noexcept;
        AssetImporter& setCachePath(const std::filesystem::path& cachePath) noexcept;
        AssetImporter& setOutputPath(const std::filesystem::path& outputPath) noexcept;
        AssetImporter& addTypeImporter(std::unique_ptr<IAssetTypeImporter>&& processor) noexcept;

        template<typename T, typename... A>
        T& addTypeImporter(A&&... args) noexcept
        {
            auto ptr = new T(std::forward<A>(args)...);
            addTypeImporter(std::unique_ptr<IAssetTypeImporter>(ptr));
            return *ptr;
        }

        std::vector<std::filesystem::path> getOutputs() const noexcept;
		void operator()(std::ostream& log) const;
	private:
		std::unique_ptr<AssetImporterImpl> _impl;
    };

    class ShaderImporter;

    class DARMOK_EXPORT DarmokCoreAssetImporter final
    {
    public:
        DarmokCoreAssetImporter(const CommandLineAssetImporterConfig& config);
        DarmokCoreAssetImporter(const std::filesystem::path& inputPath);
        DarmokCoreAssetImporter& setCachePath(const std::filesystem::path& cachePath) noexcept;
        DarmokCoreAssetImporter& setOutputPath(const std::filesystem::path& outputPath) noexcept;
        DarmokCoreAssetImporter& setShadercPath(const std::filesystem::path& path) noexcept;
        DarmokCoreAssetImporter& addShaderIncludePath(const std::filesystem::path& path) noexcept;
        std::vector<std::filesystem::path> getOutputs() const;
        void operator()(std::ostream& log) const;
    private:
        AssetImporter _importer;
        ShaderImporter& _shaderImporter;
    };

    class DARMOK_EXPORT CopyAssetImporter final : public IAssetTypeImporter
    {
    public:
        CopyAssetImporter(size_t bufferSize = 4096) noexcept;

        std::vector<std::filesystem::path> getOutputs(const Input& input) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        const std::string& getName() const noexcept override;
    private:
        size_t _bufferSize;
    };

    class ShaderImporterImpl;

    class DARMOK_EXPORT ShaderImporter final : public IAssetTypeImporter
    {
    public:
        ShaderImporter();
        ~ShaderImporter() noexcept;
        ShaderImporter& setShadercPath(const std::filesystem::path& path) noexcept;
        ShaderImporter& addIncludePath(const std::filesystem::path& path) noexcept;
        void setLogOutput(OptionalRef<std::ostream> log) noexcept override;
        std::vector<std::filesystem::path> getOutputs(const Input& input) override;
        std::vector<std::filesystem::path> getDependencies(const Input& input) override;

        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        const std::string& getName() const noexcept override;
    private:
        std::unique_ptr<ShaderImporterImpl> _impl;
    };
}