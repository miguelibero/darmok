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

    AssetImporterImpl::AssetImporterImpl(const fs::path& inputPath)
        : _inputPath(inputPath)
        , _produceHeaders(false)
    {
        if (fs::is_directory(inputPath))
        {
            loadGlobalConfig(inputPath / _globalConfigFileSuffix);
            for (auto& entry : fs::recursive_directory_iterator(inputPath))
            {
                loadInput(entry.path());
            }
        }
        if (fs::exists(inputPath))
        {
            loadGlobalConfig(inputPath.parent_path() / _globalConfigFileSuffix);
            loadInput(inputPath);
        }
        else
        {
            throw std::runtime_error("input path does not exist");
        }
    }

    const std::string AssetImporterImpl::_globalConfigFileSuffix = "darmok-import.json";
    const std::string AssetImporterImpl::_inputConfigFileSuffix = ".darmok-import.json";
    const std::string AssetImporterImpl::_globalConfigFilesKey = "files";
    const std::string AssetImporterImpl::_globalConfigHeaderVarPrefixKey = "headerVarPrefix";
    const std::string AssetImporterImpl::_globalConfigHeadersKey = "headers";

    bool AssetImporterImpl::loadInput(const fs::path& path)
    {
        fs::path configPath(path.string() + _inputConfigFileSuffix);
        if (!fs::exists(configPath))
        {
            loadInputConfig(path, nlohmann::json());
            return false;
        }
        std::ifstream is(configPath);
        loadInputConfig(path, nlohmann::json::parse(is));
        return true;
    }

    bool AssetImporterImpl::loadGlobalConfig(const fs::path& path)
    {
        if (!fs::exists(path))
        {
            return false;
        }
        std::ifstream is(path);
        auto config = nlohmann::json::parse(is);
        if (config.contains(_globalConfigHeaderVarPrefixKey))
        {
            _headerVarPrefix = config[_globalConfigHeaderVarPrefixKey];
        }
        if (config.contains(_globalConfigHeadersKey))
        {
            _produceHeaders = config[_globalConfigHeadersKey];
        }
        if (config.contains(_globalConfigFilesKey))
        {
            for (auto& item : config[_globalConfigFilesKey].items())
            {
                loadInputConfig(item.key(), item.value());
            }
        }
        return true;
    }

    const nlohmann::json& AssetImporterImpl::loadInputConfig(const std::filesystem::path& path, const nlohmann::json& json) noexcept
    {
        if (json.is_null())
        {
            auto itr = _inputs.find(path);
            if (itr == _inputs.end())
            {
                itr = _inputs.emplace(path, nullptr).first;
            }
            return itr->second;
        }
        if (json.is_primitive())
        {
            nlohmann::json objJson;
            objJson[json.get<std::string>()] = nlohmann::json::object();
            return loadInputConfig(path, objJson);
        }
        if (json.is_array())
        {
            nlohmann::json objJson;
            for (auto& elm : json)
            {
                if (elm.is_string())
                {
                    objJson[elm] = nlohmann::json::object();
                }
            }
            return loadInputConfig(path, objJson);
        }
        auto itr = _inputs.find(path);
        if (itr == _inputs.end())
        {
            return _inputs.emplace(path, json).first->second;
        }
        itr->second += json;
        return itr->second;
    }


    void AssetImporterImpl::setOutputPath(const std::filesystem::path& outputPath) noexcept
    {
        _outputPath = outputPath;
    }

    AssetImporterImpl::ImporterInputs AssetImporterImpl::getImporterInputs() const
    {
        ImporterInputs inputs;
        for (auto& [path, config] : _inputs)
        {
            for (auto& [importerName, importerConfig ] : config.items())
            {
                auto itr = _importers.find(importerName);
                if (itr == _importers.end())
                {
                    std::stringstream ss;
                    ss << "file " << path << " expects missing importer \"" << importerName << "\".";
                    throw std::runtime_error(ss.str());
                }
            }
            for (auto& [importerName, importer] : _importers)
            {
                auto itr = config.find(importerName);
                if (itr != config.end())
                {
                    inputs.emplace_back(*importer, Input{ path, *itr });
                }
                else
                {
                    inputs.emplace_back(*importer, Input{ path, nullptr });
                }
            }
        }
        return inputs;
    }

    std::vector<fs::path> AssetImporterImpl::getOutputs() const
    {
        std::vector<fs::path> outputs;
        for (auto& [importer, input] : getImporterInputs())
        {
            getOutputs(importer.value(), input, outputs);
        }
        return outputs;
    }

    void AssetImporterImpl::addTypeImporter(std::unique_ptr<IAssetTypeImporter>&& importer) noexcept
    {
        _importers[importer->getName()] = std::move(importer);
    }

    void AssetImporterImpl::operator()(std::ostream& log) const
    {
        for (auto& [importer, input] : getImporterInputs())
        {
            importFile(importer.value(), input, log);
        }
    }

    fs::path AssetImporterImpl::getHeaderPath(const fs::path& path, const std::string& baseName) const noexcept
    {
        return path.parent_path() / fs::path(baseName + ".h");
    }

    fs::path AssetImporterImpl::getHeaderPath(const fs::path& path) const noexcept
    {
        return getHeaderPath(path, path.stem().string());
    }

    size_t AssetImporterImpl::getOutputs(IAssetTypeImporter& importer, const Input& input, std::vector<fs::path>& outputs) const
    {
        std::vector<fs::path> importerOutputs;
        if (!importer.getOutputs(input, importerOutputs))
        {
            return 0;
        }

        auto relInPath = fs::relative(input.path, _inputPath);
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

    size_t AssetImporterImpl::importFile(IAssetTypeImporter& importer, const Input& input, std::ostream& log) const
    {
        std::vector<fs::path> outputs;
        if (!importer.getOutputs(input, outputs))
        {
            return 0;
        }
        size_t i = 0;
        auto relInPath = fs::relative(input.path, _inputPath);
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
                importer.writeOutput(input, i, ds);
                std::ofstream out(outPath);
                out << data.view().toHeader(headerVarName);
            }
            else
            {
                auto out = importer.createOutputStream(input, i, outPath);
                importer.writeOutput(input, i, out);
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

    CopyAssetImporter::CopyAssetImporter(size_t bufferSize) noexcept
        : _bufferSize(bufferSize)
    {
    }

    bool CopyAssetImporter::getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs)
    {
        if (input.config.is_null())
        {
            return false;
        }
        outputs.push_back(input.path.filename());
        return true;
    }

    std::ofstream CopyAssetImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return std::ofstream(input.path, std::ios::binary);
    }

    void CopyAssetImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        std::vector<char> buffer(_bufferSize);
        std::ifstream is(input.path, std::ios::binary);
        while (is.read(&buffer.front(), _bufferSize) || is.gcount() > 0)
        {
            out.write(&buffer.front(), is.gcount());
        }
    }

    std::string CopyAssetImporter::getName() const noexcept
    {
        static const std::string name("copy");
        return name;
    }

    ShaderAssetImporterImpl::ShaderAssetImporterImpl()
    {
    }

    bool ShaderAssetImporterImpl::getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs)
    {
        auto ext = StringUtils::getFileExt(input.path.filename().string());
        if (ext != ".fragment.sc" && ext != ".vertex.sc")
        {
            return false;
        }
        return true;
    }

    std::ofstream ShaderAssetImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return std::ofstream(path, std::ios::binary);
    }

    void ShaderAssetImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
    }

    std::string ShaderAssetImporterImpl::getName() const noexcept
    {
        static const std::string name("shader");
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

    bool ShaderAssetImporter::getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs)
    {
        return _impl->getOutputs(input, outputs);
    }

    std::ofstream ShaderAssetImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return _impl->createOutputStream(input, outputIndex, path);
    }

    void ShaderAssetImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
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
        _importer.addTypeImporter<CopyAssetImporter>();
        _importer.addTypeImporter<VertexLayoutImporter>();
        _importer.addTypeImporter<ShaderAssetImporter>();
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