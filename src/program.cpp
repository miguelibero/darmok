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

namespace darmok
{
    namespace fs = std::filesystem;

    struct ShaderDataUtils final
    {
        static std::string getRendererTypeName(bgfx::RendererType::Enum type)
        {
            switch (type)
            {
                case bgfx::RendererType::Noop:
                    return "noop";
                case bgfx::RendererType::Direct3D11:
                    return "dx11";
                case bgfx::RendererType::Direct3D12:
                    return "dx12";
                case bgfx::RendererType::Agc:
                    return "agc";
                case bgfx::RendererType::Gnm:
                    return "gnm";
                case bgfx::RendererType::Metal:
                    return "metal";
                case bgfx::RendererType::Nvn:
                    return "nvn";
                case bgfx::RendererType::OpenGL:
                    return "glsl";
                case bgfx::RendererType::OpenGLES:
                    return "essl";
                case bgfx::RendererType::Vulkan:
                    return "spv";
            }
            throw std::runtime_error("unknown renderer type");
        }

        static bgfx::RendererType::Enum getRendererType(std::string_view name)
        {
            auto lowerName = StringUtils::toLower(name);
            if (name == "noop")
            {
                return bgfx::RendererType::Noop;
            }
            if (name == "dx11")
            {
                return bgfx::RendererType::Direct3D11;
            }
            if (name == "dx12")
            {
                return bgfx::RendererType::Direct3D12;
            }
            if (name == "agc")
            {
                return bgfx::RendererType::Agc;
            }
            if (name == "gnm")
            {
                return bgfx::RendererType::Gnm;
            }
            if (name == "metal")
            {
                return bgfx::RendererType::Metal;
            }
            if (name == "nvn")
            {
                return bgfx::RendererType::Nvn;
            }
            if (name == "glsl")
            {
                return bgfx::RendererType::OpenGL;
            }
            if (name == "essl")
            {
                return bgfx::RendererType::OpenGLES;
            }
            if (name == "spv")
            {
                return bgfx::RendererType::Vulkan;
            }
            throw std::runtime_error("unknown renderer type");
        }

        static void read(const nlohmann::json& json, ShaderData& data)
        {
            for (auto& elm : json.items())
            {
                auto type = getRendererType(elm.key());
                data[type] = Data::fromHex(elm.value().get<std::string>());
            }
        }

        static void write(nlohmann::json& json, const ShaderData& data)
        {
            for (auto& elm : data)
            {
                json[getRendererTypeName(elm.first)] = elm.second.view().toHex();
            }
        }
        
        static bgfx::ShaderHandle createShader(const ShaderData& data)
        {
            auto itr = data.find(bgfx::getRendererType());
            if (itr == data.end())
            {
                throw std::invalid_argument("missing shader renderer data");
            }
            auto handle = bgfx::createShader(itr->second.makeRef());
            if (!isValid(handle))
            {
                throw std::runtime_error("failed to load shader");
            }
            return handle;
        }
    };

    void ProgramShaderDefinition::read(const nlohmann::json& json)
    {
        if (json.contains("vertexShader"))
        {
            ShaderDataUtils::read(json["vertexShader"], vertexShader);
        }
        if (json.contains("fragmentShader"))
        {
            ShaderDataUtils::read(json["fragmentShader"], fragmentShader);
        }
        if (json.contains("defines"))
        {
            defines = json["defines"];
        }
    }
    void ProgramShaderDefinition::write(nlohmann::json& json)
    {
        ShaderDataUtils::write(json["vertexShader"], vertexShader);
        ShaderDataUtils::write(json["fragmentShader"], fragmentShader);
        json["defines"] = defines;
    }

    void ProgramDefinition::read(const nlohmann::ordered_json& json)
    {
        if (json.contains("name"))
        {
            name = json["name"];
        }
        if (json.contains("shaders"))
        {
            for (auto& shaderJson : json["shaders"])
            {
                shaders.emplace_back().read(shaderJson);
            }
        }
        if (json.contains("vertexLayout"))
        {
            VertexLayoutUtils::readJson(json["vertexLayout"], vertexLayout);
        }
    }

    void ProgramDefinition::write(nlohmann::ordered_json& json)
    {
        json["name"] = name;
        nlohmann::json shadersJson = json["shaders"];
        for (auto& shader : shaders)
        {
            shader.write(shadersJson.emplace_back());
        }
        VertexLayoutUtils::writeJson(json["vertexLayout"], vertexLayout);
    }

	Program::Program(const Definition& def)
	{
        _layout = def.vertexLayout;

        for (auto& shader : def.shaders)
        {
            auto baseName = def.name + " " + StringUtils::join(" ", shader.defines.begin(), shader.defines.end());
            auto vertHandle = ShaderDataUtils::createShader(shader.vertexShader);
            auto vertName = baseName + " vertex";
            bgfx::setName(vertHandle, vertName.c_str());
            auto fragHandle = ShaderDataUtils::createShader(shader.fragmentShader);
            auto fragName = baseName + " fragment";
            bgfx::setName(fragHandle, fragName.c_str());
            _handles[shader.defines] = bgfx::createProgram(vertHandle, fragHandle, true);
        }
	}

    Program::Program(const Handles& handles, const bgfx::VertexLayout& layout) noexcept
		: _handles(handles)
		, _layout(layout)
	{
	}

