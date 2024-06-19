#include "asset_core.hpp"
#include <darmok/asset_core.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace darmok
{
    namespace fs = std::filesystem;

    AssetProcessorImpl::AssetProcessorImpl(const std::filesystem::path& inputPath)
        : _inputPath(inputPath)
        , _produceHeaders(false)
    {
        if (!fs::exists(inputPath))
        {
            throw std::runtime_error("input path does not exist");
        }
    }

    void AssetProcessorImpl::setOutputPath(const std::filesystem::path& outputPath) noexcept
    {
        _outputPath = outputPath;
    }

    std::vector<fs::path> AssetProcessorImpl::getOutputs() const noexcept
    {
        std::vector<fs::path> outputs;
        auto inputs = getInputs();
        for (auto& path : inputs)
        {
            for (auto& processor : _processors)
            {
                getOutputs(*processor, path, outputs);
            }
        }

        return outputs;
    }

    void AssetProcessorImpl::setHeaderVarPrefix(const std::string& prefix) noexcept
    {
        _headerVarPrefix = prefix;
    }

    void AssetProcessorImpl::setProduceHeaders(bool headers) noexcept
    {
        _produceHeaders = headers;
    }

    void AssetProcessorImpl::addTypeProcessor(std::unique_ptr<IAssetTypeProcessor>&& processor) noexcept
    {
        _processors.push_back(std::move(processor));
    }

    void AssetProcessorImpl::operator()(std::ostream& log) const
    {
        auto inputs = getInputs();
        for (auto& path : inputs)
        {
            for (auto& processor : _processors)
            {
                processFile(*processor, path, log);
            }
        }
    }

    std::vector<fs::path> AssetProcessorImpl::getInputs() const noexcept
    {
        std::vector<fs::path> inputs;
        fs::path path(_inputPath);
        if (fs::is_directory(path))
        {
            for (auto& entry : fs::recursive_directory_iterator(path))
            {
                for (auto& processor : _processors)
                {
                    inputs.push_back(entry.path());
                }
            }
            return inputs;
        }
        inputs.push_back(path);
        return inputs;
    }

    fs::path AssetProcessorImpl::getHeaderPath(const fs::path& path, const std::string& baseName) const noexcept
    {
        return path.parent_path() / fs::path(baseName + ".h");
    }

    fs::path AssetProcessorImpl::getHeaderPath(const fs::path& path) const noexcept
    {
        return getHeaderPath(path, path.stem().string());
    }

    size_t AssetProcessorImpl::getOutputs(IAssetTypeProcessor& processor, const fs::path& path, std::vector<fs::path>& outputs) const
    {
        std::vector<fs::path> processorOutputs;
        if (!processor.getOutputs(path, processorOutputs))
        {
            return 0;
        }

        auto relInPath = fs::relative(path, _inputPath);
        for (fs::path output : processorOutputs)
        {
            if (_produceHeaders)
            {
                output = getHeaderPath(output);
            }
            auto relOutPath = relInPath.parent_path() / output;
            outputs.push_back(_outputPath / relOutPath);
        }

        return processorOutputs.size();
    }

    size_t AssetProcessorImpl::processFile(IAssetTypeProcessor& processor, const fs::path& path, std::ostream& log) const
    {
        std::vector<fs::path> outputs;
        if (!processor.getOutputs(path, outputs))
        {
            return 0;
        }
        size_t i = 0;
        auto relInPath = fs::relative(path, _inputPath);
        for (fs::path output : outputs)
        {
            std::string headerVarName;
            if (_produceHeaders)
            {
                auto stem = output.stem().string();
                headerVarName = _headerVarPrefix + stem;
                output = getHeaderPath(output, stem);
            }

            auto relOutPath = relInPath.parent_path() / output;
            auto outPath = _outputPath / relOutPath;

            log << processor.getName() << " " << relInPath << " -> " << relOutPath << "..." << std::endl;

            if (_produceHeaders)
            {
                Data data;
                DataOutputStream ds(data);
                processor.writeOutput(path, i, ds);
                std::ofstream out(outPath);
                out << data.view().toHeader(headerVarName);
            }
            else
            {
                auto out = processor.createOutputStream(i, outPath);
                processor.writeOutput(path, i, out);
            }
            ++i;
        }

        return outputs.size();
    }

    AssetProcessor::AssetProcessor(const fs::path& inputPath)
        : _impl(std::make_unique<AssetProcessorImpl>(inputPath))
    {
    }

    AssetProcessor::~AssetProcessor() noexcept
    {
        // empty on purpose
    }

    AssetProcessor& AssetProcessor::setProduceHeaders(bool enabled) noexcept
    {
        _impl->setProduceHeaders(enabled);
        return *this;
    }

    AssetProcessor& AssetProcessor::setHeaderVarPrefix(const std::string& prefix) noexcept
    {
        _impl->setHeaderVarPrefix(prefix);
        return *this;
    }

    AssetProcessor& AssetProcessor::addTypeProcessor(std::unique_ptr<IAssetTypeProcessor>&& processor) noexcept
    {
        _impl->addTypeProcessor(std::move(processor));
        return *this;
    }

    AssetProcessor& AssetProcessor::setOutputPath(const fs::path& outputPath) noexcept
    {
        _impl->setOutputPath(outputPath);
        return *this;
    }

    std::vector<fs::path> AssetProcessor::getOutputs() const noexcept
    {
        return _impl->getOutputs();
    }

    void AssetProcessor::operator()(std::ostream& out) const
    {
        (*_impl)(out);
    }

    ShaderAssetProcessorImpl::ShaderAssetProcessorImpl()
    {
    }

    bool ShaderAssetProcessorImpl::getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) const
    {
        auto ext = input.extension();
        if (ext != ".fragment.sc" && ext != ".vertex.sc")
        {
            return false;
        }
        return true;
    }

    std::ofstream ShaderAssetProcessorImpl::createOutputStream(size_t outputIndex, const std::filesystem::path& path) const
    {
        return std::ofstream(path, std::ios::binary);
    }

    void ShaderAssetProcessorImpl::writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) const
    {

    }

    std::string ShaderAssetProcessorImpl::getName() const noexcept
    {
        static const std::string name("Shader");
        return name;
    }

    ShaderAssetProcessor::ShaderAssetProcessor()
        : _impl(std::make_unique<ShaderAssetProcessorImpl>())
    {
    }

    ShaderAssetProcessor::~ShaderAssetProcessor() noexcept
    {
        // empty on purpose
    }

    bool ShaderAssetProcessor::getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) const
    {
        return _impl->getOutputs(input, outputs);
    }

    std::ofstream ShaderAssetProcessor::createOutputStream(size_t outputIndex, const std::filesystem::path& path) const
    {
        return _impl->createOutputStream(outputIndex, path);
    }

    void ShaderAssetProcessor::writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) const
    {
        return _impl->writeOutput(input, outputIndex, out);
    }

    std::string ShaderAssetProcessor::getName() const noexcept
    {
        return _impl->getName();
    }

    DarmokCoreAssetProcessor::DarmokCoreAssetProcessor(const std::string& inputPath)
        : _processor(inputPath)
    {
        _processor.addTypeProcessor<VertexLayoutProcessor>();
        _processor.addTypeProcessor<ShaderAssetProcessor>();
    }

    DarmokCoreAssetProcessor& DarmokCoreAssetProcessor::setProduceHeaders(bool enabled) noexcept
    {
        _processor.setProduceHeaders(enabled);
        return *this;
    }

    DarmokCoreAssetProcessor& DarmokCoreAssetProcessor::setHeaderVarPrefix(const std::string& prefix) noexcept
    {
        _processor.setHeaderVarPrefix(prefix);
        return *this;
    }

    DarmokCoreAssetProcessor& DarmokCoreAssetProcessor::setOutputPath(const std::string& outputPath) noexcept
    {
        _processor.setOutputPath(outputPath);
        return *this;
    }

    std::vector<fs::path> DarmokCoreAssetProcessor::getOutputs() const noexcept
    {
        return _processor.getOutputs();
    }

    void DarmokCoreAssetProcessor::operator()(std::ostream& out) const
    {
        _processor(out);
    }
}