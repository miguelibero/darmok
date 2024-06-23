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
        nlohmann::json config;
        nlohmann::json globalConfig;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IAssetTypeImporter
    {
    public:
        using Input = AssetTypeImporterInput;
        virtual ~IAssetTypeImporter() = default;

        virtual void setLogOutput(OptionalRef<std::ostream> log) noexcept { };

        // return the amount of outputs added
        virtual size_t getOutputs(const Input& input, const std::filesystem::path& basePath, std::vector<std::filesystem::path>& outputs) = 0;

        // outputIndex is the index of the element in the vector returned by getOutputs
        virtual std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& outputPath)
        {
            return std::ofstream(outputPath);
        }

        virtual void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) = 0;

        virtual std::string getName() const noexcept = 0;
    };

    class CommandLineAssetImporterImpl;

    class DARMOK_EXPORT BaseCommandLineAssetImporter
    {
    public:
        BaseCommandLineAssetImporter() noexcept;
        virtual ~BaseCommandLineAssetImporter() noexcept;
        int operator()(int argc, const char* argv[]) noexcept;
    protected:
        virtual void setInputPath(const std::filesystem::path& inputPath) = 0;
        virtual void setOutputPath(const std::filesystem::path& outputPath) = 0;
        virtual void setShadercPath(const std::filesystem::path& path) = 0;
        virtual void addShaderIncludePath(const std::filesystem::path& path) = 0;
        virtual std::vector<std::filesystem::path> getOutputs() const = 0;
        virtual void import(std::ostream& log) const = 0;
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
        DarmokCoreAssetImporter(const std::filesystem::path& inputPath);
        void setOutputPath(const std::filesystem::path& outputPath) noexcept;
        void setShadercPath(const std::filesystem::path& path) noexcept;
        void addShaderIncludePath(const std::filesystem::path& path) noexcept;
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
        size_t getOutputs(const Input& input, const std::filesystem::path& basePath, std::vector<std::filesystem::path>& outputs) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        std::string getName() const noexcept override;
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
        size_t getOutputs(const Input& input, const std::filesystem::path& basePath, std::vector<std::filesystem::path>& outputs) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        std::string getName() const noexcept override;
    private:
        std::unique_ptr<ShaderImporterImpl> _impl;
    };
}