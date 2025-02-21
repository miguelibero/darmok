#include <darmok/program_core.hpp>
#include "program_core.hpp"
#include <darmok/data_stream.hpp>
#include <darmok/string.hpp>
#include <darmok/utils.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

namespace darmok
{
    namespace fs = std::filesystem;

    const ProgramDefinition::RendererProfileMap& ProgramDefinition::getRendererProfiles() noexcept
    {
        static const RendererProfileMap profiles
        {
            { bgfx::RendererType::Direct3D11,   { "s_5_0" }},
            { bgfx::RendererType::Direct3D12,   { "s_5_0" }},
            { bgfx::RendererType::Metal,        { "metal" }},
            { bgfx::RendererType::OpenGLES,     { "300_es" }},
            { bgfx::RendererType::OpenGL,       { "150", "130" }},
            { bgfx::RendererType::Vulkan,       { "spirv16-13", "spirv14-11", "spirv" }},
        };

        return profiles;
    }

    const ProgramDefinition::Profile& ProgramDefinition::getCurrentProfile() const
    {
        auto render = bgfx::getRendererType();
        auto& rendererProfiles = getRendererProfiles();
        auto itr = rendererProfiles.find(render);
        if (itr == rendererProfiles.end())
        {
            throw std::invalid_argument("could not find renderer profiles");
        }
        for (auto& profile : itr->second)
        {
            auto itr = profiles.find(profile);
            if (itr == profiles.end())
            {
                continue;
            }
            return itr->second;
        }
        throw std::invalid_argument("no valid profile found");
    }

    bool ProgramDefinition::empty() const noexcept
    {
        return vertexLayout.empty();
    }

    void ProgramDefinition::loadBinary(DataView data)
    {
        DataInputStream stream(data);
        cereal::PortableBinaryInputArchive archive(stream);
        archive(*this);
    }

    ShaderParser::ShaderParser(const IncludePaths& includePaths) noexcept
        : _includePaths(includePaths)
    {
    }

    ShaderType ShaderParser::getType(const std::string& name) noexcept
    {
        auto lowerName = StringUtils::toLower(name);
        if (lowerName == "vertex")
        {
            return ShaderType::Vertex;
        }
        if (lowerName == "fragment")
        {
            return ShaderType::Fragment;
        }
        if (lowerName == "compute")
        {
            return ShaderType::Compute;
        }
        return ShaderType::Unknown;
    }

