#include "detail/asset_core.hpp"
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/stream.hpp>
#include <darmok/image.hpp>
#include <darmok/texture.hpp>
#include <darmok/program_core.hpp>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <chrono>

#include <bx/platform.h>
#include <CLI/CLI.hpp>

namespace darmok
{
    namespace fs = std::filesystem;

    std::filesystem::path FileTypeImporterInput::getRelativePath() const noexcept
    {
        return std::filesystem::relative(path, basePath);
    }

    std::optional<const nlohmann::json> FileTypeImporterInput::getConfigField(std::string_view key) const noexcept
    {
        auto itr = config.find(key);
        if (itr != config.end())
        {
            return { *itr };
        }
        itr = dirConfig.find(key);
        if (itr != dirConfig.end())
        {
            return { *itr };
        }
        return std::nullopt;
    }

    std::filesystem::path FileTypeImporterInput::getOutputPath(std::string_view defaultExt) const noexcept
    {
        constexpr std::string_view configKey = "outputPath";
        auto relPath = getRelativePath();
        auto basePath = relPath.parent_path();
        auto outputPath = basePath;
        auto name = path.stem().string();
        auto itr = config.find(configKey);
        if (itr != config.end())
        {
            outputPath /= itr->get<std::filesystem::path>();
            return outputPath;
        }
        else
        {
            itr = dirConfig.find(configKey);
            if (itr != dirConfig.end())
            {
                std::string v = *itr;
                if (name.empty())
                {
                    name = relPath.stem().string();
                }
                StringUtils::replace(v, "*", name);
                outputPath /= v;
                return outputPath;
            }
        }
        if (defaultExt.empty())
        {
            // defaultExt == "" means we dont want a default output path
            return {};
        }
        return outputPath / (name + std::string{ defaultExt });
    }

