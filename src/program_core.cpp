#include <darmok/program_core.hpp>
#include "program_core.hpp"
#include <darmok/data_stream.hpp>
#include <darmok/string.hpp>
#include <darmok/utils.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/binary.hpp>

namespace darmok
{
    namespace fs = std::filesystem;

    void ProgramProfileDefinition::read(const nlohmann::json& json)
    {
        auto readMap = [](const nlohmann::json& json, Map& map)
        {
            for (auto& elm: json)
            {
                std::string data;
                Defines defs;
                if (elm.is_string())
                {
                    data = elm;
                }
                else if (elm.is_object())
                {
                    if (elm.contains("defines"))
                    {
                        defs = elm["defines"];
                    }
                    if (elm.contains("data"))
                    {
                        data = elm["data"];
                    }
                }
                else if (elm.is_array())
                {
                    defs = elm[0];
                    data = elm[1];
                }
                else
                {
                    continue;
                }
                map.emplace(defs, Data::fromHex(data));
            }
        };
        if (json.contains("vertex"))
        {
            readMap(json["vertex"], vertexShaders);
        }
        if (json.contains("fragment"))
        {
            readMap(json["fragment"], fragmentShaders);
        }
    }

    void ProgramProfileDefinition::write(nlohmann::json& json) const
    {
        auto writeMap = [](nlohmann::json& json, const Map& map)
        {
            for (auto& [defines, data] : map)
            {
                auto& elmJson = json.emplace_back();
                elmJson["defines"] = defines;
                elmJson["data"] = data.view().toHex();
            }
        };

        writeMap(json["vertex"], vertexShaders);
        writeMap(json["fragment"], fragmentShaders);
    }

    const std::unordered_map<bgfx::RendererType::Enum, std::vector<std::string>> ProgramDefinition::_rendererProfiles
    {
        { bgfx::RendererType::Direct3D11, { "s_5_0" }},
        { bgfx::RendererType::Direct3D12, { "s_5_0" }},
        { bgfx::RendererType::Metal, { "metal" }},
        { bgfx::RendererType::OpenGLES, { "320_es", "310_es", "300_es", "100_es" }},
        { bgfx::RendererType::OpenGL, { "440", "430", "420", "410", "400", "330", "150", "140", "130", "120" }},
        { bgfx::RendererType::Vulkan, { "spirv" }},
    };

    const ProgramDefinition::Profile& ProgramDefinition::getCurrentProfile() const
    {
        auto render = bgfx::getRendererType();
        auto itr = _rendererProfiles.find(render);
        if (itr == _rendererProfiles.end())
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

    ProgramDefinition::Format ProgramDefinition::getPathFormat(const std::filesystem::path& path) noexcept
    {
        auto ext = path.extension();
        if (ext == ".json")
        {
            return Format::Json;
        }
        if (ext == ".xml")
        {
            return Format::Xml;
        }
        return Format::Binary;
    }

    void ProgramDefinition::read(const std::filesystem::path& path)
    {
        std::ifstream in(path);
        read(in, getPathFormat(path));
    }

    void ProgramDefinition::write(const std::filesystem::path& path) const noexcept
    {
        std::ofstream out(path);
        write(out, getPathFormat(path));
    }

    void ProgramDefinition::read(std::istream& in, Format format)
    {
        if (format == Format::Json)
        {
            auto json = nlohmann::ordered_json::parse(in);
            read(json);
        }
        else if (format == Format::Xml)
        {
            cereal::XMLInputArchive archive(in);
            archive(*this);
        }
        else
        {
            cereal::BinaryInputArchive archive(in);
            archive(*this);
        }
    }

    void ProgramDefinition::write(std::ostream& out, Format format) const noexcept
    {
        if (format == Format::Json)
        {
            auto json = nlohmann::ordered_json::object();
            write(json);
            out << json.dump(2);
        }
        else if (format == Format::Xml)
        {
            cereal::XMLOutputArchive archive(out);
            archive(*this);
        }
        else
        {
            cereal::BinaryOutputArchive archive(out);
            archive(*this);
        }
    }

    void ProgramDefinition::read(const nlohmann::ordered_json& json)
    {
        if (json.contains("name"))
        {
            name = json["name"];
        }
        if (json.contains("vertexLayout"))
        {
            vertexLayout.read(json["vertexLayout"]);
        }
        if (json.contains("profiles"))
        {
            for (auto& [name, profileJson] : json["profiles"].items())
            {
                profiles[name].read(profileJson);
            }
        }
    }

    void ProgramDefinition::write(nlohmann::ordered_json& json) const
    {
        if (!name.empty())
        {
            json["name"] = name;
        }
        vertexLayout.write(json["vertexLayout"]);
        nlohmann::json profilesJson;
        for (auto& [name, profile] : profiles)
        {
            profile.write(profilesJson[name]);
        }
        json["profiles"] = profilesJson;
    }

    void ProgramDefinition::load(DataView data)
    {
        DataInputStream::read(data, *this);
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

    const std::vector<std::string> ShaderCompiler::_profiles{
        "120", "300_es", "spirv",
#if BX_PLATFORM_WINDOWS
        "s_5_0",
#endif
#if BX_PLATFORM_OSX
        "metal",
#endif
    };

    const std::unordered_map<std::string, std::string> ShaderCompiler::_profileExtensions{
        { "300_es", ".essl" },
        { "120",    ".glsl" },
        { "spirv",  ".spv" },
        { "metal",  ".mtl" },
        { "s_3_0",  ".dx9" },
        { "s_4_0",  ".dx10" },
        { "s_5_0",  ".dx11" }
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
            args.emplace_back(StringUtils::join(",", output.defines.begin(), output.defines.end()));
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

    size_t ShaderCompiler::getDependencies(std::istream& in, Dependencies& deps) const noexcept
    {
        std::string line;
        size_t count = 0;
        while (std::getline(in, line))
        {
            std::smatch match;
            if (!std::regex_search(line, match, _includeRegex))
            {
                continue;
            }
            auto name = match[1].str();
            for (auto& include : _includes)
            {
                auto path = include / name;
                if (!fs::exists(path))
                {
                    continue;
                }
                if (deps.insert(path).second)
                {
                    count++;
                }
                break;
            }
        }
        return count;
    }

    const std::regex ShaderCompiler::_ifdefRegex = std::regex("#(if|ifdef|ifndef) !?([^\\s]+)");

    size_t ShaderCompiler::getDefines(std::istream& in, Defines& defines) noexcept
    {
        std::string line;
        size_t count = 0;
        while (std::getline(in, line))
        {
            std::smatch match;
            if (!std::regex_search(line, match, _ifdefRegex))
            {
                continue;
            }
            auto define = match[2].str();
            if (define.starts_with("BGFX_"))
            {
                continue;
            }
            if (defines.insert(define).second)
            {
                count++;
            }
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

    std::vector<ShaderCompiler::Output> ShaderCompiler::getOutputs(const fs::path& basePath) const noexcept
    {
        auto defineCombs = CollectionUtils::combinations(_defines);
        std::vector<Output> outputs;
        for (auto& profile : _profiles)
        {
            std::string profileExt = profile;
            auto itr = _profileExtensions.find(profile);
            if (itr != _profileExtensions.end())
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

        auto addShaderDeps = [this, &deps](const fs::path& path) {
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
        auto format = ProgramDefinition::getPathFormat(_outputPath);
        def.write(out, format);

        fs::remove(outputPath);
        fs::remove(varyingDefPath);
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

}