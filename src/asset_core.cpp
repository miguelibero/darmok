#include "asset_core.hpp"
#include <darmok/asset_core.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/stream.hpp>
#include <darmok/utils.hpp>
#include <bx/platform.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>

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
    const std::string AssetImporterImpl::_globalConfigImportersKey = "importers";
    const std::string AssetImporterImpl::_globalConfigHeaderVarPrefixKey = "headerVarPrefix";
    const std::string AssetImporterImpl::_globalConfigProduceHeadersKey = "produceHeaders";
    const std::string AssetImporterImpl::_globalConfigHeaderIncludeDirKey = "headerIncludeDir";
    const std::string AssetImporterImpl::_globalConfigOutputPathKey = "outputPath";

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
            _produceHeaders = true;
        }
        if (config.contains(_globalConfigHeaderIncludeDirKey))
        {
            _headerIncludeDir = config[_globalConfigHeaderIncludeDirKey].get<std::string>();
            _produceHeaders = true;
        }
        if (config.contains(_globalConfigProduceHeadersKey))
        {
            _produceHeaders = config[_globalConfigProduceHeadersKey];
        }
        if (config.contains(_globalConfigOutputPathKey))
        {
            _outputPath = config[_globalConfigOutputPathKey].get<std::string>();
        }
        if (config.contains(_globalConfigFilesKey))
        {
            for (auto& item : config[_globalConfigFilesKey].items())
            {
                loadInputConfig(item.key(), item.value());
            }
        }
        if (config.contains(_globalConfigImportersKey))
        {
            _importersConfig = config[_globalConfigImportersKey];
        }
        return true;
    }

    nlohmann::json AssetImporterImpl::fixInputConfig(const nlohmann::json& json) noexcept
    {
        if (json.is_null())
        {
            return nlohmann::json::object();
        }
        if (json.is_primitive())
        {
            return nlohmann::json{
                { json.get<std::string>(), nullptr }
            };
        }
        if (json.is_array())
        {
            nlohmann::json obj;
            for (auto& elm : json)
            {
                if (elm.is_primitive())
                {
                    obj[elm.get<std::string>()] = nlohmann::json::object();
                }
                if (elm.is_object())
                {
                    obj[elm["name"].get<std::string>()] = elm;
                }
            }
            return obj;
        }
        return json;
    }

    void AssetImporterImpl::mergeJsonObjects(nlohmann::json& json1, const nlohmann::json& json2) noexcept
    {
        json1 += json2;
    }

    const nlohmann::json& AssetImporterImpl::loadInputConfig(const std::filesystem::path& path, const nlohmann::json& json) noexcept
    {
        auto fixedJson = fixInputConfig(json);
        auto itr = _inputs.find(path);
        if (itr == _inputs.end())
        {
            return _inputs.emplace(path, fixedJson).first->second;
        }
        mergeJsonObjects(itr->second, fixedJson);
        return itr->second;
    }

    void AssetImporterImpl::setOutputPath(const std::filesystem::path& outputPath) noexcept
    {
        _outputPath = outputPath;
    }

    AssetImporterImpl::ImporterInputs AssetImporterImpl::getImporterInputs() const
    {
        ImporterInputs inputs;
        for (auto& [path, inputConfig] : _inputs)
        {
            for (auto& [importerName, importerConfig ] : inputConfig.items())
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
                auto itr = inputConfig.find(importerName);
                nlohmann::json config;
                bool hasSpecificConfig = itr != inputConfig.end();
                if (hasSpecificConfig)
                {
                    config = *itr;
                }
                itr = _importersConfig.find(importerName);
                if (itr != _importersConfig.end())
                {
                    mergeJsonObjects(config, *itr);
                }
                inputs.emplace_back(*importer, Input{ path, config, hasSpecificConfig });
            }
        }
        return inputs;
    }

    AssetImporterImpl::PathGroups AssetImporterImpl::getPathGroups(const std::vector<std::filesystem::path>& paths) const noexcept
    {
        PathGroups groups;
        for (auto& path : paths)
        {
            // filename.lala.h -> filename.h
            auto groupPath = getHeaderPath(path.parent_path() / path.stem());
            groups[groupPath].push_back(path);
        }

        // remove groups with 1 element
        for (auto itr = groups.begin(); itr != groups.end(); )
        {
            if (itr->second.size() < 2)
            {
                itr = groups.erase(itr);
            }
            else
            {
                ++itr;
            }
        }
        return groups;
    }

    std::vector<fs::path> AssetImporterImpl::getOutputs() const
    {
        std::vector<fs::path> outputs;
        for (auto& [importer, input] : getImporterInputs())
        {
            auto size = getOutputs(importer.value(), input, outputs);
            if (_produceHeaders && size > 1)
            {
                auto groups = getPathGroups({ outputs.end() - size, outputs.end() });
                for (auto& [groupPath, paths] : groups)
                {
                    if (std::find(outputs.begin(), outputs.end(), groupPath) == outputs.end())
                    {
                        outputs.push_back(groupPath);
                    }
                }
            }
        }
        return outputs;
    }

    void AssetImporterImpl::addTypeImporter(std::unique_ptr<IAssetTypeImporter>&& importer) noexcept
    {
        _importers[importer->getName()] = std::move(importer);
    }

    void AssetImporterImpl::produceCombinedHeader(const std::filesystem::path& path, const std::vector<std::filesystem::path>& paths) const
    {
        auto fullPath = _outputPath / path;
        fs::create_directories(fullPath.parent_path());
        std::ofstream out(fullPath);
        out << "// generated antomatically by darmok, please do not modify manually!" << std::endl;
        for (auto& path : paths)
        {
            auto includePath = _headerIncludeDir / path.filename();
            out << "#include \"" << includePath.string() << "\"" << std::endl;
        }
    }

    void AssetImporterImpl::operator()(std::ostream& log) const
    {
        for (auto& [importer, input] : getImporterInputs())
        {
            auto outputs = importFile(importer.value(), input, log);
            if (_produceHeaders)
            {
                auto groups = getPathGroups(outputs);
                for (auto& [groupPath, paths] : groups)
                {
                    log << "combined header" << " " << groupPath  << "..." << std::endl;
                    produceCombinedHeader(groupPath, paths);
                }
            }
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

    std::vector<std::filesystem::path> AssetImporterImpl::importFile(IAssetTypeImporter& importer, const Input& input, std::ostream& log) const
    {
        std::vector<fs::path> outputs;
        if (!importer.getOutputs(input, outputs))
        {
            return outputs;
        }
        importer.setLogOutput(log);
        size_t i = 0;
        auto relInPath = fs::relative(input.path, _inputPath);
        for (fs::path& output : outputs)
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

            fs::create_directories(outPath.parent_path());
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

        return outputs;
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
        std::ifstream is(input.path, std::ios::binary);
        copyStream(is, out, _bufferSize);
    }

    std::string CopyAssetImporter::getName() const noexcept
    {
        static const std::string name("copy");
        return name;
    }

    const std::vector<std::string> ShaderImporterImpl::_profiles{
        "120", "300_es", "spirv",
#if BX_PLATFORM_WINDOWS
        "s_4_0", "s_5_0",
#endif
#if BX_PLATFORM_OSX
        "metal",
#endif
    };

    const std::unordered_map<std::string, std::string> ShaderImporterImpl::_profileExtensions{
        { "300_es", ".essl" },
        { "120", ".glsl" },
        { "spirv", ".spv" },
        { "metal", ".mtl" },
        { "s_3_0", ".dx9" },
        { "s_4_0", ".dx10" },
        { "s_5_0", ".dx11" }
    };

    const std::string ShaderImporterImpl::_binExt = ".bin";

    ShaderImporterImpl::ShaderImporterImpl(size_t bufferSize) noexcept
        : _shadercPath("shaderc")
        , _bufferSize(bufferSize)
    {
#if BX_PLATFORM_WINDOWS
        _shadercPath += ".exe";
#endif
    }

    void ShaderImporterImpl::setShadercPath(const std::filesystem::path& path) noexcept
    {
        _shadercPath = path;
    }

    void ShaderImporterImpl::addIncludePath(const std::filesystem::path& path) noexcept
    {
        _includes.push_back(path);
    }

    void ShaderImporterImpl::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _log = log;
    }

    std::filesystem::path ShaderImporterImpl::getOutputPath(const Input& input, const std::string& ext) noexcept
    {
        auto stem = input.path.stem().string();
        return stem + ext + _binExt;
    }

    bool ShaderImporterImpl::getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs)
    {
        auto fileName = input.path.filename().string();
        auto ext = StringUtils::getFileExt(fileName);
        if (ext != ".fragment.sc" && ext != ".vertex.sc")
        {
            return false;
        }
        
        for (auto& profile : _profiles)
        {
            auto itr = _profileExtensions.find(profile);
            if (itr != _profileExtensions.end())
            {
                outputs.push_back(getOutputPath(input, itr->second));
            }
        }
        return true;
    }

    std::ofstream ShaderImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return std::ofstream(path, std::ios::binary);
    }

    const std::string ShaderImporterImpl::_configTypeKey = "type";
    const std::string ShaderImporterImpl::_configVaryingDefKey = "varyingdef";
    const std::string ShaderImporterImpl::_configIncludeDirsKey = "includeDirs";

    void ShaderImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        if (!fs::exists(_shadercPath))
        {
            throw std::runtime_error("cannot find bgfx shaderc executable");
        }

        std::string type;
        if (input.config.contains(_configTypeKey))
        {
            type = input.config[_configTypeKey];
        }
        else
        {
            type = input.path.stem().extension().string().substr(1);
        }
        fs::path varyingDef = input.path.parent_path();
        if (input.config.contains(_configVaryingDefKey))
        {
            varyingDef /= input.config[_configVaryingDefKey].get<std::string>();
        }
        else
        {
            varyingDef /= input.path.stem().stem().string() + ".varyingdef";
        }

        std::vector<fs::path> includes(_includes);
        if (input.config.contains(_configIncludeDirsKey))
        {
            for (auto& elm : input.config[_configIncludeDirsKey])
            {
                includes.emplace_back(elm.get<std::string>());
            }
        }
        else
        {
            includes.push_back(input.path.parent_path());
        }

        auto& profile = _profiles[outputIndex];
        fs::path tmpPath = fs::temp_directory_path() / fs::path(std::tmpnam(nullptr));

        std::vector<std::string> args{
            fixPathArgument(_shadercPath),
            "-p", profile,
            "-f", fixPathArgument(input.path),
            "-o", fixPathArgument(tmpPath),
            "--type", type,
            "--varyingdef", fixPathArgument(varyingDef)
        };
        for (auto& include : includes)
        {
            args.push_back("-i");
            args.push_back(fixPathArgument(include));
        }

        auto r = exec(args);
        if (r.output.contains("Failed to build shader."))
        {
            if (_log)
            {
                *_log << "shaderc output:" << std::endl;
                *_log << r.output;
            }
            throw std::runtime_error("failed to build shader");
        }

        std::ifstream is(tmpPath, std::ios::binary);
        copyStream(is, out, _bufferSize);
    }

    std::string ShaderImporterImpl::fixPathArgument(const std::filesystem::path& path) noexcept
    {
        auto str = fs::absolute(path).string();
        std::replace(str.begin(), str.end(), '\\', '/');
        return str;
    }

    std::string ShaderImporterImpl::getName() const noexcept
    {
        static const std::string name("shader");
        return name;
    }

    ShaderImporter::ShaderImporter()
        : _impl(std::make_unique<ShaderImporterImpl>())
    {
    }

    ShaderImporter::~ShaderImporter() noexcept
    {
        // empty on purpose
    }

    bool ShaderImporter::getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs)
    {
        return _impl->getOutputs(input, outputs);
    }

    std::ofstream ShaderImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return _impl->createOutputStream(input, outputIndex, path);
    }

    void ShaderImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        return _impl->writeOutput(input, outputIndex, out);
    }

    std::string ShaderImporter::getName() const noexcept
    {
        return _impl->getName();
    }

    ShaderImporter& ShaderImporter::setShadercPath(const std::filesystem::path& path) noexcept
    {
        _impl->setShadercPath(path);
        return *this;
    }

    ShaderImporter& ShaderImporter::addIncludePath(const std::filesystem::path& path) noexcept
    {
        _impl->addIncludePath(path);
        return *this;
    }

    void ShaderImporter::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _impl->setLogOutput(log);
    }

    DarmokCoreAssetImporter::DarmokCoreAssetImporter(const std::filesystem::path& inputPath)
        : _importer(inputPath)
        , _shaderImporter(_importer.addTypeImporter<ShaderImporter>())
    {
        _importer.addTypeImporter<CopyAssetImporter>();
        _importer.addTypeImporter<VertexLayoutImporter>();
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::setOutputPath(const std::filesystem::path& outputPath) noexcept
    {
        _importer.setOutputPath(outputPath);
        return *this;
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::setShadercPath(const std::filesystem::path& path) noexcept
    {
        _shaderImporter.setShadercPath(path);
        return *this;
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::addShaderIncludePath(const std::filesystem::path& path) noexcept
    {
        _shaderImporter.addIncludePath(path);
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