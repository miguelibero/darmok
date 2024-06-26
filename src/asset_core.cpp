#include "asset_core.hpp"
#include <darmok/asset_core.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/stream.hpp>
#include <darmok/utils.hpp>
#include <bx/platform.h>
#include <bx/commandline.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <chrono>

namespace darmok
{
    namespace fs = std::filesystem;

    AssetImporterImpl::AssetImporterImpl(const fs::path& inputPath)
    {
        std::vector<fs::path> inputPaths;
        if (fs::is_directory(inputPath))
        {
            _inputPath = inputPath;
            fs::recursive_directory_iterator beg(inputPath), end;
            inputPaths.push_back(inputPath);
            inputPaths.insert(inputPaths.begin(), beg, end);
        }
        else if (fs::exists(inputPath))
        {
            _inputPath = inputPath.parent_path();
            inputPaths.push_back(inputPath);
        }
        else
        {
            throw std::runtime_error("input path does not exist");
        }
        for (auto& path : inputPaths)
        {
            loadInput(path, inputPaths);
        }
    }

    const std::string AssetImporterImpl::HeaderConfig::_headerVarPrefixKey = "headerVarPrefix";
    const std::string AssetImporterImpl::HeaderConfig::_produceHeadersKey = "produceHeaders";
    const std::string AssetImporterImpl::HeaderConfig::_headerIncludeDirKey = "headerIncludeDir";

    bool AssetImporterImpl::HeaderConfig::load(const nlohmann::json& json) noexcept
    {
        bool found = false;
        if (json.contains(_headerVarPrefixKey))
        {
            varPrefix = json[_headerVarPrefixKey];
            produceHeaders = true;
            found = true;
        }
        if (json.contains(_headerIncludeDirKey))
        {
            includeDir = json[_headerIncludeDirKey].get<std::string>();
            produceHeaders = true;
            found = true;
        }
        if (json.contains(_produceHeadersKey))
        {
            produceHeaders = json[_produceHeadersKey];
            found = true;
        }
        return found;
    }

    const std::string AssetImporterImpl::DirConfig::_configFileName = "darmok-import.json";
    const std::string AssetImporterImpl::DirConfig::_filesKey = "files";
    const std::string AssetImporterImpl::DirConfig::_importersKey = "importers";
    const std::string AssetImporterImpl::DirConfig::_outputPathKey = "outputPath";

    fs::path AssetImporterImpl::DirConfig::getPath(const fs::path& path) noexcept
    {
        return path / _configFileName;
    }

    bool AssetImporterImpl::DirConfig::isPath(const fs::path& path) noexcept
    {
        return path.filename() == _configFileName;
    }

    void AssetImporterImpl::DirConfig::updateOperation(Operation& op, const std::string& importerName) const
    {
        {
            auto itr = files.find(op.input.path);
            if (itr != files.end())
            {
                auto fileConfig = itr->second;
                auto itr2 = fileConfig.find(importerName);
                if (itr2 != fileConfig.end())
                {
                    mergeConfig(op.input.config, *itr2);
                }
            }
        }
        {
            auto itr = importers.find(importerName);
            if (itr != importers.end())
            {
                mergeConfig(op.input.dirConfig, *itr);
            }
        }
        if (header)
        {
            op.headerConfig = header.value();
        }
        if (outputPath)
        {
            op.outputPath = outputPath.value();
        }
    }

    void AssetImporterImpl::DirConfig::loadFile(const std::string& key, const nlohmann::json& config, const fs::path& basePath, const std::vector<fs::path>& filePaths) noexcept
    {
        auto fixedConfig = FileConfig::fix(config);
        if (StringUtils::containsGlobPattern(key))
        {
            std::regex regex(StringUtils::globToRegex(key));
            for (auto& filePath : filePaths)
            {
                if (isPath(filePath) || FileConfig::isPath(filePath))
                {
                    continue;
                }
                auto relPath = fs::relative(filePath, basePath).string();
                if (std::regex_match(relPath, regex))
                {
                    files[filePath] = fixedConfig;
                }
            }
        }
        else
        {
            auto filePath = basePath / key;
            files[filePath] = fixedConfig;
        }
    }