    std::string ShaderParser::getTypeName(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::Vertex:
            return "vertex";
        case ShaderType::Fragment:
            return "fragment";
        case ShaderType::Compute:
            return "compute";
        default:
            throw std::invalid_argument("unknown shader type");
        }
    }

    const std::unordered_map<bgfx::RendererType::Enum, std::string> ShaderParser::_rendererExtensions{

        { bgfx::RendererType::Direct3D11,   ".dx11" },
        { bgfx::RendererType::Direct3D12,   ".dx12" },
        { bgfx::RendererType::Metal,        ".mtl" },
        { bgfx::RendererType::OpenGLES,     ".essl" },
        { bgfx::RendererType::OpenGL,       ".glsl" },
        { bgfx::RendererType::Vulkan,       ".spv" },
    };


    const std::regex ShaderParser::_includeRegex = std::regex("#include <([^>]+)>");

    std::optional<std::filesystem::path> ShaderParser::readDependency(const std::string& line) const noexcept
    {
        std::smatch match;
        if (!std::regex_search(line, match, _includeRegex))
        {
            return std::nullopt;
        }
        auto name = match[1].str();
        for (auto& include : _includePaths)
        {
            auto path = include / name;
            if (fs::exists(path))
            {
                return path;
            }
        }
        return std::nullopt;
    }

    size_t ShaderParser::getDependencies(std::istream& in, Dependencies& deps) const noexcept
    {
        std::unordered_set<std::filesystem::path> checkedPaths;
        return getDependencies(in, deps, checkedPaths);
    }

    size_t ShaderParser::getDependencies(std::istream& in, Dependencies& deps, std::unordered_set<std::filesystem::path>& checkedPaths) const noexcept
    {
        std::string line;
        size_t count = 0;
        while (std::getline(in, line))
        {
            if (auto path = readDependency(line))
            {
                if (deps.insert(path.value()).second)
                {
                    count++;
                }
            }
        }
        for (auto& path : deps)
        {
            if (checkedPaths.contains(path))
            {
                continue;
            }
            checkedPaths.insert(path);
            std::ifstream in(path);
            count += getDependencies(in, deps, checkedPaths);
        }
        return count;
    }

    const std::regex ShaderParser::_ifdefRegex = std::regex("#(if|ifdef|ifndef) !?([^\\s]+)");
    const std::string ShaderParser::_definePrefix = "DARMOK_VARIANT_";

    std::optional<std::string> ShaderParser::readDefine(const std::string& line) noexcept
    {
        std::smatch match;
        if (!std::regex_search(line, match, _ifdefRegex))
        {
            return std::nullopt;
        }
        auto define = match[2].str();
        if (!define.starts_with(_definePrefix))
        {
            return std::nullopt;
        }
        return define.substr(_definePrefix.size());
    }

    size_t ShaderParser::getDefines(std::istream& in, Defines& defines) const noexcept
    {
        std::unordered_set<std::filesystem::path> checkedPaths;
        return getDefines(in, defines, checkedPaths);
    }

    size_t ShaderParser::getDefines(std::istream& in, Defines& defines, std::unordered_set<std::filesystem::path>& checkedPaths) const noexcept
    {
        std::string line;
        size_t count = 0;

        Dependencies deps;
        while (std::getline(in, line))
        {
            if (auto define = readDefine(line))
            {
                if (defines.insert(define.value()).second)
                {
                    count++;
                }
            }
            else if (auto path = readDependency(line))
            {
                deps.insert(path.value());
            }
        }
        for (auto& path : deps)
        {
            if (checkedPaths.contains(path))
            {
                continue;
            }
            checkedPaths.insert(path);
            std::ifstream in(path);
            count += getDefines(in, defines, checkedPaths);
        }
        return count;
    }

    std::string ShaderParser::getDefinesArgument(const Defines& defines) noexcept
    {
        std::vector<std::string> realDefines;
        realDefines.reserve(defines.size());
        for (auto& define : defines)
        {
            realDefines.push_back(_definePrefix + define);
        }
        return StringUtils::join(",", realDefines);
    }

    const std::string ShaderParser::_binExt = ".bin";
    const std::string ShaderParser::_enableDefineSuffix = "_enabled";

    ShaderType ShaderParser::getType(const fs::path& path) noexcept
    {
        auto exts = StringUtils::split(StringUtils::getFileExt(path.filename().string()), ".");
        for (auto& ext : exts)
        {
            auto type = getType(ext);
            if (type != ShaderType::Unknown)
            {
                return type;
            }
        }
        return ShaderType::Unknown;
    }

    const std::vector<bgfx::RendererType::Enum>& ShaderParser::getSupportedRenderers() noexcept
    {
        static const std::vector<bgfx::RendererType::Enum> renderers
        {
#if BX_PLATFORM_WINDOWS
            bgfx::RendererType::Direct3D11, bgfx::RendererType::Direct3D12, bgfx::RendererType::Vulkan,
#elif BX_PLATFORM_OSX
            bgfx::RendererType::Metal,
#elif BX_PLATFORM_LINUX
            bgfx::RendererType::Vulkan,
#endif
            bgfx::RendererType::OpenGL, bgfx::RendererType::OpenGLES,
        };
        return renderers;
    }

    std::vector<ShaderParser::CompilerOperation> ShaderParser::prepareCompilerOperations(const CompilerConfig& config, const DataView& shader, const std::filesystem::path& baseOutputPath) const noexcept
    {
        shader.write(config.path);
        ShaderDefines defines;
        DataInputStream in(shader);
        getDefines(in, defines);
        return getCompilerOperations(config, defines, baseOutputPath);
    }

    std::vector<ShaderParser::CompilerOperation> ShaderParser::getCompilerOperations(const CompilerConfig& config, const Defines& defines, const std::filesystem::path& baseOutputPath) noexcept
    {
        auto defineCombs = CollectionUtils::combinations(defines);
        std::vector<CompilerOperation> ops;

        auto& rendererProfiles = ProgramDefinition::getRendererProfiles();
        auto& renderers = getSupportedRenderers();

        auto baseOutputStr = baseOutputPath.string();
        if(baseOutputStr.empty())
        {
            baseOutputStr = config.path.string() + ".";
        }

        for (auto& renderer : renderers)
        {
            for (auto& profile : rendererProfiles.at(renderer))
            {
                std::string profileExt = profile;
                auto itr = _rendererExtensions.find(renderer);
                if (itr != _rendererExtensions.end())
                {
                    profileExt = itr->second;
                }
                for (auto& defines : defineCombs)
                {
                    CompilerOperation op{
                        .profile = profile,
                        .defines = defines,
                    };
                    op.outputPath = baseOutputStr + getDefaultOutputFile(config, op).string();
                    ops.push_back(op);
                }
            }
        }
        return ops;
    }

    std::filesystem::path ShaderParser::getDefaultOutputFile(const CompilerConfig& config, const CompilerOperation& op) noexcept
    {
        auto suffix = StringUtils::join("-", op.defines.begin(), op.defines.end(), [](std::string define) {
            define = StringUtils::toLower(define);
            if (StringUtils::endsWith(define, _enableDefineSuffix))
            {
                define = define.substr(0, define.size() - _enableDefineSuffix.size());
            }
            return define;
            });        
        if (!suffix.empty())
        {
            suffix = "-" + suffix;
        }
        return suffix + op.profile + _binExt;
    }

    ShaderCompiler::ShaderCompiler(const Config& config) noexcept
        : _config(config)
    {
    }

    std::optional<ShaderCompiler::PlatformType> ShaderCompiler::getDefaultPlatform() noexcept
    {
#if BX_PLATFORM_WINDOWS
        return PlatformType::Windows;
#elif BX_PLATFORM_OSX
        return PlatformType::Osx;
#else
        return std::nullopt;
#endif
    }

    void ShaderCompiler::operator()(const Operation& op) const
    {
        if (!fs::exists(_config.programConfig.shadercPath))
        {
            throw std::runtime_error("could not find shaderc.exe");
        }
        fs::path varyingPath = _config.varyingPath;
        auto inputDir = _config.path.parent_path();
        if (varyingPath.empty())
        {
            varyingPath = inputDir / (StringUtils::getFileStem(_config.path.string()) + ".varyingdef");
        }
        auto shaderType = _config.type;
        if (shaderType == ShaderType::Unknown)
        {
            shaderType = ShaderParser::getType(_config.path);
        }
        auto outputPath = op.outputPath;
        if (outputPath.empty())
        {
            outputPath = _config.path.parent_path() / ShaderParser::getDefaultOutputFile(_config, op);
        }
        std::vector<Exec::Arg> args{
            _config.programConfig.shadercPath,
            "-p", op.profile,
            "-f", _config.path,
            "-o", outputPath,
            "--type", ShaderParser::getTypeName(shaderType),
            "--varyingdef", varyingPath
        };

        auto getPlatformArg = [](PlatformType plat)
        {
            switch(plat)
            {
            case PlatformType::Android:
                return "android";
            case PlatformType::Emscripten:
                return "asm.js";
            case PlatformType::Ios:
                return "ios";
            case PlatformType::Linux:
                return "linux";
            case PlatformType::Osx:
                return "osx";
            case PlatformType::Orbis:
                return "orbis";
            case PlatformType::Windows:
                return "windows";
            default:
                break;
            }
            return "";
        };

        auto plat = _config.platform;
        if(!plat)
        {
            plat = getDefaultPlatform();
        }
        if(plat)
        {
            args.push_back("--platform");
            args.push_back(getPlatformArg(plat.value()));
        }

#ifdef _DEBUG
        args.push_back("--debug");
#endif
        if (!op.defines.empty())
        {
            args.emplace_back("--define");
            args.emplace_back(ShaderParser::getDefinesArgument(op.defines));
        }

        if (!inputDir.empty())
        {
            args.push_back("-i");
            args.push_back(inputDir);
        }

        for (auto& include : _config.programConfig.includePaths)
        {
            args.push_back("-i");
            args.push_back(include);
        }

        auto r = Exec::run(args);
        if (r.returnCode != 0 || StringUtils::contains(r.out, "Failed to build shader."))
        {
            if (auto log = _config.programConfig.log)
            {
                *log << "shaderc cmd:" << std::endl;
                *log << Exec::argsToString(args) << std::endl;
                *log << "shaderc output:" << std::endl;
                *log << r.out;
                *log << "shaderc error output:" << std::endl;
                *log << r.err;
            }
            throw std::runtime_error("failed to build shader");
        }
    }

    void ProgramSource::read(const nlohmann::ordered_json& json, fs::path path)
    {
        // maybe switch to arrays to avoid ordered_json
        auto basePath = path.parent_path();
        auto readJson = [this, &basePath](const nlohmann::ordered_json& json)
            {
                if (json.contains("vertexShader"))
                {
                    auto vertPath = basePath / json["vertexShader"].get<std::string>();
                    vertexShader = Data::fromFile(vertPath);
                }
                if (json.contains("fragmentShader"))
                {
                    auto fragPath = basePath / json["fragmentShader"].get<std::string>();
                    fragmentShader = Data::fromFile(fragPath);
                }
                if (json.contains("varyingDef"))
                {
                    auto def = json["varyingDef"];
                    if (def.is_string())
                    {
                        fs::path varyingPath = basePath / json["varyingDef"].get<std::string>();
                        varying.read(varyingPath);
                    }
                    else
                    {
                        varying.read(def);
                    }
                }
                else
                {
                    varying.read(json);
                }
            };

        if (fs::is_regular_file(path) && path.extension() == ".json")
        {
            auto pathJson = nlohmann::ordered_json::parse(std::ifstream(path));
            readJson(pathJson);
        }

        readJson(json);
        
        if (!path.empty())
        {
            auto fileName = path.filename();
            auto stem = StringUtils::getFileStem(fileName.string());

            auto basePath = path.parent_path();
            if (vertexShader.empty())
            {
                auto vertPath = basePath / (stem + ".vertex.sc");
                vertexShader = Data::fromFile(vertPath);
            }
            if (fragmentShader.empty())
            {
                auto fragPath = basePath / (stem + ".fragment.sc");
                fragmentShader = Data::fromFile(fragPath);
            }
            if (varying.empty())
            {
                auto varyingPath = basePath / (stem + ".varyingdef");
                varying.read(varyingPath);
            }
        }
    }

    void ProgramSource::read(const std::filesystem::path& path)
    {
        auto ext = path.extension();
        std::ifstream is(path);
        if (ext == ".json")
        {
            auto fileJson = nlohmann::ordered_json::parse(is);
            read(fileJson, path);
        }
        else if (ext == ".xml")
        {
            cereal::XMLInputArchive archive(is);
            archive(*this);
        }
        else
        {
            cereal::PortableBinaryInputArchive archive(is);
            archive(*this);
        }
    }

    void ProgramCompilerConfig::read(const nlohmann::json& json, std::filesystem::path basePath)
    {
        if (json.contains("includeDirs"))
        {
            for (fs::path path : json["includeDirs"])
            {
                includePaths.insert(basePath / path);
            }
        }
        includePaths.insert(basePath);

        if (json.contains("shadercPath"))
        {
            shadercPath = basePath / json["shadercPath"];
        }
    }
    
    ProgramCompiler::ProgramCompiler(const Config& config) noexcept
        : _config(config)
    {
    }

    ProgramDefinition ProgramCompiler::operator()(const ProgramSource& src)
    {
        ProgramDefinition def
        {
            .vertexLayout = src.varying.vertex,
        };
        auto varyingDefPath = getTempPath("darmok.varyingdef.");
        src.varying.writeBgfx(varyingDefPath);

        ShaderCompilerConfig shaderConfig
        {
            .programConfig = _config,
            .varyingPath = varyingDefPath,
        };
        ShaderParser shaderParser(_config.includePaths);

        {
            shaderConfig.type = ShaderType::Fragment;
            shaderConfig.path = getTempPath("darmok.fragment.");
            auto ops = shaderParser.prepareCompilerOperations(shaderConfig, src.fragmentShader);
            ShaderCompiler shaderCompiler(shaderConfig);
            for (auto& op : ops)
            {
                shaderCompiler(op);
                auto data = Data::fromFile(op.outputPath);
                def.profiles[op.profile].fragmentShaders[op.defines] = std::move(data);
                fs::remove(op.outputPath);
            }
            fs::remove(shaderConfig.path);
        }

        {
            shaderConfig.type = ShaderType::Vertex;
            shaderConfig.path = getTempPath("darmok.vertex.");
            auto ops = shaderParser.prepareCompilerOperations(shaderConfig, src.vertexShader);
            ShaderCompiler shaderCompiler(shaderConfig);
            for (auto& op : ops)
            {
                shaderCompiler(op);
                auto data = Data::fromFile(op.outputPath);
                def.profiles[op.profile].vertexShaders[op.defines] = std::move(data);
                fs::remove(op.outputPath);
            }
            fs::remove(shaderConfig.path);
        }

        // TODO: remove files in case of exception?
        fs::remove(varyingDefPath);
        
        return def;
    }

    ProgramFileImporterImpl::ProgramFileImporterImpl(size_t defaultBufferSize) noexcept
        : _defaultConfig{ defaultBufferSize }
    {
    }

    void ProgramFileImporterImpl::setShadercPath(const std::filesystem::path& path) noexcept
    {
        _defaultConfig.shadercPath = path;
    }

    void ProgramFileImporterImpl::addIncludePath(const std::filesystem::path& path) noexcept
    {
        _defaultConfig.includePaths.insert(path);
    }

    ProgramSource ProgramFileImporterImpl::readSource(const Input& input)
    {
        ProgramSource src;
        if (input.config.is_object())
        {
            src.read(input.config, input.path);
        }
        if (fs::exists(input.path))
        {
            src.read(input.path);
        }
        
        if (src.vertexShader.empty())
        {
            throw std::runtime_error("missing vertex shader");
        }
        if (src.fragmentShader.empty())
        {
            throw std::runtime_error("missing fragment shader");
        }
        if (src.varying.empty())
        {
            throw std::runtime_error("missing varying definition");
        }
        return src;
    }

    void ProgramFileImporterImpl::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _log = log;
    }

    bool ProgramFileImporterImpl::startImport(const Input& input, bool dry)
    {
        if (input.config.is_null())
        {
            auto ext = StringUtils::getFileExt(input.path.filename().string());
            if (ext != ".program.json")
            {
                return false;
            }
        }

        _config = _defaultConfig;
        _config->read(input.dirConfig, input.basePath);
        _config->read(input.config, input.basePath);

        _src.emplace();
        _src->read(input.config, input.path);

        _outputPath = input.getOutputPath(".bin");
        return true;
    }

    std::vector<fs::path> ProgramFileImporterImpl::getOutputs(const Input& input)
    {
        return { _outputPath };
    }

    ProgramFileImporterImpl::Dependencies ProgramFileImporterImpl::getDependencies(const Input& input)
    {
        Dependencies deps;
        if (!_src)
        {
            return deps;
        }
        ShaderParser parser(_config ? _config->includePaths : ShaderParser::IncludePaths{});
        {
            DataInputStream in(_src->vertexShader);
            parser.getDependencies(in, deps);
        }
        {
            DataInputStream in(_src->fragmentShader);
            parser.getDependencies(in, deps);
        }

        return deps;
    }

    void ProgramFileImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        if (!_src || !_config)
        {
            return;
        }

        auto& config = _config.value();
        config.log = _log;
        ProgramCompiler compiler(config);
        auto def = compiler(_src.value());
        writeDefinition(def, out);
    }

    void ProgramFileImporterImpl::writeDefinition(const ProgramDefinition& def, std::ostream& out)
    {
        CerealUtils::save(def, out, CerealUtils::getExtensionFormat(_outputPath));
    }

    void ProgramFileImporterImpl::endImport(const Input& input)
    {
        _config.reset();
        _outputPath.clear();
    }

    const std::string& ProgramFileImporterImpl::getName() const noexcept
    {
        static const std::string name("program");
        return name;
    }

    ProgramFileImporter::ProgramFileImporter()
        : _impl(std::make_unique<ProgramFileImporterImpl>())
    {
    }

    ProgramFileImporter::~ProgramFileImporter() noexcept
    {
        // empty on purpose
    }

    ProgramFileImporter& ProgramFileImporter::setShadercPath(const fs::path& path) noexcept
    {
        _impl->setShadercPath(path);
        return *this;
    }

    ProgramFileImporter& ProgramFileImporter::addIncludePath(const fs::path& path) noexcept
    {
        _impl->addIncludePath(path);
        return *this;
    }

    void ProgramFileImporter::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _impl->setLogOutput(log);
    }

    bool ProgramFileImporter::startImport(const Input& input, bool dry)
    {
        return _impl->startImport(input, dry);
    }

    ProgramFileImporter::Outputs ProgramFileImporter::getOutputs(const Input& input)
    {
        return _impl->getOutputs(input);
    }

    ProgramFileImporter::Dependencies ProgramFileImporter::getDependencies(const Input& input)
    {
        return _impl->getDependencies(input);
    }

    void ProgramFileImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        _impl->writeOutput(input, outputIndex, out);
    }

    const std::string& ProgramFileImporter::getName() const noexcept
    {
        return _impl->getName();
    }

    void ProgramFileImporter::endImport(const Input& input)
    {
        return _impl->endImport(input);
    }
}