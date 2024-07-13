#include <darmok/program.hpp>
#include "program.hpp"
#include "embedded_shader.hpp"
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/vertex.hpp>
#include <darmok/vertex_layout.hpp>
#include <darmok/stream.hpp>
#include <darmok/utils.hpp>
#include <darmok/string.hpp>
#include <nlohmann/json.hpp>

namespace darmok
{
    namespace fs = std::filesystem;

	Program::Program(const std::string& name, const bgfx::EmbeddedShader* embeddedShaders, const DataView& vertexLayout)
		: _handle{ bgfx::kInvalidHandle }
	{
		auto renderer = bgfx::getRendererType();
		auto vertName = name + "_vertex";
		auto vertHandle = bgfx::createEmbeddedShader(embeddedShaders, renderer, vertName.c_str());
		if (!isValid(vertHandle))
		{
			throw std::runtime_error("could not load embedded vertex shader" + vertName);
		}
		auto fragName = name + "_fragment";
		auto fragHandle = bgfx::createEmbeddedShader(embeddedShaders, renderer, fragName.c_str());
		if (!isValid(fragHandle))
		{
			throw std::runtime_error("could not load embedded fragment shader" + fragName);
		}
		DataInputStream::read(vertexLayout, _layout);
		_handle = bgfx::createProgram(vertHandle, fragHandle, true);
	}

    Program::Program(const bgfx::ProgramHandle& handle, const bgfx::VertexLayout& layout) noexcept
		: _handle(handle)
		, _layout(layout)
	{
	}

    Program::~Program() noexcept
    {
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
    }

	const bgfx::ProgramHandle& Program::getHandle() const noexcept
	{
		return _handle;
	}

	const bgfx::VertexLayout& Program::getVertexLayout() const noexcept
	{
		return _layout;
	}

	const DataProgramLoader::Suffixes& DataProgramLoader::getDefaultSuffixes() noexcept
	{
		static const DataProgramLoader::Suffixes suffixes{ "_vertex", "_fragment", "_vertex_layout" };
		return suffixes;
	}

	DataProgramLoader::DataProgramLoader(IDataLoader& dataLoader, IVertexLayoutLoader& vertexLayoutLoader, Suffixes suffixes) noexcept
		: _dataLoader(dataLoader)
		, _vertexLayoutLoader(vertexLayoutLoader)
		, _suffixes(suffixes)
	{
	}

	static std::string getShaderExt()
	{
		switch (bgfx::getRendererType())
		{
		case bgfx::RendererType::Noop:
			return "";
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12:
			return ".dx11";
		case bgfx::RendererType::Agc:
		case bgfx::RendererType::Gnm:
			return ".pssl";
		case bgfx::RendererType::Metal:
			return ".metal";
		case bgfx::RendererType::Nvn:
			return ".nvn";
		case bgfx::RendererType::OpenGL:
			return ".glsl";
		case bgfx::RendererType::OpenGLES:
			return ".essl";
		case bgfx::RendererType::Vulkan:
			return ".spv";
		}
		throw std::runtime_error("unknown renderer type");
	}

	bgfx::ShaderHandle DataProgramLoader::loadShader(const std::string& filePath)
	{
		std::string dataName = filePath + getShaderExt() + ".bin";
		auto data = _dataLoader(dataName);
		if (data.empty())
		{
			throw std::runtime_error("got empty data");
		}
		bgfx::ShaderHandle handle = bgfx::createShader(data.makeRef());
		bgfx::setName(handle, filePath.c_str());
		return handle;
	}

	bgfx::VertexLayout DataProgramLoader::loadVertexLayout(std::string_view name)
	{
		return _vertexLayoutLoader(std::string(name) + _suffixes.vertexLayout);
	}

	std::shared_ptr<Program> DataProgramLoader::operator()(std::string_view name)
	{
		std::string nameStr(name);
		const bgfx::ShaderHandle vsh = loadShader(nameStr + _suffixes.vertex);
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (!name.empty())
		{
			fsh = loadShader(nameStr + _suffixes.fragment);
		}
		auto handle = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
		auto layout = loadVertexLayout(name);
		return std::make_shared<Program>(handle, layout);
	}