    bool AssetImporterImpl::DirConfig::load(const fs::path& inputPath, const std::vector<fs::path>& filePaths)
    {
        path = isPath(inputPath) ? inputPath : getPath(inputPath);
        if (!fs::exists(path))
        {
            return false;
        }
        auto config = nlohmann::json::parse(std::ifstream(path));

        HeaderConfig headerConfig;
        if (headerConfig.load(config))
        {
            header = headerConfig;
        }
        if (config.contains(_outputPathKey))
        {
            outputPath = config[_outputPathKey].get<std::string>();
        }
        if (config.contains(_filesKey))
        {
            auto basePath = path.parent_path();
            for (auto& item : config[_filesKey].items())
            {
                loadFile(item.key(), item.value(), basePath, filePaths);
            }
        }
        if (config.contains(_importersKey))
        {
            importers.update(config[_importersKey]);
        }
        return true;
    }

    const std::string AssetImporterImpl::FileConfig::_configFileSuffix = ".darmok-import.json";

    fs::path AssetImporterImpl::FileConfig::getPath(const fs::path& path) noexcept
    {
        return path.string() + _configFileSuffix;
    }

    bool AssetImporterImpl::FileConfig::isPath(const fs::path& path) noexcept
    {
        return path.filename().string().ends_with(_configFileSuffix);
    }

    bool AssetImporterImpl::FileConfig::load(const fs::path& inputPath)
    {
        if (!fs::exists(inputPath))
        {
            return false;
        }
        path = isPath(inputPath) ? inputPath : getPath(inputPath);
        if (!fs::exists(path))
        {
            return false;
        }
        auto config = nlohmann::json::parse(std::ifstream(path));
        importers = fix(config);
        return true;
    }

