#include "asset_core.hpp"
#include <darmok/asset_core.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace darmok
{
    namespace fs = std::filesystem;

    AssetImporterImpl::AssetImporterImpl(const std::filesystem::path& inputPath)
        : _inputPath(inputPath)
        , _produceHeaders(false)
    {
        if (!fs::exists(inputPath))
        {
            throw std::runtime_error("input path does not exist");
        }
    }

    void AssetImporterImpl::setOutputPath(const std::filesystem::path& outputPath) noexcept
    {
        _outputPath = outputPath;
    }

    std::vector<fs::path> AssetImporterImpl::getOutputs() const
    {
        std::vector<fs::path> outputs;
        auto inputs = getInputs();
        for (auto& path : inputs)
        {
            for (auto& importer : _importers)
            {
                getOutputs(*importer, path, outputs);
            }
        }

        return outputs;
    }

    void AssetImporterImpl::setHeaderVarPrefix(const std::string& prefix) noexcept
    {
        _headerVarPrefix = prefix;
    }

    void AssetImporterImpl::setProduceHeaders(bool headers) noexcept
    {
        _produceHeaders = headers;
    }

    void AssetImporterImpl::addTypeImporter(std::unique_ptr<IAssetTypeImporter>&& importer) noexcept
    {
        _importers.push_back(std::move(importer));
    }

    void AssetImporterImpl::operator()(std::ostream& log) const
    {
        auto inputs = getInputs();
        for (auto& path : inputs)
        {
            for (auto& importer : _importers)
            {
                processFile(*importer, path, log);
            }
        }
    }

    std::vector<fs::path> AssetImporterImpl::getInputs() const noexcept
    {
        std::vector<fs::path> inputs;
        fs::path path(_inputPath);
        if (fs::is_directory(path))
        {
            for (auto& entry : fs::recursive_directory_iterator(path))
            {
                inputs.push_back(entry.path());
            }
        }
        else
        {
            inputs.push_back(path);
        }
        return inputs;
    }

    fs::path AssetImporterImpl::getHeaderPath(const fs::path& path, const std::string& baseName) const noexcept
    {
        return path.parent_path() / fs::path(baseName + ".h");
    }

    fs::path AssetImporterImpl::getHeaderPath(const fs::path& path) const noexcept
    {
        return getHeaderPath(path, path.stem().string());
    }

    size_t AssetImporterImpl::getOutputs(IAssetTypeImporter& importer, const fs::path& path, std::vector<fs::path>& outputs) const
    {
        std::vector<fs::path> importerOutputs;
        if (!importer.getOutputs(path, importerOutputs))
        {
            return 0;
        }

        auto relInPath = fs::relative(path, _inputPath);
        for (fs::path output : importerOutputs)
        {
            if (_produceHeaders)
            {
                output = getHeaderPath(output);
            }
            output = _outputPath / relInPath.parent_path() / output;
            if (std::find(outputs.begin(), outputs.end(), output) != outputs.end())
            {
                throw std::runtime_error(std::string("multiple importers produce the same output: ") + output.string());
            }
            outputs.push_back(output);
        }

        return importerOutputs.size();
    }

    size_t AssetImporterImpl::processFile(IAssetTypeImporter& importer, const fs::path& path, std::ostream& log) const
    {
        std::vector<fs::path> outputs;
        if (!importer.getOutputs(path, outputs))
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

            log << importer.getName() << " " << relInPath << " -> " << relOutPath << "..." << std::endl;

            if (_produceHeaders)
            {
                Data data;
                DataOutputStream ds(data);
                importer.writeOutput(path, i, ds);
                std::ofstream out(outPath);
                out << data.view().toHeader(headerVarName);
            }
            else
            {
                auto out = importer.createOutputStream(path, i, outPath);
                importer.writeOutput(path, i, out);
            }
            ++i;
        }

        return outputs.size();
    }

    AssetImporter::AssetImporter(const fs::path& inputPath)
        : _impl(std::make_unique<AssetImporterImpl>(inputPath))
    {
    }

    AssetImporter::~AssetImporter() noexcept
    {
        // empty on purpose
    }

    AssetImporter& AssetImporter::setProduceHeaders(bool enabled) noexcept
    {
        _impl->setProduceHeaders(enabled);
        return *this;
    }

    AssetImporter& AssetImporter::setHeaderVarPrefix(const std::string& prefix) noexcept
    {
        _impl->setHeaderVarPrefix(prefix);
        return *this;
    }

    AssetImporter& AssetImporter::addTypeImporter(std::unique_ptr<IAssetTypeImporter>&& importer) noexcept
    {
        _impl->addTypeImporter(std::move(importer));
        return *this;
    }

    AssetImporter& AssetImporter::setOutputPath(const fs::path& outputPath) noexcept
    {
        _impl->setOutputPath(outputPath);
        return *this;
    }

    std::vector<fs::path> AssetImporter::getOutputs() const noexcept
    {
        return _impl->getOutputs();
    }

    void AssetImporter::operator()(std::ostream& out) const
    {
        (*_impl)(out);
    }

    ShaderAssetImporterImpl::ShaderAssetImporterImpl()
    {
    }

    bool ShaderAssetImporterImpl::getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) const
    {
        auto ext = StringUtils::getFileExt(input.filename().string());
        if (ext != ".fragment.sc" && ext != ".vertex.sc")
        {
            return false;
        }
        return true;
    }

    std::ofstream ShaderAssetImporterImpl::createOutputStream(size_t outputIndex, const std::filesystem::path& path) const
    {
        return std::ofstream(path, std::ios::binary);
    }

    void ShaderAssetImporterImpl::writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) const
    {
    }

    std::string ShaderAssetImporterImpl::getName() const noexcept
    {
        static const std::string name("Shader");
        return name;
    }

    ShaderAssetImporter::ShaderAssetImporter()
        : _impl(std::make_unique<ShaderAssetImporterImpl>())
    {
    }

    ShaderAssetImporter::~ShaderAssetImporter() noexcept
    {
        // empty on purpose
    }

    bool ShaderAssetImporter::getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs)
    {
        return _impl->getOutputs(input, outputs);
    }

    std::ofstream ShaderAssetImporter::createOutputStream(const std::filesystem::path& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return _impl->createOutputStream(outputIndex, path);
    }

    void ShaderAssetImporter::writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out)
    {
        return _impl->writeOutput(input, outputIndex, out);
    }

    std::string ShaderAssetImporter::getName() const noexcept
    {
        return _impl->getName();
    }

    DarmokCoreAssetImporter::DarmokCoreAssetImporter(const std::string& inputPath)
        : _importer(inputPath)
    {
        _importer.addTypeImporter<VertexLayoutImporter>();
        _importer.addTypeImporter<ShaderAssetImporter>();
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::setProduceHeaders(bool enabled) noexcept
    {
        _importer.setProduceHeaders(enabled);
        return *this;
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::setHeaderVarPrefix(const std::string& prefix) noexcept
    {
        _importer.setHeaderVarPrefix(prefix);
        return *this;
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::setOutputPath(const std::string& outputPath) noexcept
    {
        _importer.setOutputPath(outputPath);
        return *this;
    }

    std::vector<fs::path> DarmokCoreAssetImporter::getOutputs() const noexcept
    {
        return _importer.getOutputs();
    }

    void DarmokCoreAssetImporter::operator()(std::ostream& out) const
    {
        _importer(out);
    }
}