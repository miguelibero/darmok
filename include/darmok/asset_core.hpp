#pragma once

#include <darmok/export.h>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <bx/bx.h>

namespace darmok
{
    class DARMOK_EXPORT BX_NO_VTABLE IAssetTypeProcessor
    {
    public:
        virtual ~IAssetTypeProcessor() = default;
        // return false if the processor cannot handle the input
        virtual bool getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) const = 0;

        // outputIndex is the index of the element in the vector returned by getOutputs
        virtual std::ofstream createOutputStream(size_t outputIndex, const std::filesystem::path& path) const = 0;
        virtual void writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) const = 0;

        virtual std::string getName() const noexcept = 0;
    };


    class AssetProcessorImpl;

    class DARMOK_EXPORT AssetProcessor final
    {
    public:
    	public:
		AssetProcessor(const std::filesystem::path& inputPath);
		~AssetProcessor() noexcept;
        AssetProcessor& setProduceHeaders(bool enabled) noexcept;
        AssetProcessor& setHeaderVarPrefix(const std::string& prefix) noexcept;
        AssetProcessor& setOutputPath(const std::filesystem::path& outputPath) noexcept;
        AssetProcessor& addTypeProcessor(std::unique_ptr<IAssetTypeProcessor>&& processor) noexcept;

        template<typename T, typename... A>
        T& addTypeProcessor(A&&... args) noexcept
        {
            auto ptr = new T(std::forward<A>(args)...);
            addTypeProcessor(std::unique_ptr<IAssetTypeProcessor>(ptr));
            return *ptr;
        }

        std::vector<std::filesystem::path> getOutputs() const noexcept;
		void operator()(std::ostream& log) const;
	private:
		std::unique_ptr<AssetProcessorImpl> _impl;
    };

    class DARMOK_EXPORT DarmokCoreAssetProcessor final
    {
    public:
        DarmokCoreAssetProcessor(const std::string& inputPath);
        DarmokCoreAssetProcessor& setProduceHeaders(bool enabled) noexcept;
        DarmokCoreAssetProcessor& setHeaderVarPrefix(const std::string& prefix) noexcept;
        DarmokCoreAssetProcessor& setOutputPath(const std::string& outputPath) noexcept;
        std::vector<std::filesystem::path> getOutputs() const noexcept;
        void operator()(std::ostream& log) const;
    private:
        AssetProcessor _processor;
    };

    class ShaderAssetProcessorImpl;

    class DARMOK_EXPORT ShaderAssetProcessor final : public IAssetTypeProcessor
    {
    public:
    public:
        ShaderAssetProcessor();
        ~ShaderAssetProcessor() noexcept;
        bool getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) const override;
        std::ofstream createOutputStream(size_t outputIndex, const std::filesystem::path& path) const override;
        void writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) const override;
        std::string getName() const noexcept override;
    private:
        std::unique_ptr<ShaderAssetProcessorImpl> _impl;
    };
}