#pragma once

#include <darmok/export.h>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <bx/bx.h>

namespace darmok
{
    class DARMOK_EXPORT BX_NO_VTABLE IAssetTypeImporter
    {
    public:
        virtual ~IAssetTypeImporter() = default;
        // return false if the processor cannot handle the input
        virtual bool getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) = 0;

        // outputIndex is the index of the element in the vector returned by getOutputs
        virtual std::ofstream createOutputStream(const std::filesystem::path& input, size_t outputIndex, const std::filesystem::path& path) = 0;
        virtual void writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) = 0;

        virtual std::string getName() const noexcept = 0;
    };


    class AssetImporterImpl;

    class DARMOK_EXPORT AssetImporter final
    {
    public:
    	public:
		AssetImporter(const std::filesystem::path& inputPath);
		~AssetImporter() noexcept;
        AssetImporter& setProduceHeaders(bool enabled) noexcept;
        AssetImporter& setHeaderVarPrefix(const std::string& prefix) noexcept;
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
        DarmokCoreAssetImporter& setProduceHeaders(bool enabled) noexcept;
        DarmokCoreAssetImporter& setHeaderVarPrefix(const std::string& prefix) noexcept;
        DarmokCoreAssetImporter& setOutputPath(const std::string& outputPath) noexcept;
        std::vector<std::filesystem::path> getOutputs() const noexcept;
        void operator()(std::ostream& log) const;
    private:
        AssetImporter _importer;
    };

    class ShaderAssetImporterImpl;

    class DARMOK_EXPORT ShaderAssetImporter final : public IAssetTypeImporter
    {
    public:
    public:
        ShaderAssetImporter();
        ~ShaderAssetImporter() noexcept;
        bool getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) override;
        std::ofstream createOutputStream(const std::filesystem::path& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) override;
        std::string getName() const noexcept override;
    private:
        std::unique_ptr<ShaderAssetImporterImpl> _impl;
    };
}