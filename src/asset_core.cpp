#include "asset_core.hpp"
#include <darmok/asset_core.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/stream.hpp>
#include <darmok/image.hpp>
#include <darmok/program_core.hpp>
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

        for (auto& [dirPath, dirConfig] : _dirs)
        {
            for (auto& [path, dirFileConfig] : dirConfig.files)
            {
                if (_files.contains(path))
                {
                    continue;
                }
                FileConfig config;
                config.load(dirFileConfig);
                _files.emplace(path, config);
            }
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
    const std::string AssetImporterImpl::DirConfig::_includesKey = "includes";
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
                auto& fileConfig = itr->second;
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
        {
            auto itr = fileMatches.find(op.input.path);
            if (itr != fileMatches.end())
            {
                op.input.pathMatches = itr->second;
            }
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
                std::smatch match;
                if (std::regex_match(relPath, match, regex))
                {
                    std::vector<std::string> matchGroups;
                    for (auto& group : match)
                    {
                        matchGroups.push_back(group.str());
                    }
                    fileMatches[filePath] = matchGroups;
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

        nlohmann::json includes;
        if (config.contains(_includesKey))
        {
            includes = config[_includesKey];
        }

        if (config.contains(_filesKey))
        {
            nlohmann::json filesJson = config[_filesKey];
            if (!includes.empty())
            {
                FileConfig::replaceIncludes(filesJson, includes);
            }
            auto basePath = path.parent_path();
            for (auto& item : filesJson.items())
            {
                loadFile(item.key(), item.value(), basePath, filePaths);
            }
        }
        if (config.contains(_importersKey))
        {
            importers = FileConfig::fix(config[_importersKey]);
            if (!includes.empty())
            {
                FileConfig::replaceIncludes(importers, includes);
            }
        }
        return true;
    }

    const std::string AssetImporterImpl::FileConfig::_configFileSuffix = ".darmok-import.json";
    const std::string AssetImporterImpl::FileConfig::_importersKey = "importers";
    const std::string AssetImporterImpl::FileConfig::_includesKey = "includes";
    const std::regex AssetImporterImpl::FileConfig::_includePattern("\\$\\{(.+)\\}");
    const std::string AssetImporterImpl::FileConfig::_includePatternToken = "${";

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
        load(nlohmann::json::parse(std::ifstream(path)));
        return true;
    }

    void AssetImporterImpl::FileConfig::load(const nlohmann::json& json)
    {
        auto config = fix(json);
        if (config.contains(_importersKey))
        {
            importers = fix(config[_importersKey]);
            if (config.contains(_includesKey))
            {
                auto& includes = config[_includesKey];
                replaceIncludes(importers, includes);
            }
        }
        else
        {
            importers = config;
        }
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

    bool AssetImporterImpl::FileConfig::replaceIncludes(nlohmann::json& json, const nlohmann::json& includes) noexcept
    {
        auto replace = [&includes](nlohmann::json& elm)
        {
            auto changed = false;
            if (!elm.is_string())
            {
                return changed;
            }
            std::string str = elm;
            if (!str.contains(_includePatternToken)) // fast check
            {
                return changed;
            }
            std::smatch match;
            if (std::regex_match(str, match, _includePattern))
            {
                auto itr = includes.find(match[1]);
                if (itr != includes.end())
                {
                    changed = true;
                    elm = itr.value();
                }
            }
            else
            {
                changed = StringUtils::regexReplace(str, _includePattern, [&includes](auto& match, auto& repl) {
                    auto itr = includes.find(match[1]);
                    if (itr != includes.end())
                    {
                        repl = itr.value();
                        return true;
                    }
                    return false;
                });
                if (changed)
                {
                    elm = str;
                }
            }
            return changed;
        };

        auto changed = false;
        if (json.is_array())
        {
            for (auto& elm : json)
            {
                if (replaceIncludes(elm, includes))
                {
                    changed = true;
                }
            }
        }
        else if (json.is_object())
        {
            for (auto& elm : json.items())
            {
                if (replaceIncludes(elm.value(), includes))
                {
                    changed = true;
                }
            }
        }
        else
        {
            while (replace(json))
            {
                changed = true;
            }
        }
        return changed;
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
            addFileCachePath(path);
            _files.emplace(path, config);
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
        auto basePath = fs::absolute(cachePath).parent_path();
        auto fileName = fs::relative(fs::absolute(_inputPath), basePath).string();
        StringUtils::replace(fileName, "..", "@");
        static const std::string separators("\\/:");
        for (auto& chr : separators)
        {
            std::replace(fileName.begin(), fileName.end(), chr, '-');
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
        std::string pathStr(path.string());
        StringUtils::replace(pathStr, "*", "_");
        StringUtils::replace(pathStr, "?", "_");
        return fs::weakly_canonical(pathStr).make_preferred();
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
        auto parentPath = path.parent_path();
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
                auto r = _fileDependencies.emplace(op.input.path, Dependencies());
                auto& deps = r.first->second;
                getDependencies(op.input.path, ops, deps);
                for (auto& dep : deps)
                {
                    addFileCachePath(dep);
                }
            }
        }
    }

    void AssetImporterImpl::getDependencies(const fs::path& path, const std::vector<Operation>& ops, Dependencies& deps) const
    {
        Dependencies baseDeps;
        for (auto& op : ops)
        {
            if (op.input.path != path)
            {
                continue;
            }
            if (!op.importer.startImport(op.input, true))
            {
                continue;
            }
            auto importerDeps = op.importer.getDependencies(op.input);
            op.importer.endImport(op.input);
            for (auto& dep : importerDeps)
            {
                if (std::find(deps.begin(), deps.end(), dep) == deps.end())
                {
                    deps.insert(dep);
                    baseDeps.insert(dep);
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
        std::filesystem::path output = path;
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
        auto relInput = fs::relative(op.input.path, _inputPath);
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
            auto relOutput = fs::relative(output, _outputPath);
            log << op.importer.getName() << ": " << relInput << " -> " << relOutput << "..." << std::endl;
            result.updatedOutputs.push_back(output);
            fs::create_directories(output.parent_path());
            if (op.headerConfig.produceHeaders)
            {
                Data data;
                DataOutputStream ds(data);
                op.importer.writeOutput(op.input, i, ds);
                std::streamoff pos = ds.tellp();
                std::ofstream os(output);
                os << data.view(0, pos).toHeader(headerVarName);
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
        StreamUtils::copy(is, out, _bufferSize);
    }

    const std::string& CopyAssetImporter::getName() const noexcept
    {
        static const std::string name("copy");
        return name;
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
        , _progImporter(_importer.addTypeImporter<ProgramImporter>())
    {
        _importer.addTypeImporter<CopyAssetImporter>();
        _importer.addTypeImporter<ImageImporter>();
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
        _progImporter.setShadercPath(path);
        return *this;
    }

    DarmokCoreAssetImporter& DarmokCoreAssetImporter::addShaderIncludePath(const fs::path& path) noexcept
    {
        _progImporter.addIncludePath(path);
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