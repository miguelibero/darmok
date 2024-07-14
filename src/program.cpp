#include <darmok/program.hpp>
#include "program.hpp"
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/vertex.hpp>
#include <darmok/vertex_layout.hpp>
#include <darmok/stream.hpp>
#include <darmok/utils.hpp>
#include <darmok/string.hpp>
#include <darmok/data_stream.hpp>
#include <nlohmann/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/string.hpp>
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
        { bgfx::RendererType::Direct3D11, { "s_4_0" }},
        { bgfx::RendererType::Direct3D12, { "s_5_0", "s_4_0" }},
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

    void ProgramDefinition::read(const nlohmann::ordered_json& json)
    {
        if (json.contains("name"))
        {
            name = json["name"];
        }
        if (json.contains("vertexLayout"))
        {
            VertexLayoutUtils::readJson(json["vertexLayout"], vertexLayout);
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
        VertexLayoutUtils::writeJson(json["vertexLayout"], vertexLayout);
        nlohmann::json profilesJson;
        for (auto& [name, profile] : profiles)
        {
            profile.write(profilesJson[name]);
        }
        json["profiles"] = profilesJson;
    }

    Program::ShaderHandles Program::createShaders(const ProgramProfileDefinition::Map& defMap, const std::string& name)
    {
        ShaderHandles handles;
        for (auto& [defines, data] : defMap)
        {
            auto handle = bgfx::createShader(data.copyMem());
            auto defName = name + StringUtils::join(" ", defines.begin(), defines.end());
            if (!isValid(handle))
            {
                throw std::runtime_error("failed to compile shader: " + defName);
            }
            bgfx::setName(handle, defName.c_str());
            _vertexHandles[defines] = handle;
        }
        return handles;
    }

	Program::Program(const Definition& def)
        : _layout(def.vertexLayout)
	{
        auto& profile = def.getCurrentProfile();
        _vertexHandles = createShaders(profile.vertexShaders, def.name + " vertex ");
        _fragmentHandles = createShaders(profile.fragmentShaders, def.name + " fragment ");

        for (auto& [vertDefines, vertHandle] : _vertexHandles)
        {
            for (auto& [fragDefines, fragHandle] : _fragmentHandles)
            {
                auto defines = vertDefines;
                defines.insert(fragDefines.begin(), fragDefines.end());
                _handles[defines] = bgfx::createProgram(vertHandle, fragHandle);
            }
        }
	}

    Program::Program(const Handles& handles, const bgfx::VertexLayout& layout) noexcept
		: _handles(handles)
		, _layout(layout)
	{
	}

    Program::~Program() noexcept
    {
        for (auto& [defines, handle] : _vertexHandles)
        {
            if (isValid(handle))
            {
                bgfx::destroy(handle);
            }
        }
        for (auto& [defines, handle] : _fragmentHandles)
        {
            if (isValid(handle))
            {
                bgfx::destroy(handle);
            }
        }
        for (auto& [defines, handle] : _handles)
        {
            if (isValid(handle))
            {
                bgfx::destroy(handle);
            }
        }
    }

	bgfx::ProgramHandle Program::getHandle(const Defines& defines) const noexcept
	{
        auto itr = _handles.find(defines);
        if (itr != _handles.end())
        {
            return itr->second;
        }
        return { bgfx::kInvalidHandle };
	}

	const bgfx::VertexLayout& Program::getVertexLayout() const noexcept
	{
		return _layout;
	}

	DataProgramLoader::DataProgramLoader(IDataLoader& dataLoader) noexcept
		: _dataLoader(dataLoader)
	{
	}

	std::shared_ptr<Program> DataProgramLoader::operator()(std::string_view name)
	{
        auto defData = _dataLoader(name);
        ProgramDefinition def;
        DataInputStream::read(defData, def);
        if (def.name.empty())
        {
            def.name = name;
        }
		return std::make_shared<Program>(def);
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

    std::string ShaderCompiler::fixPathArgument(const fs::path& path) noexcept
    {
        auto str = fs::absolute(path).string();
        std::replace(str.begin(), str.end(), '\\', '/');
        return str;
    }

    const std::vector<std::string> ShaderCompiler::_profiles{
        "120", "300_es", "spirv",
#if BX_PLATFORM_WINDOWS
        "s_4_0", "s_5_0",
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

    int ShaderCompiler::operator()(const fs::path& input, const Output& output) const
    {
        fs::path varyingDef = _varyingDef;
        if (varyingDef.empty())
        {
            varyingDef = input.parent_path() / (StringUtils::getFileStem(input.string()) + ".varyingdef");
        }
        auto shaderType = getShaderType(input);
        std::vector<std::string> args{
            fixPathArgument(_shadercPath),
            "-p", output.profile,
            "-f", fixPathArgument(input),
            "-o", fixPathArgument(output.path),
            "--type", getShaderTypeName(shaderType),
            "--varyingdef", fixPathArgument(varyingDef)
        };
        if (!output.defines.empty())
        {
            args.emplace_back("--define");
            args.emplace_back(StringUtils::join(",", output.defines.begin(), output.defines.end()));
        }
        for (auto& include : _includes)
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
        return r.returnCode;
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

    ShaderImporterImpl::ShaderImporterImpl(size_t bufferSize) noexcept
        : _bufferSize(bufferSize)
    {
    }

    void ShaderImporterImpl::setShadercPath(const fs::path& path) noexcept
    {
        _compiler.setShadercPath(path);
    }

    void ShaderImporterImpl::addIncludePath(const fs::path& path) noexcept
    {
        _includes.push_back(path);
    }

    void ShaderImporterImpl::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _compiler.setLogOutput(log);
    }

    const std::string ShaderImporterImpl::_manifestFileSuffix = ".manifest.json";
    const std::string ShaderImporterImpl::_configTypeKey = "type";
    const std::string ShaderImporterImpl::_configVaryingDefKey = "varyingDef";


    struct ShaderCompilerUtils final
    {
        static void configureIncludes(ShaderCompiler& compiler, const AssetTypeImporterInput& input) noexcept
        {
            if (input.config.contains(_configIncludeDirsKey))
            {
                addIncludes(compiler, input.config[_configIncludeDirsKey], input.basePath);
            }
            if (input.dirConfig.contains(_configIncludeDirsKey))
            {
                addIncludes(compiler, input.dirConfig[_configIncludeDirsKey], input.basePath);
            }
            compiler.addIncludePath(input.path.parent_path());
        }

        static fs::path getTempPath() noexcept
        {
            return fs::temp_directory_path() / fs::path(std::tmpnam(nullptr));
        }

    private:

        static void addIncludes(ShaderCompiler& compiler, const nlohmann::json& json, const fs::path& basePath) noexcept
        {
            for (auto& elm : json)
            {
                compiler.addIncludePath(basePath / elm.get<std::string>());
            }
        }

        static const std::string _configIncludeDirsKey;
    };
    const std::string ShaderCompilerUtils::_configIncludeDirsKey = "includeDirs";


    bool ShaderImporterImpl::startImport(const Input& input, bool dry)
    {
        if (input.config.is_null())
        {
            return false;
        }
        
        _compiler.reset();
        _compiler.setIncludePaths(_includes);
        ShaderCompilerUtils::configureIncludes(_compiler, input);

        if (input.config.contains("defines"))
        {
            _compiler.setDefines(input.config["defines"]);
        }

        if (input.config.contains(_configVaryingDefKey))
        {
             fs::path varyingDef = input.basePath / input.config[_configVaryingDefKey].get<std::string>();
            _compiler.setVaryingDef(varyingDef);
        }

        if (input.config.contains(_configTypeKey))
        {
            std::string type = input.config[_configTypeKey];
            _compiler.setShaderType(type);
        }

        _basePath = input.getRelativePath();
        if (input.config.contains("outputPath"))
        {
            _basePath = input.config["outputPath"].get<fs::path>();
        }
        _outputs = _compiler.getOutputs(_basePath);
        return true;
    }

    void ShaderImporterImpl::endImport(const Input& input)
    {
        _outputs.clear();
    }

    std::vector<fs::path> ShaderImporterImpl::getOutputs(const Input& input)
    {
        std::vector<fs::path> outputs;

        // manifest file
        outputs.push_back(_basePath.stem().string() + _manifestFileSuffix);

        for (auto& output : _outputs)
        {
            outputs.push_back(output.path);
        }

        return outputs;
    }

    ShaderImporterImpl::Dependencies ShaderImporterImpl::getDependencies(const Input& input)
    {
        Dependencies deps;
        if (!fs::exists(input.path))
        {
            return deps;
        }
        std::ifstream is(input.path);
        _compiler.getDependencies(is, deps);
        return deps;
    }

    void ShaderImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        if (outputIndex == 0)
        {
            // manifest file
            auto json = nlohmann::json::object();
            for (auto& output : _outputs)
            {
                auto& configJson = json[output.path.string()];
                configJson["profile"] = output.profile;
                configJson["defines"] = output.defines;
            }
            out << json.dump(2);
            return;
        }

        outputIndex--;
        if (outputIndex < 0 || outputIndex >= _outputs.size())
        {
            throw std::runtime_error("cannot find output config");
        }

        ShaderCompilerOutput output = _outputs[outputIndex];
        output.path = ShaderCompilerUtils::getTempPath();
        _compiler(input.path, output);
        std::ifstream is(output.path, std::ios::binary);
        StreamUtils::copy(is, out, _bufferSize);
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

    bool ShaderImporter::startImport(const Input& input, bool dry)
    {
        return _impl->startImport(input, dry);
    }

    ShaderImporter::Outputs ShaderImporter::getOutputs(const Input& input)
    {
        return _impl->getOutputs(input);
    }

    ShaderImporter::Dependencies ShaderImporter::getDependencies(const Input& input)
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

    void ShaderImporter::endImport(const Input& input)
    {
        _impl->endImport(input);
    }

    void ShaderImportConfig::read(const nlohmann::json& json, fs::path basePath)
    {
        if (json.is_string())
        {
            path = basePath / json.get<std::string>();
            return;
        }
        if (json.contains("path"))
        {
            path = basePath / json["path"].get<std::string>();
        }
        if (json.contains("defines"))
        {
            defines = json["defines"];
        }
    }

    void ProgramImportConfig::read(const nlohmann::ordered_json& json, fs::path basePath)
    {
        if (json.contains("name"))
        {
            name = json["name"];
        }
        if (json.contains("vertexShader"))
        {
            vertexShader.read(json["vertexShader"], basePath);
        }
        if (json.contains("fragmentShader"))
        {
            fragmentShader.read(json["fragmentShader"], basePath);
        }
        if (json.contains("varyingDef"))
        {
            varyingDefPath = basePath / json["varyingDef"].get<std::string>();
        }
        if (json.contains("vertexLayout"))
        {
            auto& layoutJson = json["vertexLayout"];
            if (layoutJson.is_string())
            {
                vertexLayoutPath = basePath / layoutJson.get<std::string>();
            }
            else
            {
                VertexLayoutUtils::readJson(layoutJson, vertexLayout);
            }
        }
    }

    void ProgramImportConfig::load(const AssetTypeImporterInput& input)
    {
        auto basePath = input.path.parent_path();
        auto fileName = input.path.filename();
        auto stem = StringUtils::getFileStem(fileName.string());
        varyingDefPath = basePath / (stem + ".varyingdef");
        vertexShader.path = basePath / (stem + ".vertex.sc");
        fragmentShader.path = basePath / (stem + ".fragment.sc");
        vertexLayoutPath = basePath / (stem + ".vlayout.json");

        if (input.config.is_object())
        {
            read(input.config, input.basePath);
        }
        if (fs::exists(input.path))
        {
            std::ifstream is(input.path);
            auto fileJson = nlohmann::ordered_json::parse(is);
            read(fileJson, input.basePath);
        }
        if (!fs::exists(varyingDefPath))
        {
            throw std::runtime_error(std::string("missing varyingdef: ") + varyingDefPath.string());
        }
        if (!fs::exists(vertexShader.path))
        {
            throw std::runtime_error(std::string("missing vertex shader: ") + vertexShader.path.string());
        }
        if (!fs::exists(fragmentShader.path))
        {
            throw std::runtime_error(std::string("missing fragment shader: ") + fragmentShader.path.string());
        }
        if (vertexLayout.getStride() == 0 && fs::exists(vertexLayoutPath))
        {
            VertexLayoutUtils::readFile(vertexLayoutPath, vertexLayout);
        }
        if (vertexLayout.getStride() == 0)
        {
            std::ifstream is(varyingDefPath);
            VertexLayoutUtils::readVaryingDef(is, vertexLayout);
        }
        if (vertexLayout.getStride() == 0)
        {
            throw std::runtime_error("empty vertex layout");
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

    bool ProgramImporterImpl::startImport(const Input& input, bool dry)
    {
        if (input.config.is_null())
        {
            return false;
        }

        _compiler.reset();
        _compiler.setIncludePaths(_includes);
        ShaderCompilerUtils::configureIncludes(_compiler, input);

        _config.emplace().load(input);

        _outputPath = input.getRelativePath().parent_path();
        if (input.config.contains("outputPath"))
        {
            _outputPath /= input.config["outputPath"].get<fs::path>();
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
        addShaderDeps(config.vertexShader.path);
        addShaderDeps(config.fragmentShader.path);
        deps.insert(config.varyingDefPath);
        if (!config.vertexLayoutPath.empty())
        {
            deps.insert(config.vertexLayoutPath);
        }
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
            .vertexLayout = config.vertexLayout,
        };
        auto tmpPath = ShaderCompilerUtils::getTempPath();

        _compiler.setShaderType(ShaderType::Vertex);
        _compiler.setDefines(config.vertexShader.defines);
        for (auto output : _compiler.getOutputs())
        {
            output.path = tmpPath;
            _compiler(config.vertexShader.path, output);
            auto data = Data::fromFile(tmpPath);
            def.profiles[output.profile].vertexShaders[output.defines] = data;
        }
        _compiler.setShaderType(ShaderType::Fragment);
        _compiler.setDefines(config.fragmentShader.defines);
        for (auto output : _compiler.getOutputs())
        {
            output.path = tmpPath;
            _compiler(config.fragmentShader.path, output);
            auto data = Data::fromFile(tmpPath);
            def.profiles[output.profile].fragmentShaders[output.defines] = data;
        }

        auto ext = _outputPath.extension();
        if (ext == ".json")
        {
            auto json = nlohmann::ordered_json::object();
            def.write(json);
            out << json.dump(2);
        }
        else if (ext == ".xml")
        {
            cereal::XMLOutputArchive archive(out);
            archive(def);
        }
        else
        {
            cereal::BinaryOutputArchive archive(out);
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