    std::shared_ptr<Program> ProgramGroup::getProgram(const Defines& defines) const noexcept
    {
        auto itr = _programs.find(defines);
        if (itr == _programs.end())
        {
            return nullptr;
        }
        return itr->second;
    }

    void ProgramGroup::setProgram(const std::shared_ptr<Program>& prog, const Defines& defines) noexcept
    {
        _programs[defines] = prog;
    }

    void ProgramGroup::read(const nlohmann::json& json, IProgramLoader& prog) noexcept
    {
        for (auto& [key, val] : json.items())
        {
        }
    }

    JsonProgramGroupLoader::JsonProgramGroupLoader(IDataLoader& dataLoader, IProgramLoader& programLoader, const std::string& manifestSuffix) noexcept
        : _dataLoader(dataLoader)
        , _progLoader(programLoader)
        , _manifestSuffix(manifestSuffix)
    {
    }

    std::shared_ptr<ProgramGroup> JsonProgramGroupLoader::operator()(std::string_view name)
    {
        auto manifestName = std::string(name) + _manifestSuffix;
        auto data = _dataLoader(manifestName);
        if (data.empty())
        {
            return nullptr;
        }
        auto group = std::make_shared<ProgramGroup>();
        auto json = nlohmann::json::parse(data.stringView());
        group->read(json, _progLoader);
        return group;
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

    const std::string ShaderImporterImpl::_enableDefineSuffix = "_enabled";
    const std::string ShaderImporterImpl::_manifestFileSuffix = ".manifest.json";

    fs::path ShaderImporterImpl::getOutputPath(const fs::path& path, const std::string& profileExt, const Defines& defines) noexcept
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
        auto ext = StringUtils::getFileExt(path.string());
        return path.parent_path() / (stem + suffix + ext + profileExt + _binExt);
    }

    std::vector<ShaderImporterImpl::Defines> ShaderImporterImpl::getDefineCombinations(const Input& input) const noexcept
    {
        Defines defines;
        if (input.config.contains("defines"))
        {
            defines = input.config["defines"];
        }
        return CollectionUtils::combinations(defines);
    }

    std::vector<fs::path> ShaderImporterImpl::getOutputs(const Input& input)
    {
        _outputConfigs.clear();
        std::vector<fs::path> outputs;
        auto fileName = input.path.filename().string();
        if (input.config.is_null())
        {
            auto ext = StringUtils::getFileExt(fileName);
            if (ext != ".fragment.sc" && ext != ".vertex.sc")
            {
                return outputs;
            }
        }
        auto basePath = input.getRelativePath();
        // manifest file
        outputs.push_back(basePath.stem().string() + _manifestFileSuffix);

        if (input.config.contains("outputPath"))
        {
            basePath = input.config["outputPath"].get<fs::path>();
        }
        auto defcombs = getDefineCombinations(input);
        for (auto& profile : _profiles)
        {
            auto itr = _profileExtensions.find(profile);
            if (itr == _profileExtensions.end())
            {
                continue;
            }
            for (auto& defines : defcombs)
            {
                auto path = getOutputPath(basePath, itr->second, defines);
                outputs.push_back(path);
                _outputConfigs.emplace_back(path, profile, defines);
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
        if (outputIndex == 0)
        {
            // manifest file
            auto json = nlohmann::json::object();
            for (auto& config : _outputConfigs)
            {
                auto& configJson = json[config.path.string()];
                configJson["profile"] = config.profile;
                configJson["defines"] = config.defines;
            }
            out << json.dump(2);
            return;
        }

        outputIndex--;
        if (outputIndex < 0 || outputIndex >= _outputConfigs.size())
        {
            throw std::runtime_error("cannot find output config");
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

        auto& config = _outputConfigs[outputIndex];
        fs::path tmpPath = fs::temp_directory_path() / fs::path(std::tmpnam(nullptr));

        std::vector<std::string> args{
            fixPathArgument(_shadercPath),
            "-p", config.profile,
            "-f", fixPathArgument(input.path),
            "-o", fixPathArgument(tmpPath),
            "--type", type,
            "--varyingdef", fixPathArgument(varyingDef)
        };
        if (!config.defines.empty())
        {
            args.emplace_back("--define");
            args.emplace_back(StringUtils::join(",", config.defines.begin(), config.defines.end()));
        }
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
        StreamUtils::copy(is, out, _bufferSize);
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

}