    FileImporterImpl::FileImporterImpl(const fs::path& inputPath)
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
                if(config.load(dirFileConfig))
                {
                    _files.emplace(path, config);
                    }
            }
        }
    }

    const std::string FileImporterImpl::HeaderConfig::_headerVarPrefixKey = "headerVarPrefix";
    const std::string FileImporterImpl::HeaderConfig::_produceHeadersKey = "produceHeaders";
    const std::string FileImporterImpl::HeaderConfig::_headerIncludeDirKey = "headerIncludeDir";

    bool FileImporterImpl::HeaderConfig::load(const nlohmann::json& json) noexcept
    {
        bool found = false;
		auto itr = json.find(_headerVarPrefixKey);
        if (itr != json.end())
        {
            varPrefix = *itr;
            produceHeaders = true;
            found = true;
        }
        itr = json.find(_headerIncludeDirKey);
        if (itr != json.end())
        {
            includeDir = itr->get<std::string>();
            produceHeaders = true;
            found = true;
        }
        itr = json.find(_produceHeadersKey);
        if (itr != json.end())
        {
            produceHeaders = *itr;
            found = true;
        }
        return found;
    }

    const std::string FileImporterImpl::DirConfig::_configFileName = "darmok-import.json";
    const std::string FileImporterImpl::DirConfig::_filesKey = "files";
    const std::string FileImporterImpl::DirConfig::_importersKey = "importers";
    const std::string FileImporterImpl::DirConfig::_includesKey = "includes";
    const std::string FileImporterImpl::DirConfig::_outputPathKey = "outputPath";

    fs::path FileImporterImpl::DirConfig::getPath(const fs::path& path) noexcept
    {
        return path / _configFileName;
    }

    bool FileImporterImpl::DirConfig::isPath(const fs::path& path) noexcept
    {
        return path.filename() == _configFileName;
    }

    void FileImporterImpl::DirConfig::updateOperation(Operation& op, const std::string& importerName) const
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

    void FileImporterImpl::DirConfig::loadFile(const std::string& key, const nlohmann::json& config, const fs::path& basePath, const std::vector<fs::path>& filePaths) noexcept
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

    bool FileImporterImpl::DirConfig::load(const fs::path& inputPath, const std::vector<fs::path>& filePaths)
    {
        path = isPath(inputPath) ? inputPath : getPath(inputPath);
        auto jsonResult = StreamUtils::parseJson(path);
        if(!jsonResult)
        {
            return false;
		}

        HeaderConfig headerConfig;
		auto& json = jsonResult.value();
        if (headerConfig.load(json))
        {
            header = headerConfig;
        }
        auto itr = json.find(_outputPathKey);
        if (itr != json.end())
        {
            outputPath = itr->get<std::string>();
        }

        nlohmann::json includes;
        itr = json.find(_includesKey);
        if (itr != json.end())
        {
            includes = *itr;
        }
        itr = json.find(_filesKey);
        if (itr != json.end())
        {
            nlohmann::json filesJson = *itr;
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
        itr = json.find(_importersKey);
        if (itr != json.end())
        {
            importers = FileConfig::fix(*itr);
            if (!includes.empty())
            {
                FileConfig::replaceIncludes(importers, includes);
            }
        }
        return true;
    }

    const std::string FileImporterImpl::FileConfig::_configFileSuffix = ".darmok-import.json";
    const std::string FileImporterImpl::FileConfig::_importersKey = "importers";
    const std::string FileImporterImpl::FileConfig::_includesKey = "includes";
    const std::string FileImporterImpl::FileConfig::_includePatternToken = "${";
    const std::regex  FileImporterImpl::FileConfig::_includePattern("\\$\\{(.+)\\}");

    fs::path FileImporterImpl::FileConfig::getPath(const fs::path& path) noexcept
    {
        return path.string() + _configFileSuffix;
    }

    bool FileImporterImpl::FileConfig::isPath(const fs::path& path) noexcept
    {
        return path.filename().string().ends_with(_configFileSuffix);
    }

    expected<void, std::string> FileImporterImpl::FileConfig::load(const fs::path& inputPath)
    {
        path = isPath(inputPath) ? inputPath : getPath(inputPath);
        auto jsonResult = StreamUtils::parseJson(path);
        if (!jsonResult)
        {
            return unexpected{ jsonResult.error() };
        }
        return load(*jsonResult);
    }

    expected<void, std::string> FileImporterImpl::FileConfig::load(const nlohmann::json& json)
    {
        auto config = fix(json);
        auto itr = config.find(_importersKey);
        if (itr != config.end())
        {
            importers = fix(*itr);
            itr = config.find(_includesKey);
            if (itr != config.end())
            {
                auto& includes = *itr;
                replaceIncludes(importers, includes);
            }
        }
        else
        {
            importers = config;
        }
        return {};
    }

    nlohmann::json FileImporterImpl::FileConfig::fix(const nlohmann::json& json) noexcept
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

    bool FileImporterImpl::FileConfig::replaceIncludes(nlohmann::json& json, const nlohmann::json& includes) noexcept
    {
        auto replace = [&includes](nlohmann::json& elm)
        {
            auto changed = false;
            if (!elm.is_string())
            {
                return changed;
            }
            std::string str = elm;
            if (!StringUtils::contains(str, _includePatternToken)) // fast check
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

    bool FileImporterImpl::loadInput(const fs::path& path, const std::vector<fs::path>& paths)
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

    std::time_t FileImporterImpl::getUpdateTime(const fs::path& path)
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

    bool FileImporterImpl::addFileCachePath(const fs::path& path, std::time_t cacheTime) const noexcept
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

    void FileImporterImpl::setCachePath(const fs::path& cachePath) noexcept
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
        if (auto jsonResult = StreamUtils::parseJson(_cachePath))
        {
            for (auto& [relPath, cacheTime] : jsonResult->items())
            {
                addFileCachePath(_inputPath / relPath, cacheTime);
            }
        }
    }

    bool FileImporterImpl::isCached(const fs::path& path) const noexcept
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

    fs::path FileImporterImpl::normalizePath(const fs::path& path) noexcept
    {
        std::string pathStr(path.string());
        StringUtils::replace(pathStr, "*", "_");
        StringUtils::replace(pathStr, "?", "_");
        return fs::weakly_canonical(pathStr).make_preferred();
    }

    bool FileImporterImpl::isPathCached(const fs::path& path) const noexcept
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

    bool FileImporterImpl::isCacheUpdated() const noexcept
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

    bool FileImporterImpl::writeCache() const
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

    void FileImporterImpl::setOutputPath(const fs::path& outputPath) noexcept
    {
        _outputPath = outputPath;
    }

    FileImporterImpl::DirConfigs FileImporterImpl::getDirConfigs(const fs::path& path) const noexcept
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

    void FileImporterImpl::mergeConfig(nlohmann::json& json, const nlohmann::json& other)
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

    std::vector<FileImporterImpl::Operation> FileImporterImpl::getOperations() const
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
                    throw std::runtime_error{ ss.str() };
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

    void FileImporterImpl::loadDependencies(const std::vector<Operation>& ops) const
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

    void FileImporterImpl::getDependencies(const fs::path& path, const std::vector<Operation>& ops, Dependencies& deps) const
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

    FileImporterImpl::PathGroups FileImporterImpl::getPathGroups(const std::vector<fs::path>& paths) const noexcept
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

    std::vector<fs::path> FileImporterImpl::getOutputs() const
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

    void FileImporterImpl::addTypeImporter(std::unique_ptr<IFileTypeImporter>&& importer) noexcept
    {
        _importers[importer->getName()] = std::move(importer);
    }

    void FileImporterImpl::produceCombinedHeader(const fs::path& path, const std::vector<fs::path>& paths, const fs::path& includeDir) const
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

    void FileImporterImpl::operator()(std::ostream& log) const
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

    fs::path FileImporterImpl::getHeaderPath(const fs::path& path, const std::string& baseName) const noexcept
    {
        return path.parent_path() / fs::path(baseName + ".h");
    }

    fs::path FileImporterImpl::getHeaderPath(const fs::path& path) const noexcept
    {
        return getHeaderPath(path, path.stem().string());
    }

    std::filesystem::path FileImporterImpl::fixOutput(const std::filesystem::path& path, const Operation& op) const noexcept
    {
        std::filesystem::path output = path;
        if (op.headerConfig.produceHeaders)
        {
            output = getHeaderPath(output);
        }
        return normalizePath(_outputPath / op.outputPath / output);
    }

    std::vector<fs::path> FileImporterImpl::getOutputs(const Operation& op) const
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

    FileImporterImpl::FileImportResult FileImporterImpl::importFile(const Operation& op, std::ostream& log) const
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
                DataOutputStream ds{ data };
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

    FileImporter::FileImporter(const fs::path& inputPath)
        : _impl{ std::make_unique<FileImporterImpl>(inputPath) }
    {
    }

    FileImporter::~FileImporter() noexcept
    {
        // empty on purpose
    }

    FileImporter& FileImporter::setCachePath(const fs::path& cachePath) noexcept
    {
        _impl->setCachePath(cachePath);
        return *this;
    }

    FileImporter& FileImporter::addTypeImporter(std::unique_ptr<IFileTypeImporter>&& importer) noexcept
    {
        _impl->addTypeImporter(std::move(importer));
        return *this;
    }

    FileImporter& FileImporter::setOutputPath(const fs::path& outputPath) noexcept
    {
        _impl->setOutputPath(outputPath);
        return *this;
    }

    std::vector<fs::path> FileImporter::getOutputs() const noexcept
    {
        return _impl->getOutputs();
    }

    void FileImporter::operator()(std::ostream& out) const
    {
        (*_impl)(out);
    }

    CopyFileImporter::CopyFileImporter(size_t bufferSize) noexcept
        : _bufferSize(bufferSize)
    {
    }

    std::vector<fs::path> CopyFileImporter::getOutputs(const Input& input)
    {
        std::vector<fs::path> outputs;
        if (input.config.is_null() && (!input.dirConfig.is_object() || input.dirConfig["all"] != true))
        {
            return outputs;
        }
        auto itr = input.config.find("outputPath");
        if (itr != input.config.end())
        {
            outputs.push_back(*itr);
        }
        else
        {
            outputs.push_back(input.getRelativePath());
        }
        return outputs;
    }

    void CopyFileImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        std::ifstream is(input.path, std::ios::binary);
        StreamUtils::copy(is, out, _bufferSize);
    }

    const std::string& CopyFileImporter::getName() const noexcept
    {
        static const std::string name("copy");
        return name;
    }

    DarmokCoreAssetFileImporter::DarmokCoreAssetFileImporter(const CommandLineFileImporterConfig& config)
        : DarmokCoreAssetFileImporter(config.inputPath)
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

    DarmokCoreAssetFileImporter::DarmokCoreAssetFileImporter(const fs::path& inputPath)
        : _importer(inputPath)
        , _progImporter(_importer.addTypeImporter<ProgramFileImporter>())
    {
        _importer.addTypeImporter<CopyFileImporter>();
        _importer.addTypeImporter<ImageFileImporter>();
        _importer.addTypeImporter<TextureFileImporter>();
    }

    DarmokCoreAssetFileImporter& DarmokCoreAssetFileImporter::setCachePath(const fs::path& cachePath) noexcept
    {
        _importer.setCachePath(cachePath);
        return *this;
    }

    DarmokCoreAssetFileImporter& DarmokCoreAssetFileImporter::setOutputPath(const fs::path& outputPath) noexcept
    {
        _importer.setOutputPath(outputPath);
        return *this;
    }

    DarmokCoreAssetFileImporter& DarmokCoreAssetFileImporter::setShadercPath(const fs::path& path) noexcept
    {
        _progImporter.setShadercPath(path);
        return *this;
    }

    DarmokCoreAssetFileImporter& DarmokCoreAssetFileImporter::addShaderIncludePath(const fs::path& path) noexcept
    {
        _progImporter.addIncludePath(path);
        return *this;
    }

    std::vector<fs::path> DarmokCoreAssetFileImporter::getOutputs() const
    {
        return _importer.getOutputs();
    }

    void DarmokCoreAssetFileImporter::operator()(std::ostream& out) const
    {
        _importer(out);
    }

    BaseCommandLineFileImporter::BaseCommandLineFileImporter() noexcept
        : _impl(std::make_unique<CommandLineFileImporterImpl>(*this))
    {
    }

    BaseCommandLineFileImporter::~BaseCommandLineFileImporter() noexcept
    {
        // empty on purpose
    }

    int BaseCommandLineFileImporter::operator()(const CmdArgs& args) noexcept
    {
        return (*_impl)(args);
    }

    void BaseCommandLineFileImporter::setup(CLI::App& cli, Config& cfg, bool required) noexcept
    {
        CommandLineFileImporterImpl::setup(cli, cfg, required);
    }

    CommandLineFileImporterImpl::CommandLineFileImporterImpl(BaseCommandLineFileImporter& importer) noexcept
      : _importer(importer)
    {
    }

    void CommandLineFileImporterImpl::setup(CLI::App& cli, Config& cfg, bool required) noexcept
    {
        cli.set_version_flag("-v,--version", "VERSION " DARMOK_VERSION);
        auto inputOpt = cli.add_option("-i,--import-input", cfg.inputPath, "Input file path (can be a file or a directory).")
            ->envname("DARMOK_IMPORT_INPUT")
            ->option_text("PATH");
        if (required)
        {
            inputOpt->required(true);
        }
        cli.add_option("-o,--import-output", cfg.outputPath, "Output file path (can be a file or a directory).")
            ->option_text("PATH")
            ->envname("DARMOK_IMPORT_OUTPUT");
        cli.add_option("-c,--import-cache", cfg.cachePath, "Cache file path (directory that keeps the timestamps of the inputs).")
            ->expected(0, 1)
            ->option_text("PATH")
            ->envname("DARMOK_IMPORT_CACHE");
        cli.add_flag("-d, --import-dry", cfg.dry, "Do not process assets, just print output files.")
            ->envname("DARMOK_IMPORT_DRY");

        auto progGroup = cli.add_option_group("Program Compiler");
        progGroup->add_option("--bgfx-shaderc", cfg.shadercPath, "path to the shaderc executable")
            ->option_text("PATH")
            ->envname("DARMOK_IMPORT_BGFX_SHADERC");
        progGroup->add_option("--bgfx-shader-include", cfg.shaderIncludePaths, "paths to shader files to be included")
            ->option_text("PATH ...")
            ->envname("DARMOK_IMPORT_BGFX_SHADER_INCLUDE");
    }

    const std::string CommandLineFileImporterConfig::defaultInputPath = "assets";
    const std::string CommandLineFileImporterConfig::defaultOutputPath = "runtime_assets";
    const std::string CommandLineFileImporterConfig::defaultCachePath = "asset_cache";

    void CommandLineFileImporterConfig::fix(const CLI::App& cli) noexcept
    {
        auto cacheOpt = cli.get_option("--import-cache");
        if (!cacheOpt->results().empty() && cachePath.empty())
        {
            // passing --aset-cache without parameter sets default asset cache path
            cachePath = defaultCachePath;
        }
        if (inputPath.empty() && std::filesystem::exists(defaultInputPath))
        {
            inputPath = defaultInputPath;
        }
        if (outputPath.empty())
        {
            outputPath = defaultOutputPath;
        }
        if (cachePath.empty() && std::filesystem::exists(defaultCachePath))
        {
            cachePath = defaultCachePath;
        }
    }

    int CommandLineFileImporterImpl::operator()(const CmdArgs& args) noexcept
    {
        CLI::App cli{ "darmok asset compile tool" };
        Config cfg;

        setup(cli, cfg, true);

        try
        {
            cli.parse(static_cast<int>(args.size()), args.data());
            cfg.fix(cli);

            if (cfg.dry)
            {
                for (auto& output : _importer.getOutputs(cfg))
                {
                    std::cout << output.string() << std::endl;
                }
                return 0;
            }
            
            PrefixStream log(std::cout, cli.get_name() + ": ");
            _importer.import(cfg, log);
            return 0;
        }
        catch (const CLI::ParseError& ex)
        {
            return cli.exit(ex);                                                                                          \
        }
        catch(const std::exception& ex)
        {
            std::cerr << "error: " << ex.what() << std::endl;
            return 1;
        }
    }
}