    nlohmann::json AssetImporterImpl::FileConfig::fix(const nlohmann::json& json) noexcept
    {
        if (json.is_null())
        {
            return nlohmann::json::object();
        }
        if (json.is_primitive())
        {
            return nlohmann::json{
                { json.get<std::string>(), nlohmann::json::object() }
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

    bool AssetImporterImpl::loadInput(const fs::path& path, const std::vector<fs::path>& paths)
    {
        if (FileConfig::isPath(path) || DirConfig::isPath(path))
        {
            return false;
        }
        if (fs::is_directory(path))
        {
            DirConfig config;
            if (config.load(path, paths))
            {
                _dirs.emplace(path, config);
                addFileCachePath(config.path);
            }
        }
        else
        {
            FileConfig config;
            if (config.load(path))
            {
                addFileCachePath(config.path);
            }
            _files.emplace(path, config);
            addFileCachePath(path);
        }
        return true;
    }

    std::time_t AssetImporterImpl::getUpdateTime(const fs::path& path)
    {
        if (!fs::exists(path))
        {
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        }
        fs::file_time_type ftime = fs::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() +
            std::chrono::system_clock::now()
        );
        return std::chrono::system_clock::to_time_t(sctp);
    }

    bool AssetImporterImpl::addFileCachePath(const fs::path& path, std::time_t cacheTime) const noexcept
    {
        if (!fs::exists(path))
        {
            return false;
        }
        auto normPath = normalizePath(path);
        auto itr = _fileCache.find(normPath);
        auto updateTime = getUpdateTime(normPath);
        if (itr == _fileCache.end())
        {
            _fileCache.emplace(normPath, FileCacheData{ updateTime, cacheTime });
            return true;
        }
        auto& data = itr->second;
        if (data.cacheTime < cacheTime)
        {
            data.cacheTime = cacheTime;
        }
        if (data.updateTime < updateTime)
        {
            data.updateTime = updateTime;
        }
        return false;
    }

    void AssetImporterImpl::setCachePath(const fs::path& cachePath) noexcept
    {
        auto fileName = fs::absolute(_inputPath.string()).string();
        static const std::string separators("\\/:");
        for (auto& chr : separators)
        {
            std::replace(fileName.begin(), fileName.end(), chr, '_');
        }
        _cachePath = cachePath / (fileName + ".json");
        if (fs::exists(_cachePath))
        {
            auto cache = nlohmann::json::parse(std::ifstream(_cachePath));
            for (auto& [relPath, cacheTime] : cache.items())
            {
                addFileCachePath(_inputPath / relPath, cacheTime);
            }
        }
    }

    bool AssetImporterImpl::isCached(const fs::path& path) const noexcept
    {
        for (auto& config : getDirConfigs(path))
        {
            if (!isPathCached(config->path))
            {
                return false;
            }
        }
        auto itr = _fileDependencies.find(path);
        if (itr != _fileDependencies.end())
        {
            for (auto& dep : itr->second)
            {
                if (!isPathCached(FileConfig::getPath(dep)))
                {
                    return false;
                }
                if (!isPathCached(dep))
                {
                    return false;
                }
            }
        }
        if (!isPathCached(FileConfig::getPath(path)))
        {
            return false;
        }
        return isPathCached(path);
    }

    fs::path AssetImporterImpl::normalizePath(const fs::path& path) noexcept
    {
        return fs::weakly_canonical(path).make_preferred();
    }

    bool AssetImporterImpl::isPathCached(const fs::path& path) const noexcept
    {
        auto normPath = normalizePath(path);
        auto itr = _fileCache.find(normPath);
        if (itr == _fileCache.end())
        {
            if (fs::exists(normPath))
            {
                return false;
            }
            return true;
        }
        auto& data = itr->second;
        if (data.updateTime == 0)
        {
            data.updateTime = getUpdateTime(normPath);
        }
        if (data.updateTime > data.cacheTime)
        {
            return false;
        }
        return true;
    }

    bool AssetImporterImpl::isCacheUpdated() const noexcept
    {
        if (_cachePath.empty())
        {
            return false;
        }
        for (auto& [path, data] : _fileCache)
        {
            if (data.updateTime > data.cacheTime)
            {
                return true;
            }
        }
        return false;
    }

    bool AssetImporterImpl::writeCache() const
    {
        if (_cachePath.empty())
        {
            return false;
        }
        auto cache = nlohmann::json::object();
        for (auto& elm : _fileCache)
        {
            auto relPath = fs::relative(elm.first, _inputPath).string();
            cache[relPath] = elm.second.updateTime;
        }
        fs::create_directories(_cachePath.parent_path());
        std::ofstream os(_cachePath);
        os << cache.dump(2);
        os.close();
        return true;
    }

    void AssetImporterImpl::setOutputPath(const fs::path& outputPath) noexcept
    {
        _outputPath = outputPath;
    }

    AssetImporterImpl::DirConfigs AssetImporterImpl::getDirConfigs(const fs::path& path) const noexcept
    {
        DirConfigs configs;
        auto parentPath = fs::absolute(path).parent_path();
        while (parentPath != parentPath.root_path())
        {
            auto itr = _dirs.find(parentPath);
            if (itr != _dirs.end())
            {
                configs.emplace_back(itr->second);
            }
            parentPath = parentPath.parent_path();
        }
        std::reverse(configs.begin(), configs.end());
        return configs;
    }

    void AssetImporterImpl::mergeConfig(nlohmann::json& json, const nlohmann::json& other)
    {
        if (json.empty())
        {
            json = other;
        }
        else
        {
            json.update(other);
        }
    }

    std::vector<AssetImporterImpl::Operation> AssetImporterImpl::getOperations() const
    {
        std::vector<Operation> ops;
        for (auto& [path, fileConfig] : _files)
        {
            for (auto& [importerName, importerConfig] : fileConfig.importers.items())
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
                auto& op = ops.emplace_back(*importer, Input{ path, _inputPath });
                auto itr = fileConfig.importers.find(importerName);
                if (itr != fileConfig.importers.end())
                {
                    op.input.config = *itr;
                }
                for (auto& dirConfig : getDirConfigs(path))
                {
                    dirConfig->updateOperation(op, importerName);
                }
            }
        }
        loadDependencies(ops);
        return ops;
    }

    void AssetImporterImpl::loadDependencies(const std::vector<Operation>& ops) const
    {
        for (auto& op : ops)
        {
            auto itr = _fileDependencies.find(op.input.path);
            if (itr == _fileDependencies.end())
            {
                auto r = _fileDependencies.emplace(op.input.path, std::vector<fs::path>());
                auto& deps = r.first->second;
                getDependencies(op.input.path, ops, deps);
                for (auto& dep : deps)
                {
                    addFileCachePath(dep);
                }
            }
        }
    }

    void AssetImporterImpl::getDependencies(const fs::path& path, const std::vector<Operation>& ops, std::vector<fs::path>& deps) const
    {
        std::vector<fs::path> baseDeps;
        for (auto& op : ops)
        {
            if (op.input.path != path)
            {
                continue;
            }
            for (auto& dep : op.importer.getDependencies(op.input))
            {
                if (std::find(deps.begin(), deps.end(), dep) == deps.end())
                {
                    deps.push_back(dep);
                    baseDeps.push_back(dep);
                }
            }
        }
        for (auto& dep : baseDeps)
        {
            getDependencies(dep, ops, deps);
        }
    }

    AssetImporterImpl::PathGroups AssetImporterImpl::getPathGroups(const std::vector<fs::path>& paths) const noexcept
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
        for (auto& op : getOperations())
        {
            auto importerOutputs = getOutputs(op);
            if (importerOutputs.empty())
            {
                continue;
            }
            for (auto& output : importerOutputs)
            {
                if (std::find(outputs.begin(), outputs.end(), output) != outputs.end())
                {
                    throw std::runtime_error(std::string("multiple importers produce the same output: ") + output.string());
                }
            }

            auto allOutputsCached = true;
            if (!isCached(op.input.path))
            {
                outputs.insert(outputs.end(), importerOutputs.begin(), importerOutputs.end());
                allOutputsCached = false;
            }
            else
            {
                for (auto& output : importerOutputs)
                {
                    if (!fs::exists(output))
                    {
                        outputs.push_back(output);
                        allOutputsCached = false;
                    }
                }
            }
            if (op.headerConfig.produceHeaders && !allOutputsCached)
            {
                auto groups = getPathGroups(importerOutputs);
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

    void AssetImporterImpl::produceCombinedHeader(const fs::path& path, const std::vector<fs::path>& paths, const fs::path& includeDir) const
    {
        auto fullPath = _outputPath / path;
        fs::create_directories(fullPath.parent_path());
        std::ofstream out(fullPath);
        out << "// generated antomatically by darmok, please do not modify manually!" << std::endl;
        for (auto& path : paths)
        {
            auto includePath = includeDir / path.filename();
            out << "#include \"" << includePath.string() << "\"" << std::endl;
        }
    }

    void AssetImporterImpl::operator()(std::ostream& log) const
    {
        log << "importing " << _inputPath << " -> " << _outputPath << "..." << std::endl;
        for (auto& op : getOperations())
        {
            auto result = importFile(op, log);
            if (op.headerConfig.produceHeaders && !result.updatedOutputs.empty())
            {
                auto groups = getPathGroups(result.outputs);
                for (auto& [groupPath, paths] : groups)
                {
                    log << "combined header " << groupPath  << "..." << std::endl;
                    produceCombinedHeader(groupPath, paths, op.headerConfig.includeDir);
                }
            }
        }
        if (isCacheUpdated())
        {
            log << "writing cache " << _cachePath << "..." << std::endl;
            writeCache();
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

    std::filesystem::path AssetImporterImpl::fixOutput(const std::filesystem::path& path, const Operation& op) const noexcept
    {
        auto output = path;
        if (op.headerConfig.produceHeaders)
        {
            output = getHeaderPath(output);
        }
        return normalizePath(_outputPath / op.outputPath / output);
    }

    std::vector<fs::path> AssetImporterImpl::getOutputs(const Operation& op) const
    {
        if (!op.importer.startImport(op.input, true))
        {
            return {};
        }
        auto outputs = op.importer.getOutputs(op.input);
        for (fs::path& output : outputs)
        {
            output = fixOutput(output, op);
        }
        op.importer.endImport(op.input);
        return outputs;
    }

    AssetImporterImpl::FileImportResult AssetImporterImpl::importFile(const Operation& op, std::ostream& log) const
    {
        FileImportResult result;

        if (!op.importer.startImport(op.input, false))
        {
            return result;
        }

        result.outputs = op.importer.getOutputs(op.input);
        if (result.outputs.empty())
        {
            op.importer.endImport(op.input);
            return result;
        }

        result.inputCached = isCached(op.input.path);
        op.importer.setLogOutput(log);
        size_t i = 0;
        auto relInPath = fs::relative(op.input.path, _inputPath);
        for (fs::path& output : result.outputs)
        {
            std::string headerVarName;
            if (op.headerConfig.produceHeaders)
            {
                auto stem = output.stem().string();
                headerVarName = op.headerConfig.varPrefix + stem;
            }
            output = fixOutput(output, op);
            if (result.inputCached && fs::exists(output))
            {
                // log << importer.getName() << ": skipping " << relInPath << " -> " << relOutPath << std::endl;
                continue;
            }
            log << op.importer.getName() << ": " << relInPath << " -> " << output << "..." << std::endl;
            result.updatedOutputs.push_back(output);
            fs::create_directories(output.parent_path());
            if (op.headerConfig.produceHeaders)
            {
                Data data;
                DataOutputStream ds(data);
                op.importer.writeOutput(op.input, i, ds);
                std::ofstream os(output);
                os << data.view().toHeader(headerVarName);
            }
            else
            {
                auto out = op.importer.createOutputStream(op.input, i, output);
                op.importer.writeOutput(op.input, i, out);
            }
            ++i;
        }
        op.importer.endImport(op.input);
        return result;
    }

    AssetImporter::AssetImporter(const fs::path& inputPath)
        : _impl(std::make_unique<AssetImporterImpl>(inputPath))
    {
    }

    AssetImporter::~AssetImporter() noexcept
    {
        // empty on purpose
    }

    AssetImporter& AssetImporter::setCachePath(const fs::path& cachePath) noexcept
    {
        _impl->setCachePath(cachePath);
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

    CopyAssetImporter::CopyAssetImporter(size_t bufferSize) noexcept
        : _bufferSize(bufferSize)
    {
    }

    std::vector<fs::path> CopyAssetImporter::getOutputs(const Input& input)
    {
        std::vector<fs::path> outputs;
        if (input.config.is_null() && (!input.dirConfig.is_object() || input.dirConfig["all"] != true))
        {
            return outputs;
        }
        if (input.config.contains("outputPath"))
        {
            outputs.push_back(input.config["outputPath"]);
        }
        else
        {
            outputs.push_back(input.getRelativePath());
        }
        return outputs;
    }

    void CopyAssetImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        std::ifstream is(input.path, std::ios::binary);
        StreamUtils::copyStream(is, out, _bufferSize);
    }

    const std::string& CopyAssetImporter::getName() const noexcept
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

    void ShaderImporterImpl::setShadercPath(const fs::path& path) noexcept
    {
        _shadercPath = path;
    }

    void ShaderImporterImpl::addIncludePath(const fs::path& path) noexcept
    {
        _includes.push_back(path);
    }

    void ShaderImporterImpl::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _log = log;
    }

    fs::path ShaderImporterImpl::getOutputPath(const fs::path& path, const std::string& ext) noexcept
    {
        auto stem = path.stem().string();
        return path.parent_path() / (stem + ext + _binExt);
    }

    std::vector<fs::path> ShaderImporterImpl::getOutputs(const Input& input)
    {
        std::vector<fs::path> outputs;
        auto fileName = input.path.filename().string();
        auto ext = StringUtils::getFileExt(fileName);
        if (ext != ".fragment.sc" && ext != ".vertex.sc")
        {
            return outputs;
        }
        for (auto& profile : _profiles)
        {
            auto itr = _profileExtensions.find(profile);
            if (itr != _profileExtensions.end())
            {
                outputs.push_back(getOutputPath(input.getRelativePath(), itr->second));
            }
        }
        return outputs;
    }

    std::vector<fs::path> ShaderImporterImpl::getDependencies(const Input& input)
    {
        std::vector<fs::path> deps;
        if (!fs::exists(input.path))
        {
            return deps;
        }
        auto includes = getIncludes(input);
        std::ifstream is(input.path);
        std::string line;
        while (std::getline(is, line))
        {
            std::smatch match;
            if (!std::regex_search(line, match, _includeRegex))
            {
                continue;
            }
            auto name = match[1].str();
            for (auto& include : includes)
            {
                auto path = include / name;
                if (fs::exists(path))
                {
                    if (std::find(deps.begin(), deps.end(), path) == deps.end())
                    {
                        deps.push_back(path);
                    }
                    break;
                }
            }
        }
        return deps;
    }

    const std::string ShaderImporterImpl::_configTypeKey = "type";
    const std::string ShaderImporterImpl::_configVaryingDefKey = "varyingdef";
    const std::string ShaderImporterImpl::_configIncludeDirsKey = "includeDirs";
    const std::regex ShaderImporterImpl::_includeRegex = std::regex("#include <([^>]+)>");

    std::vector<fs::path> ShaderImporterImpl::getIncludes(const Input& input) const noexcept
    {
        std::vector<fs::path> includes(_includes);
        if (input.config.contains(_configIncludeDirsKey))
        {
            for (auto& elm : input.config[_configIncludeDirsKey])
            {
                includes.emplace_back(input.basePath / elm.get<std::string>());
            }
        }
        else
        {
            includes.push_back(input.path.parent_path());
        }
        return includes;
    }

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

        auto includes = getIncludes(input);

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
        StreamUtils::copyStream(is, out, _bufferSize);
    }

    std::string ShaderImporterImpl::fixPathArgument(const fs::path& path) noexcept
    {
        auto str = fs::absolute(path).string();
        std::replace(str.begin(), str.end(), '\\', '/');
        return str;
    }

    const std::string& ShaderImporterImpl::getName() const noexcept
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

    std::vector<fs::path> ShaderImporter::getOutputs(const Input& input)
    {
        return _impl->getOutputs(input);
    }

    std::vector<fs::path> ShaderImporter::getDependencies(const Input& input)
    {
        return _impl->getDependencies(input);
    }

    void ShaderImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        return _impl->writeOutput(input, outputIndex, out);
    }

    const std::string& ShaderImporter::getName() const noexcept
    {
        return _impl->getName();
    }

    ShaderImporter& ShaderImporter::setShadercPath(const fs::path& path) noexcept
    {
        _impl->setShadercPath(path);
        return *this;
    }

    ShaderImporter& ShaderImporter::addIncludePath(const fs::path& path) noexcept
    {
        _impl->addIncludePath(path);
        return *this;
    }

    void ShaderImporter::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _impl->setLogOutput(log);
    }

    DarmokCoreAssetImporter::DarmokCoreAssetImporter(const CommandLineAssetImporterConfig& config)
        : DarmokCoreAssetImporter(config.inputPath)
    {
        if (!config.cachePath.empty())
        {
            setCachePath(config.cachePath);
        }
        if (!config.outputPath.empty())
        {
            setOutputPath(config.outputPath);
        }
        if (!config.shadercPath.empty())
        {
            setShadercPath(config.shadercPath);
        }
        for (auto& path : config.shaderIncludePaths)
        {
            addShaderIncludePath(path);
        }
    }

    DarmokCoreAssetImporter::DarmokCoreAssetImporter(const fs::path& inputPath)
        : _importer(inputPath)
        , _shaderImporter(_importer.addTypeImporter<ShaderImporter>())
    {
        _importer.addTypeImporter<CopyAssetImporter>();
        _importer.addTypeImporter<VertexLayoutImporter>();
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::setCachePath(const fs::path& cachePath) noexcept
    {
        _importer.setCachePath(cachePath);
        return *this;
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::setOutputPath(const fs::path& outputPath) noexcept
    {
        _importer.setOutputPath(outputPath);
        return *this;
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::setShadercPath(const fs::path& path) noexcept
    {
        _shaderImporter.setShadercPath(path);
        return *this;
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::addShaderIncludePath(const fs::path& path) noexcept
    {
        _shaderImporter.addIncludePath(path);
        return *this;
    }

    std::vector<fs::path> DarmokCoreAssetImporter::getOutputs() const
    {
        return _importer.getOutputs();
    }

    void DarmokCoreAssetImporter::operator()(std::ostream& out) const
    {
        _importer(out);
    }

    BaseCommandLineAssetImporter::BaseCommandLineAssetImporter() noexcept
        : _impl(std::make_unique<CommandLineAssetImporterImpl>(*this))
    {
    }

    BaseCommandLineAssetImporter::~BaseCommandLineAssetImporter() noexcept
    {
        // empty on purpose
    }

    int BaseCommandLineAssetImporter::operator()(int argc, const char* argv[]) noexcept
    {
        return (*_impl)(argc, argv);
    }

    CommandLineAssetImporterImpl::CommandLineAssetImporterImpl(BaseCommandLineAssetImporter& importer) noexcept
      : _importer(importer)
    {
    }

    int CommandLineAssetImporterImpl::operator()(int argc, const char* argv[]) noexcept
    {
        bx::CommandLine cmdLine(argc, argv);
        auto path = std::string(cmdLine.get(0));
        auto name = fs::path(path).filename().string();
        try
        {
            return run(name, cmdLine);
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Exception thrown:" << std::endl;
            std::cerr << ex.what() << std::endl;
            return bx::kExitFailure;
        }
    }

    void CommandLineAssetImporterImpl::version(const std::string& name) noexcept
    {
	    std::cout << name << ": darmok asset compile tool." << std::endl;
    }

    void CommandLineAssetImporterImpl::help(const std::string& name, const char* error) noexcept
    {
        if (error)
        {
            std::cerr << "Error:" << std::endl << error << std::endl << std::endl;
        }
        version(name);
        std::cout << "Usage: " << name << " -i <in> -o <out>" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h, --help              Display this help and exit." << std::endl;
        std::cout << "  -v, --version           Output version information and exit." << std::endl;
        std::cout << "  -i, --input <path>      Input file path (can be a file or a directory)." << std::endl;
        std::cout << "  -o, --output <path>     Output file path (can be a file or a directory)." << std::endl;
        std::cout << "  -c, --cache <path>      Cache file path (directory that keeps the timestamps of the inputs)." << std::endl;
        std::cout << "  -d, --dry               Do not process assets, just print output files." << std::endl;
        std::cout << "  --bgfx-shaderc          Path of the bgfx shaderc executable." << std::endl;
        std::cout << "  --bgfx-shader-include   Path of the bgfx shader include dir." << std::endl;
    }

    int CommandLineAssetImporterImpl::run(const std::string& name, const bx::CommandLine cmdLine)
    {
        if (cmdLine.hasArg('h', "help"))
        {
            help(name);
            return bx::kExitSuccess;
        }

        if (cmdLine.hasArg('v', "version"))
        {
            version(name);
            return bx::kExitSuccess;
        }

        const char* inputPath = cmdLine.findOption('i', "input");
        if (inputPath == nullptr)
        {
            throw std::runtime_error("Input file path must be specified.");
        }
        Config config;
        config.inputPath = inputPath;

        const char* outputPath = cmdLine.findOption('o', "output");
        if (outputPath != nullptr)
        {
            config.outputPath = outputPath;
        }

        const char* cachePath = cmdLine.findOption('c', "cache");
        if (cachePath != nullptr)
        {
            config.cachePath = cachePath;
        }

        const char* shadercPath = cmdLine.findOption("bgfx-shaderc");
        if (shadercPath != nullptr)
        {
            config.shadercPath = shadercPath;
        }

        // TODO: support multiple includes
        const char* shaderIncludePath = cmdLine.findOption("bgfx-shader-include");
        if (shaderIncludePath != nullptr)
        {
            config.shaderIncludePaths.push_back(shaderIncludePath);
        }

        if (cmdLine.hasArg('d', "dry"))
        {
            for (auto& output : _importer.getOutputs(config))
            {
                std::cout << output.string() << std::endl;
            }
        }
        else
        {
            PrefixStream log(std::cout, name + ": ");
            _importer.import(config, log);
        }
        return bx::kExitSuccess;
    }
}