    Program::~Program() noexcept
    {
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

    ShaderCompiler& ShaderCompiler::setShadercPath(const std::filesystem::path& path) noexcept
    {
        _shadercPath = path;
        return *this;
    }

    ShaderCompiler& ShaderCompiler::setIncludePaths(const std::vector<std::filesystem::path>& paths) noexcept
    {
        _includes = paths;
        return *this;
    }

    ShaderCompiler& ShaderCompiler::addIncludePath(const std::filesystem::path& path) noexcept
    {
        _includes.push_back(path);
        return *this;
    }

    ShaderCompiler& ShaderCompiler::setVaryingDef(const std::filesystem::path& path) noexcept
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

    int ShaderCompiler::operator()(const std::filesystem::path& input, const Output& output) const
    {
        auto varyingDef = _varyingDef;
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
        args.push_back("-i");
        args.push_back(fixPathArgument(input.parent_path()));

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

    std::vector<std::filesystem::path> ShaderCompiler::getDependencies(std::istream& in) const noexcept
    {
        std::vector<fs::path> deps;
        std::string line;
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

    const std::string ShaderCompiler::_binExt = ".bin";
    const std::string ShaderCompiler::_enableDefineSuffix = "_enabled";

    ShaderType ShaderCompiler::getShaderType(const std::filesystem::path& path) const noexcept
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

    std::vector<ShaderCompiler::Output> ShaderCompiler::getOutputs(const std::filesystem::path& basePath) const noexcept
    {
        auto defineCombs = CollectionUtils::combinations(_defines);
        std::vector<Output> outputs;
        for (auto& profile : _profiles)
        {
            auto profileExt = profile;
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
    const std::string ShaderImporterImpl::_configVaryingDefKey = "varyingdef";
    const std::string ShaderImporterImpl::_configIncludeDirsKey = "includeDirs";

    bool ShaderImporterImpl::startImport(const Input& input, bool dry)
    {
        if (input.config.is_null())
        {
            return false;
        }
        
        _compiler.setDefines({});
        _compiler.setIncludePaths(_includes);
        _compiler.setVaryingDef("");
        _compiler.setShaderType(ShaderType::Unknown);

        if (input.config.contains("defines"))
        {
            _compiler.setDefines(input.config["defines"]);
        }

        if (input.config.contains(_configIncludeDirsKey))
        {
            for (auto& elm : input.config[_configIncludeDirsKey])
            {
                _compiler.addIncludePath(input.basePath / elm.get<std::string>());
            }
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

    std::vector<fs::path> ShaderImporterImpl::getDependencies(const Input& input)
    {
        std::vector<fs::path> deps;
        if (!fs::exists(input.path))
        {
            return deps;
        }
        std::ifstream is(input.path);
        return _compiler.getDependencies(is);
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

        auto output = _outputs[outputIndex];
        output.path = fs::temp_directory_path() / fs::path(std::tmpnam(nullptr));
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

    void ShaderImporter::endImport(const Input& input)
    {
        _impl->endImport(input);
    }

    void ShaderImportDefinition::read(const nlohmann::json& json, std::filesystem::path basePath)
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

    void ProgramImportDefinition::loadDefault(const std::filesystem::path& path)
    {
        auto basePath = path.parent_path();
        auto fileName = path.filename();
        auto stem = StringUtils::getFileStem(fileName.string());
        varyingDefPath = basePath / (stem + ".varyingdef");
        vertexShader.path = basePath / (stem + ".vertex.sc");
        fragmentShader.path = basePath / (stem + ".fragment.sc");
        vertexLayoutPath = basePath / (stem + ".vlayout.json");
    }

    void ProgramImportDefinition::read(const nlohmann::ordered_json& json, std::filesystem::path basePath)
    {
        if (json.contains("vertexShader"))
        {
            vertexShader.read(json["vertexShader"], basePath);
        }
        if (json.contains("fragmentShader"))
        {
            fragmentShader.read(json["fragmentShader"], basePath);
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

    bool ProgramImporterImpl::startImport(const Input& input, bool dry)
    {
        if (input.config.is_null())
        {
            return false;
        }
        auto& def = _def.emplace();

        def.loadDefault(input.path);
        if (input.config.is_object())
        {
            def.read(input.config, input.basePath);
        }
        if (fs::exists(input.path))
        {
            std::ifstream is(input.path);
            auto fileJson = nlohmann::ordered_json::parse(is);
            def.read(fileJson, input.basePath);
        }
        if (!def.vertexLayoutPath.empty())
        {
            VertexLayoutUtils::readFile(def.vertexLayoutPath, def.vertexLayout);
        }
        return true;
    }

    std::vector<std::filesystem::path> ProgramImporterImpl::getOutputs(const Input& input)
    {
        std::vector<std::filesystem::path> outputs;
        auto path = input.getRelativePath();
        if (input.config.contains("outputPath"))
        {
            path = input.config["outputPath"].get<fs::path>();
        }
        else
        {
            path = path.stem().string() + ".bin";
        }
        outputs.push_back(path);
        return outputs;
    }

    std::vector<std::filesystem::path> ProgramImporterImpl::getDependencies(const Input& input)
    {
        std::vector<std::filesystem::path> deps;
        if (!_def)
        {
            return deps;
        }
        if (!_def->vertexShader.path.empty())
        {
            deps.push_back(_def->vertexShader.path);
        }
        if (!_def->fragmentShader.path.empty())
        {
            deps.push_back(_def->fragmentShader.path);
        }
        if (!_def->vertexLayoutPath.empty())
        {
            deps.push_back(_def->vertexLayoutPath);
        }
        return deps;
    }

    void ProgramImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
    }

    void ProgramImporterImpl::endImport(const Input& input)
    {
        _def.reset();
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

    bool ProgramImporter::startImport(const Input& input, bool dry)
    {
        return _impl->startImport(input, dry);
    }

    std::vector<std::filesystem::path> ProgramImporter::getOutputs(const Input& input)
    {
        return _impl->getOutputs(input);
    }

    std::vector<std::filesystem::path> ProgramImporter::getDependencies(const Input& input)
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