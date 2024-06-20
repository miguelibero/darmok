#pragma once

#include <darmok/export.h>
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
    };

    class DARMOK_EXPORT BX_NO_VTABLE IAssetTypeImporter
    {
    public:
        using Input = AssetTypeImporterInput;
        virtual ~IAssetTypeImporter() = default;

        // return false if the processor cannot handle the input
        virtual bool getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs) = 0;

        // outputIndex is the index of the element in the vector returned by getOutputs
        virtual std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
        {
            return std::ofstream(input.path);
        }

        virtual void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) = 0;

        virtual std::string getName() const noexcept = 0;
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

    class DARMOK_EXPORT DarmokCoreAssetImporter final
    {
    public:
        DarmokCoreAssetImporter(const std::string& inputPath);
        DarmokCoreAssetImporter& setOutputPath(const std::string& outputPath) noexcept;
        std::vector<std::filesystem::path> getOutputs() const noexcept;
        void operator()(std::ostream& log) const;
    private:
        AssetImporter _importer;
    };

    class DARMOK_EXPORT CopyAssetImporter final : public IAssetTypeImporter
    {
    public:
        CopyAssetImporter(size_t bufferSize = 4096) noexcept;
        bool getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        std::string getName() const noexcept override;
    private:
        size_t _bufferSize;
    };

    class ShaderAssetImporterImpl;

    class DARMOK_EXPORT ShaderAssetImporter final : public IAssetTypeImporter
    {
    public:
        ShaderAssetImporter();
        ~ShaderAssetImporter() noexcept;
        bool getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        std::string getName() const noexcept override;
    private:
        std::unique_ptr<ShaderAssetImporterImpl> _impl;
    };
}