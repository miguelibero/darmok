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

    ShaderCompiler::ShaderCompiler() noexcept
        : _shadercPath("shaderc")
        , _shaderType(ShaderType::Unknown)
    {
#if BX_PLATFORM_WINDOWS
        _shadercPath += ".exe";
#endif
    }

    void ShaderCompiler::reset() noexcept
    {
        _includes.clear();
        _defines.clear();
        _varyingDef = "";
        _shaderType = ShaderType::Unknown;
    }

    ShaderCompiler& ShaderCompiler::setShadercPath(const fs::path& path) noexcept
    {
        _shadercPath = path;
        return *this;
    }

    ShaderCompiler& ShaderCompiler::setIncludePaths(const std::vector<fs::path>& paths) noexcept
    {
        _includes = paths;
        return *this;
    }

    ShaderCompiler& ShaderCompiler::addIncludePath(const fs::path& path) noexcept
    {
        _includes.push_back(path);
        return *this;
    }

    ShaderCompiler& ShaderCompiler::setVaryingDef(const fs::path& path) noexcept
    {
        _varyingDef = path;
        return *this;
    }

    ShaderCompiler& ShaderCompiler::setShaderType(ShaderType type) noexcept
    {
        _shaderType = type;
        return *this;
    }

    ShaderCompiler& ShaderCompiler::setShaderType(const std::string& type)
    {
        _shaderType = getShaderType(type);
        return *this;
    }

    ShaderType ShaderCompiler::getShaderType(const std::string& name) noexcept
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

    std::string ShaderCompiler::getShaderTypeName(ShaderType type)
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

    ShaderCompiler& ShaderCompiler::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _log = log;
        return *this;
    }

    ShaderCompiler& ShaderCompiler::setDefines(const Defines& defines) noexcept
    {
        _defines = defines;
        return *this;
    }

    ShaderCompiler& ShaderCompiler::setDefines(const std::filesystem::path& shaderPath) noexcept
    {
        std::ifstream in(shaderPath);
        _defines.clear();
        getDefines(in, _defines);
        return *this;
    }

    const std::unordered_map<bgfx::RendererType::Enum, std::string> ShaderCompiler::_rendererExtensions{

        { bgfx::RendererType::Direct3D11,   ".dx11" },
        { bgfx::RendererType::Direct3D12,   ".dx12" },
        { bgfx::RendererType::Metal,        ".mtl" },
        { bgfx::RendererType::OpenGLES,     ".essl" },
        { bgfx::RendererType::OpenGL,       ".glsl" },
        { bgfx::RendererType::Vulkan,       ".spv" },
    };

    void ShaderCompiler::operator()(const fs::path& input, const Output& output) const
    {
        if (!fs::exists(_shadercPath))
        {
            throw std::runtime_error("could not find shaderc.exe");
        }
        fs::path varyingDef = _varyingDef;
        if (varyingDef.empty())
        {
            varyingDef = input.parent_path() / (StringUtils::getFileStem(input.string()) + ".varyingdef");
        }
        auto shaderType = getShaderType(input);
        std::vector<Exec::Arg> args{
            _shadercPath,
            "-p", output.profile,
            "-f", input,
            "-o", output.path,
            "--type", getShaderTypeName(shaderType),
            "--varyingdef", varyingDef
        };
#ifdef _DEBUG
        args.push_back("--debug");
#endif
        if (!output.defines.empty())
        {
            args.emplace_back("--define");
            std::vector<std::string> realDefines;
            realDefines.reserve(output.defines.size());
            for (auto& define : output.defines)
            {
                realDefines.push_back(_definePrefix + define);
            }
            args.emplace_back(StringUtils::join(",", realDefines));
        }

        auto inputDir = input.parent_path();
        if (!inputDir.empty())
        {
            args.push_back("-i");
            args.push_back(inputDir);
        }

        for (auto& include : _includes)
        {
            args.push_back("-i");
            args.push_back(include);
        }

        auto r = Exec::run(args);
        if (r.returnCode != 0 || r.out.contains("Failed to build shader."))
        {
            if (_log)
            {
                *_log << "shaderc output:" << std::endl;
                *_log << r.out;
                *_log << "shaderc error output:" << std::endl;
                *_log << r.err;
            }
            throw std::runtime_error("failed to build shader");
        }
    }

    const std::regex ShaderCompiler::_includeRegex = std::regex("#include <([^>]+)>");

    std::optional<std::filesystem::path> ShaderCompiler::readDependency(const std::string& line) const noexcept
    {
        std::smatch match;
        if (!std::regex_search(line, match, _includeRegex))
        {
            return std::nullopt;
        }
        auto name = match[1].str();
        for (auto& include : _includes)
        {
            auto path = include / name;
            if (fs::exists(path))
            {
                return path;
            }
        }
        return std::nullopt;
    }

    size_t ShaderCompiler::getDependencies(std::istream& in, Dependencies& deps) const noexcept
    {
        std::unordered_set<std::filesystem::path> checkedPaths;
        return getDependencies(in, deps, checkedPaths);
    }

    size_t ShaderCompiler::getDependencies(std::istream& in, Dependencies& deps, std::unordered_set<std::filesystem::path>& checkedPaths) const noexcept
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

    const std::regex ShaderCompiler::_ifdefRegex = std::regex("#(if|ifdef|ifndef) !?([^\\s]+)");
    const std::string ShaderCompiler::_definePrefix = "DARMOK_VARIANT_";

    std::optional<std::string> ShaderCompiler::readDefine(const std::string& line) const noexcept
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

    size_t ShaderCompiler::getDefines(std::istream& in, Defines& defines) const noexcept
    {
        std::unordered_set<std::filesystem::path> checkedPaths;
        return getDefines(in, defines, checkedPaths);
    }

    size_t ShaderCompiler::getDefines(std::istream& in, Defines& defines, std::unordered_set<std::filesystem::path>& checkedPaths) const noexcept
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

    const std::string ShaderCompiler::_binExt = ".bin";
    const std::string ShaderCompiler::_enableDefineSuffix = "_enabled";

    ShaderType ShaderCompiler::getShaderType(const fs::path& path) const noexcept
    {
        if (_shaderType != ShaderType::Unknown)
        {
            return _shaderType;
        }
        auto exts = StringUtils::split(StringUtils::getFileExt(path.filename().string()), ".");
        for (auto& ext : exts)
        {
            auto type = getShaderType(ext);
            if (type != ShaderType::Unknown)
            {
                return type;
            }
        }
        return ShaderType::Unknown;
    }

    fs::path ShaderCompiler::getOutputPath(const fs::path& path, const std::string& profileExt, const Defines& defines) const noexcept
    {
        auto suffix = StringUtils::join("-", defines.begin(), defines.end(), [](std::string define) {
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
        auto stem = StringUtils::getFileStem(path.string());
        auto shaderType = getShaderType(path);
        auto type = "." + getShaderTypeName(shaderType);
        return path.parent_path() / (stem + suffix + type + profileExt + _binExt);
    }

    const std::vector<bgfx::RendererType::Enum>& ShaderCompiler::getSupportedRenderers() noexcept
    {
        static const std::vector<bgfx::RendererType::Enum> renderers
        {
#if BX_PLATFORM_WINDOWS
            bgfx::RendererType::Direct3D11, bgfx::RendererType::Direct3D12,
#endif
#if BX_PLATFORM_OSX
            bgfx::RendererType::Metal,
#endif
            bgfx::RendererType::Vulkan, bgfx::RendererType::OpenGL, bgfx::RendererType::OpenGLES,
        };
        return renderers;
    }

    std::vector<ShaderCompiler::Output> ShaderCompiler::getOutputs(const fs::path& basePath) const noexcept
    {
        auto defineCombs = CollectionUtils::combinations(_defines);
        std::vector<Output> outputs;

        auto& rendererProfiles = ProgramDefinition::getRendererProfiles();
        auto& renderers = getSupportedRenderers();

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
                    outputs.push_back(Output{
                        .path = getOutputPath(basePath, profileExt, defines),
                        .profile = profile,
                        .defines = defines,
                    });
                }
            }
        }
        return outputs;
    }

    void ProgramImportConfig::read(const nlohmann::ordered_json& json, fs::path basePath)
    {
        if (json.contains("name"))
        {
            name = json["name"];
        }
        if (json.contains("vertexShader"))
        {
            vertexShader = basePath / json["vertexShader"].get<std::string>();
        }
        if (json.contains("fragmentShader"))
        {
            fragmentShader = basePath / json["fragmentShader"].get<std::string>();
        }
        if (json.contains("varyingDef"))
        {
            fs::path varyingPath = basePath / json["varyingDef"].get<std::string>();
            varying.read(varyingPath);
        }
        else
        {
            varying.read(json);
        }
    }

    void ProgramImportConfig::load(const AssetTypeImporterInput& input)
    {
        auto basePath = input.path.parent_path();
        auto fileName = input.path.filename();
        auto stem = StringUtils::getFileStem(fileName.string());
        vertexShader = basePath / (stem + ".vertex.sc");
        fragmentShader = basePath / (stem + ".fragment.sc");
        name = stem;

        if (input.config.is_object())
        {
            read(input.config, input.basePath);
        }
        if (fs::exists(input.path))
        {
            std::ifstream is(input.path);
            auto fileJson = nlohmann::ordered_json::parse(is);
            read(fileJson, input.path.parent_path());
        }
        if (!fs::exists(vertexShader))
        {
            throw std::runtime_error(std::string("missing vertex shader: ") + vertexShader.string());
        }
        if (!fs::exists(fragmentShader))
        {
            throw std::runtime_error(std::string("missing fragment shader: ") + fragmentShader.string());
        }
    }

    ProgramImporterImpl::ProgramImporterImpl(size_t bufferSize) noexcept
        : _bufferSize(bufferSize)
    {
    }

    void ProgramImporterImpl::setShadercPath(const std::filesystem::path& path) noexcept
    {
        _compiler.setShadercPath(path);
    }

    void ProgramImporterImpl::addIncludePath(const std::filesystem::path& path) noexcept
    {
        _includes.push_back(path);
    }

    void ProgramImporterImpl::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _compiler.setLogOutput(log);
    }

    void ProgramImporterImpl::addIncludes(const nlohmann::json& json, const fs::path& basePath) noexcept
    {
        for (auto& elm : json)
        {
            _compiler.addIncludePath(basePath / elm.get<std::string>());
        }
    }

    const std::string ProgramImporterImpl::_configIncludeDirsKey = "includeDirs";
    const std::string ProgramImporterImpl::_shadercPathKey = "shadercPath";

    bool ProgramImporterImpl::startImport(const Input& input, bool dry)
    {
        if (input.config.is_null())
        {
            auto ext = StringUtils::getFileExt(input.path.filename().string());
            if (ext != ".program.json")
            {
                return false;
            }
        }

        _compiler.reset();
        _compiler.setIncludePaths(_includes);
        if (input.config.contains(_configIncludeDirsKey))
        {
            addIncludes(input.config[_configIncludeDirsKey], input.basePath);
        }
        if (input.dirConfig.contains(_configIncludeDirsKey))
        {
            addIncludes(input.dirConfig[_configIncludeDirsKey], input.basePath);
        }
        _compiler.addIncludePath(input.path.parent_path());

        if (input.dirConfig.contains(_shadercPathKey))
        {
            fs::path shadercPath = input.dirConfig[_shadercPathKey];
            _compiler.setShadercPath(input.basePath / shadercPath);
        }

        _config.emplace().load(input);

        _outputPath = input.getRelativePath().parent_path();
        if (input.config.contains("outputPath"))
        {
            _outputPath /= input.config["outputPath"].get<fs::path>();
        }
        else if (input.dirConfig.contains("outputPath"))
        {
            std::string v = input.dirConfig["outputPath"];
            std::string name = _config->name;
            if (name.empty())
            {
                name = StringUtils::getFileStem(input.getRelativePath().string());
            }
            StringUtils::replace(v, "*", name);
            _outputPath /= v;
        }
        else
        {
            _outputPath /= input.path.stem().string() + ".bin";
        }

        return true;
    }

    std::vector<fs::path> ProgramImporterImpl::getOutputs(const Input& input)
    {
        return { _outputPath };
    }

    ProgramImporterImpl::Dependencies ProgramImporterImpl::getDependencies(const Input& input)
    {
        Dependencies deps;
        if (!_config)
        {
            return deps;
        }

        auto addShaderDeps = [this, &deps](const fs::path& path)
        {
            deps.insert(path);
            std::ifstream is(path);
            _compiler.getDependencies(is, deps);
        };

        auto& config = _config.value();
        addShaderDeps(config.vertexShader);
        addShaderDeps(config.fragmentShader);
        return deps;
    }

    void ProgramImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        if (!_config)
        {
            return;
        }
        auto& config = _config.value();
        ProgramDefinition def
        {
            .name = config.name,
            .vertexLayout = config.varying.vertex,
        };
        auto ext = _outputPath.extension().string();
        auto outputPath = getTempPath("-darmok.program" + ext);
        auto varyingDefPath = getTempPath("-darmok.varyingdef");
        _compiler.setVaryingDef(varyingDefPath);

        _compiler.setDefines(config.vertexShader);
        _compiler.setShaderType(ShaderType::Vertex);
        for (ShaderCompilerOutput output : _compiler.getOutputs())
        {
            {
                std::ofstream out(varyingDefPath);
                config.varying.writeBgfx(out, output.defines);
            }
            output.path = outputPath;
            _compiler(config.vertexShader, output);
            auto data = Data::fromFile(output.path);
            def.profiles[output.profile].vertexShaders[output.defines] = std::move(data);
        }

        _compiler.setDefines(config.fragmentShader);
        _compiler.setShaderType(ShaderType::Fragment);
        for (ShaderCompilerOutput output : _compiler.getOutputs())
        {
            {
                std::ofstream out(varyingDefPath);
                config.varying.writeBgfx(out, output.defines);
            }
            output.path = outputPath;
            _compiler(config.fragmentShader, output);
            auto data = Data::fromFile(output.path);
            def.profiles[output.profile].fragmentShaders[output.defines] = std::move(data);
        }
        writeDefinition(def, out);
        fs::remove(outputPath);
        fs::remove(varyingDefPath);
    }

    void ProgramImporterImpl::writeDefinition(const ProgramDefinition& def, std::ostream& out)
    {
        auto ext = _outputPath.extension();
        if (ext == ".xml")
        {
            cereal::XMLOutputArchive archive(out);
            archive(def);
        }
        else if (ext == ".json")
        {
            cereal::JSONOutputArchive archive(out);
            archive(def);
        }
        else
        {
            cereal::PortableBinaryOutputArchive archive(out);
            archive(def);
        }
    }

    void ProgramImporterImpl::endImport(const Input& input)
    {
        _config.reset();
        _outputPath = "";
    }

    const std::string& ProgramImporterImpl::getName() const noexcept
    {
        static const std::string name("program");
        return name;
    }

    ProgramImporter::ProgramImporter()
        : _impl(std::make_unique<ProgramImporterImpl>())
    {
    }

    ProgramImporter::~ProgramImporter() noexcept
    {
        // empty on purpose
    }

    ProgramImporter& ProgramImporter::setShadercPath(const fs::path& path) noexcept
    {
        _impl->setShadercPath(path);
        return *this;
    }

    ProgramImporter& ProgramImporter::addIncludePath(const fs::path& path) noexcept
    {
        _impl->addIncludePath(path);
        return *this;
    }

    void ProgramImporter::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _impl->setLogOutput(log);
    }

    bool ProgramImporter::startImport(const Input& input, bool dry)
    {
        return _impl->startImport(input, dry);
    }

    ProgramImporter::Outputs ProgramImporter::getOutputs(const Input& input)
    {
        return _impl->getOutputs(input);
    }

    ProgramImporter::Dependencies ProgramImporter::getDependencies(const Input& input)
    {
        return _impl->getDependencies(input);
    }

    void ProgramImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        _impl->writeOutput(input, outputIndex, out);
    }

    const std::string& ProgramImporter::getName() const noexcept
    {
        return _impl->getName();
    }

    void ProgramImporter::endImport(const Input& input)
    {
        return _impl->endImport(input);
    }

    ProgramDefinitionLoader::ProgramDefinitionLoader(IDataLoader& dataLoader) noexcept
        : CerealLoader(dataLoader)
    {
    }
}