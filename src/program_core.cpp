#include <darmok/program_core.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/string.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/stream.hpp>
#include "detail/program_core.hpp"

#include <fmt/format.h>
#include <magic_enum/magic_enum_format.hpp>

namespace darmok
{
    namespace fs = std::filesystem;

    ConstProgramDefinitionWrapper::ConstProgramDefinitionWrapper(const Definition& def)
        : _def{ def }
    {
    }

    expected<std::reference_wrapper<const ConstProgramDefinitionWrapper::Profile>, std::string> ConstProgramDefinitionWrapper::getCurrentProfile()
    {
        auto render = bgfx::getRendererType();
        auto& rendererProfiles = getRendererProfiles();
        auto itr = rendererProfiles.find(render);
        if (itr == rendererProfiles.end())
        {
            return unexpected{ std::string{"could not find renderer profiles"} };
        }
        auto& profiles = _def.profiles();
        for (auto& profileName : itr->second)
        {
            auto itr = profiles.find(profileName);
            if (itr == profiles.end())
            {
                continue;
            }
            return itr->second;
        }
        return unexpected{ std::string{"no valid profile found"} };
    }

    const ConstProgramDefinitionWrapper::RendererProfileMap& ConstProgramDefinitionWrapper::getRendererProfiles() noexcept
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

    ProgramSourceWrapper::ProgramSourceWrapper(Source& src)
        : _src{ src }
    {
    }

    expected<void, std::string> ProgramSourceWrapper::read(const std::filesystem::path& path)
    {
        auto jsonResult = StreamUtils::parseJson(path);
        if(!jsonResult)
        {
            return unexpected{ jsonResult.error() };
		}
        auto basePath = path.parent_path();
        return read(*jsonResult, basePath);
    }

    expected<void, std::string> ProgramSourceWrapper::read(const nlohmann::ordered_json& json, const std::filesystem::path& basePath)
    {       
        auto itr = json.find("name");
        if (itr != json.end())
        {
            _src.set_name(itr->get<std::string>());
        }
        itr = json.find("vertexShader");
        if (itr != json.end())
        {
            auto vertPath = basePath / itr->get<std::string>();
            auto readResult = StreamUtils::readString(vertPath);
            if (!readResult)
            {
				return unexpected{ readResult.error() };
            }
            *_src.mutable_vertex_shader() = std::move(readResult).value();
        }
        itr = json.find("fragmentShader");
        if (itr != json.end())
        {
            auto fragPath = basePath / itr->get<std::string>();
            auto readResult = StreamUtils::readString(fragPath);
            if (!readResult)
            {
                return unexpected{ readResult.error() };
            }
            *_src.mutable_fragment_shader() = std::move(readResult).value();
        }
        expected<void, std::string> result;
        itr = json.find("varyingDef");
        VaryingDefinitionWrapper varying{ *_src.mutable_varying() };
        if (itr != json.end())
        {
            if (itr->is_string())
            {
                fs::path varyingPath = basePath / itr->get<std::string>();
                result = std::move(varying.read(varyingPath));
            }
            else
            {
                result = std::move(varying.read(*itr));
            }
        }
        else
        {
            result = std::move(varying.read(json));
        }
        return result;
    }

    ShaderParser::ShaderParser(const IncludePaths& includePaths) noexcept
        : _includePaths{ includePaths }
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
            std::ifstream in{ path };
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
        auto exts = StringUtils::split(path.extension().string(), ".");
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
        if (config.path.empty())
        {
            return {};
        }
        shader.write(config.path);
        ShaderDefines defines;
        DataInputStream in{ shader };
        getDefines(in, defines);
        return getCompilerOperations(config, defines, baseOutputPath);
    }

    std::vector<ShaderParser::CompilerOperation> ShaderParser::getCompilerOperations(const CompilerConfig& config, const Defines& defines, const std::filesystem::path& baseOutputPath) noexcept
    {
        auto defineCombs = CollectionUtils::combinations(defines);
        std::vector<CompilerOperation> ops;

        auto& rendererProfiles = ProgramDefinitionWrapper::getRendererProfiles();
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

    protobuf::Shader& ShaderParser::getShader(protobuf::Program& def, ShaderType shaderType, const CompilerOperation& op)
    {
        auto& profiles = *def.mutable_profiles();
        auto itr = profiles.find(op.profile);
        if (itr == profiles.end())
        {
            itr = profiles.emplace(op.profile, protobuf::ProgramProfile{}).first;
        }
        auto& profile = itr->second;

        google::protobuf::RepeatedPtrField<protobuf::Shader>* shaders = nullptr;

        switch (shaderType)
        {
        case ShaderType::Fragment:
            shaders = profile.mutable_fragment_shaders();
            break;
		case ShaderType::Vertex:
            shaders = profile.mutable_vertex_shaders();
            break;
        default:
			throw std::runtime_error{ "unsupported shader type" };
        }

        auto itr2 = std::find_if(shaders->begin(), shaders->end(), [&op](auto& shader)
            {
                return shader.defines() == op.defines;
            });
        if (itr2 != shaders->end())
        {
            return *itr2;
        }
        auto& shader = *shaders->Add();
        for (auto& define : op.defines)
        {
            *shader.add_defines() = define;
        }

        return shader;
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
        : _config{ config }
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

    expected<void, std::string> ShaderCompiler::operator()(const Operation& op) const
    {
        if (!fs::exists(_config.programConfig.shadercPath))
        {
            return unexpected{ "could not find shaderc.exe" };
        }
        fs::path varyingPath = _config.varyingPath;
        auto inputDir = _config.path.parent_path();
        if (varyingPath.empty())
        {
            varyingPath = inputDir / (_config.path.stem().string() + ".varyingdef");
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
            return unexpected{ "failed to build shader" };
        }
        return {};
    }

    expected<void, std::string> ProgramFileImporterImpl::readSource(protobuf::ProgramSource& src, const nlohmann::ordered_json& json, const fs::path& path)
    {
        // maybe switch to arrays to avoid ordered_json
        auto basePath = path.parent_path();
        ProgramSourceWrapper srcWrapper{ src };
        if (fs::is_regular_file(path) && path.extension() == ".json")
        {
            auto pathJsonResult = StreamUtils::parseOrderedJson(path);
            if(!pathJsonResult)
            {
                return unexpected{ pathJsonResult.error() };
			}
            auto result = srcWrapper.read(*pathJsonResult, basePath);
            if (!result)
            {
                return result;
            }
        }

        auto result = srcWrapper.read(json, basePath);
        if (!result)
        {
            return result;
        }

        auto fileName = path.filename();
        auto stem = fileName.stem().string();
        stem = stem.substr(0, stem.find_first_of('.'));

        if (src.name().empty())
        {
            src.set_name(stem);
        }
        if (src.vertex_shader().empty())
        {
            auto vertPath = basePath / (stem + ".vertex.sc");
			auto readResult = StreamUtils::readString(vertPath);
            if (!readResult)
            {
				return unexpected{ readResult.error() };
            }
            *src.mutable_vertex_shader() = std::move(readResult).value();
        }
        if (src.fragment_shader().empty())
        {
            auto fragPath = basePath / (stem + ".fragment.sc");
            auto readResult = StreamUtils::readString(fragPath);
            if (!readResult)
            {
                return unexpected{ readResult.error() };
            }
            *src.mutable_fragment_shader() = std::move(readResult).value();
        }
        if (!src.has_varying())
        {
            auto varyingPath = basePath / (stem + ".varyingdef");
            VaryingDefinitionWrapper varying{ *src.mutable_varying() };
            auto result = varying.read(varyingPath);
            if (!result)
            {
                return result;
            }
        }

        return {};
    }

    void ProgramCompilerConfig::read(const nlohmann::json& json, std::filesystem::path basePath)
    {
        auto itr = json.find("includeDirs");
        if (itr != json.end())
        {
            for (fs::path path : *itr)
            {
                includePaths.insert(basePath / path);
            }
        }
        includePaths.insert(basePath);

        itr = json.find("shadercPath");
        if (itr != json.end())
        {
            shadercPath = basePath / *itr;
        }
    }
    
    ProgramCompiler::ProgramCompiler(const Config& config) noexcept
        : _config{ config }
    {
    }

    expected<protobuf::Program, std::string> ProgramCompiler::operator()(const protobuf::ProgramSource& src)
    {
        protobuf::Program def;
        def.set_name(src.name());
        *def.mutable_varying() = src.varying();
		std::string prefix = "darmok." + src.name() + ".";
        auto varyingDefPath = getTempPath(prefix + "varyingdef.");
        ConstVaryingDefinitionWrapper varying{ src.varying() };
        varying.writeBgfx(varyingDefPath);

        ShaderCompilerConfig shaderConfig
        {
            .programConfig = _config,
            .varyingPath = varyingDefPath,
        };
        ShaderParser shaderParser{ _config.includePaths };

		auto compileShaders = [&](DataView srcData) -> std::optional<std::string>
        {
            auto ops = shaderParser.prepareCompilerOperations(shaderConfig, srcData);
            ShaderCompiler shaderCompiler{ shaderConfig };
            for (auto& op : ops)
            {
                auto compileResult = shaderCompiler(op);
                if (!compileResult)
                {
                    return fmt::format("compiling {} profile {}: {}", shaderConfig.type, op.profile, compileResult.error());
                }
                auto& shader = ShaderParser::getShader(def, shaderConfig.type, op);
                auto readResult = StreamUtils::readString(op.outputPath);
				if (!readResult)
				{
					return fmt::format("reading file {}: {}", op.outputPath.string(), readResult.error());
				}
                *shader.mutable_data() = std::move(readResult).value();
                fs::remove(op.outputPath);
            }
            fs::remove(shaderConfig.path);
            return std::nullopt;
        };

        shaderConfig.type = ShaderType::Fragment;
        shaderConfig.path = getTempPath(prefix + "fragment.");
        auto error = compileShaders(DataView{ src.fragment_shader() });
        if (error)
        {
			return unexpected{ *error };
        }

        shaderConfig.type = ShaderType::Vertex;
        shaderConfig.path = getTempPath(prefix + "vertex.");
        error = compileShaders(DataView{ src.vertex_shader() });
        if (error)
        {
            return unexpected{ *error };
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

    void ProgramFileImporterImpl::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _log = log;
    }

    bool ProgramFileImporterImpl::startImport(const Input& input, bool dry)
    {
        if (input.config.is_null())
        {
            if (!input.path.string().ends_with(".program.json"))
            {
                return false;
            }
        }

        _config = _defaultConfig;
        _config->read(input.dirConfig, input.basePath);
        _config->read(input.config, input.basePath);

        _src.emplace();
        auto result = readSource(*_src, input.config, input.path);
        if (!result)
        {
            return false;
        }

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
        ShaderParser parser{ _config ? _config->includePaths : ShaderParser::IncludePaths{} };
        {
            std::istringstream in{ _src->vertex_shader() };
            parser.getDependencies(in, deps);
        }
        {
            std::istringstream in{ _src->fragment_shader() };
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
        ProgramCompiler compiler{ config };
        auto compileResult = compiler(_src.value());
        if (!compileResult)
        {
			throw std::runtime_error(fmt::format("failed to compile program: {}", compileResult.error()));
        }
		auto& def = compileResult.value();
        auto format = protobuf::getFormat(input.getOutputPath());
        auto writeResult = protobuf::write(def, out, format);
        if (!writeResult)
        {
			throw std::runtime_error("failed to write program file");
        }
    }

    void ProgramFileImporterImpl::endImport(const Input& input)
    {
        _config.reset();
        _outputPath.clear();
    }

    const std::string& ProgramFileImporterImpl::getName() const noexcept
    {
        static const std::string name{ "program" };
        return name;
    }

    ProgramDefinitionFromSourceLoader::ProgramDefinitionFromSourceLoader(IProgramSourceLoader& srcLoader, const ProgramCompilerConfig& compilerConfig) noexcept
        : FromDefinitionLoader{ srcLoader }
        , _compiler{ compilerConfig }
    {
    }

    ProgramDefinitionFromSourceLoader::Result ProgramDefinitionFromSourceLoader::create(const std::shared_ptr<protobuf::ProgramSource>& src)
    {
        auto result = _compiler(*src);
        if(!result)
        {
            return unexpected{ result.error() };
		}
		return std::make_shared<protobuf::Program>(std::move(result.value()));
    }

    ProgramFileImporter::ProgramFileImporter()
        : _impl{ std::make_unique<ProgramFileImporterImpl>() }
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