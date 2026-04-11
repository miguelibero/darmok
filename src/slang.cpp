#include <darmok/slang.hpp>
#include <darmok/stream.hpp>
#include "detail/program_core.hpp"
#include "detail/slang.hpp"
#include <fmt/format.h>
#include <magic_enum/magic_enum_format.hpp>
#include <bx/bx.h>

#define SLANG_TRY(desc, failable)                             \
    do {                                                      \
        auto result = (failable);                             \
        if (SLANG_FAILED(result))                             \
            return darmok::unexpected{ std::string{desc} };   \
    } while (false)

namespace darmok
{
    void SlangProgramCompilerConfig::read(const nlohmann::json& json, const ReadConfig& config) noexcept
    {
        auto fixPath = [&config](std::filesystem::path path)
            {
                if (path.string().starts_with("/"))
                {
                    return config.rootPath / path.relative_path();
                }
                return config.basePath / path;
            };
        auto itr = json.find("includeDirs");
        if (itr != json.end())
        {
            for (std::filesystem::path path : *itr)
            {
                includePaths.insert(fixPath(path));
            }
        }
        if (!config.rootPath.empty())
        {
            includePaths.insert(config.rootPath);
        }
        if (!config.basePath.empty())
        {
            includePaths.insert(config.basePath);
        }
    }

    const SlangProgramCompilerImpl::TargetProfileMap SlangProgramCompilerImpl::_targetProfileMap
    {
        {SlangCompileTarget::SLANG_DXBC, "sm_5_0"}, 
        // {SlangCompileTarget::SLANG_DXIL, "sm_6_0"}, // -disable-payload-qualifiers option error
        {SlangCompileTarget::SLANG_METAL, "metallib_2_4"},
        {SlangCompileTarget::SLANG_GLSL, "glsl_330"},
        {SlangCompileTarget::SLANG_SPIRV, "spirv_1_3"},
    };

    const std::vector<SlangCompileTarget> SlangProgramCompilerImpl::_supportedTargets
    {
#if BX_PLATFORM_WINDOWS
            SlangCompileTarget::SLANG_DXBC, SlangCompileTarget::SLANG_DXIL,
#elif BX_PLATFORM_OSX
           SlangCompileTarget::SLANG_METAL,
#endif
            SlangCompileTarget::SLANG_SPIRV, SlangCompileTarget::SLANG_GLSL,
    };

    const SlangProgramCompilerImpl::TargetRendererMap SlangProgramCompilerImpl::_targetRenderers
    {
        {SlangCompileTarget::SLANG_DXBC, bgfx::RendererType::Direct3D11},
        {SlangCompileTarget::SLANG_DXIL, bgfx::RendererType::Direct3D12},
        {SlangCompileTarget::SLANG_METAL, bgfx::RendererType::Metal},
        {SlangCompileTarget::SLANG_GLSL, bgfx::RendererType::OpenGL},
        {SlangCompileTarget::SLANG_SPIRV, bgfx::RendererType::Vulkan},
    };

    SlangProgramCompilerImpl::SlangProgramCompilerImpl(Slang::ComPtr<slang::IGlobalSession> globalSession, const Config& config) noexcept
        : _globalSession{ std::move(globalSession) }
        , _config{ config }
		, _searchPathStrings(config.includePaths.size())
        , _searchPathChars(config.includePaths.size())
    {
        std::transform(config.includePaths.begin(), config.includePaths.end(),
            _searchPathStrings.begin(), [](const std::filesystem::path& p) { return p.string(); });
        std::transform(_searchPathStrings.begin(), _searchPathStrings.end(),
            _searchPathChars.begin(), [](const std::string& str) { return str.c_str(); });

        _sessionDesc.searchPaths = &_searchPathChars.front();
        _sessionDesc.searchPathCount = _searchPathChars.size();
    }

    expected<SlangProgramCompilerImpl, std::string> SlangProgramCompilerImpl::create(const Config& config) noexcept
    {
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        SlangGlobalSessionDesc globalDesc = {};
        SLANG_TRY("creating global slang session",
            slang::createGlobalSession(&globalDesc, globalSession.writeRef()));

		return SlangProgramCompilerImpl{ std::move(globalSession), config };
    }

    void SlangProgramCompilerImpl::updateShader(protobuf::Shader& shader, const std::unordered_set<std::string>& defines, slang::IBlob& data) noexcept
    {
        shader.mutable_defines()->Clear();
        for (auto& define : defines)
        {
            *shader.add_defines() = define;
        }
        shader.set_data(DataView{ data.getBufferPointer(), data.getBufferSize() }.toString());
    }

    expected<protobuf::Program, std::string> SlangProgramCompilerImpl::operator()(const Source& src) noexcept
    {
        ShaderParser shaderParser{ _config.includePaths };
        ShaderParser::Defines defines;
        std::istringstream in{ src.data() };
        shaderParser.getDefines(in, defines);

        protobuf::Program program;
		program.set_name(src.name());
        ProgramDefinitionWrapper progWrap{ program };

        Slang::ComPtr<slang::ISession> session;
        SLANG_TRY("creating slang session",
            _globalSession->createSession(_sessionDesc, session.writeRef()));


        for (auto& defineComb : CollectionUtils::combinations(defines))
        {
            Slang::ComPtr<slang::ICompileRequest> request;
            SLANG_TRY("creating slang compile request",
                session->createCompileRequest(request.writeRef()));

			std::unordered_map<SlangCompileTarget, SlangInt> targetIdxMap;
            for (auto& target : _supportedTargets)
            {
				auto itr = _targetProfileMap.find(target);
				if (itr != _targetProfileMap.end())
                {
                    auto targetIdx = request->addCodeGenTarget(target);
                    auto profile = _globalSession->findProfile(itr->second.c_str());
					if(profile != SLANG_PROFILE_UNKNOWN)
                    {
					    request->setTargetProfile(targetIdx, profile);                   
                    }
                    targetIdxMap.emplace(target, targetIdx);
                }
            }
            auto path = src.name() + ".slang";
            auto transUnitIdx = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, path.c_str());
            request->addTranslationUnitSourceString(transUnitIdx, path.c_str(), src.data().c_str());

            for (auto& name : defineComb)
            {
                request->addPreprocessorDefine(name.c_str(), "1");
            }

            auto vertIdx = request->addEntryPoint(transUnitIdx, "vert", SlangStage::SLANG_STAGE_VERTEX);
            auto fragIdx = request->addEntryPoint(transUnitIdx, "frag", SlangStage::SLANG_STAGE_FRAGMENT);
            
            if (SLANG_FAILED(request->compile()))
            {
                return unexpected<std::string>{ request->getDiagnosticOutput() };
            }
           
            auto vertexLayoutResult = createVertexLayout(*request, vertIdx);
			if (!vertexLayoutResult)
            {
                return unexpected<std::string>{ "failed to create vertex layout: " + vertexLayoutResult.error() };
            }
			*program.mutable_varying()->mutable_vertex() = std::move(vertexLayoutResult).value();

            auto fragmentLayoutResult = createFragmentLayout(*request, fragIdx);
            if (!fragmentLayoutResult)
            {
                return unexpected<std::string>{ "failed to create fragment layout: " + fragmentLayoutResult.error() };
            }
            *program.mutable_varying()->mutable_fragment() = std::move(fragmentLayoutResult).value();

            for (auto& [target, targetIdx] : targetIdxMap)
            {
                auto itr = _targetRenderers.find(target);
                if (itr == _targetRenderers.end())
                {
                    continue;
                }
                auto renderer = itr->second;
                auto& programRenderer = progWrap.getRendererProgram(renderer);

                Slang::ComPtr<slang::IBlob> compiledBlob;
                if (SLANG_FAILED(request->getEntryPointCodeBlob(vertIdx, targetIdx, compiledBlob.writeRef())))
                {
                    return unexpected<std::string>{ fmt::format("failed to read compiled vertex shader {}", renderer) };
                }
                updateShader(*programRenderer.add_vertex_shaders(), defineComb, *compiledBlob);
                if (SLANG_FAILED(request->getEntryPointCodeBlob(fragIdx, targetIdx, compiledBlob.writeRef())))
                {
                    return unexpected<std::string>{ fmt::format("failed to read compiled fragment shader {}", renderer) };
                }
                updateShader(*programRenderer.add_fragment_shaders(), defineComb, *compiledBlob);
            }
        }

        return program;
    }

    std::optional<protobuf::Bgfx::Attrib> SlangProgramCompilerImpl::getBgfxAttrib(std::string_view semanticName, size_t semanticIndex) noexcept
    {
        auto consecutive = [semanticIndex](protobuf::Bgfx::Attrib base, int max) -> std::optional<protobuf::Bgfx::Attrib>
        {
            if(semanticIndex >= max)
            {
				return std::nullopt;
            }
            return static_cast<protobuf::Bgfx::Attrib>(static_cast<int>(base) + semanticIndex);
        };
        if (semanticName == "POSITION" || semanticName == "SV_POSITION")
        {
            return protobuf::Bgfx::Position;
        }
        if (semanticName == "NORMAL")
        {
            return protobuf::Bgfx::Normal;
        }
        if (semanticName == "TANGENT")
        {
            return protobuf::Bgfx::Tangent;
        }
        if (semanticName == "BITANGENT")
        {
            return protobuf::Bgfx::Bitangent;
        }
        if (semanticName == "TEXCOORD")
        {
            return consecutive(protobuf::Bgfx::TexCoord0, 8);
        }
        if (semanticName == "COLOR")
        {
            return consecutive(protobuf::Bgfx::Color0, 4);
        }
        if (semanticName == "INDICES")
        {
            return protobuf::Bgfx::Indices;
        }
        if (semanticName == "WEIGHT")
        {
            return protobuf::Bgfx::Weight;
        }
        return std::nullopt;
    }

    std::optional<protobuf::Bgfx::AttribType> SlangProgramCompilerImpl::getBgfxAttribType(slang::TypeReflection::ScalarType scalarType) noexcept
    {
        switch (scalarType)
        {
        case SLANG_SCALAR_TYPE_UINT8:
            return protobuf::Bgfx::Uint8;
        case SLANG_SCALAR_TYPE_INT16:
            return protobuf::Bgfx::Int16;
        case SLANG_SCALAR_TYPE_FLOAT16:
            return protobuf::Bgfx::Half;
        case SLANG_SCALAR_TYPE_FLOAT32:
            return protobuf::Bgfx::Float;
        }
        return std::nullopt;
    }

    expected<slang::TypeLayoutReflection*, std::string> SlangProgramCompilerImpl::getStructParamLayout(slang::ICompileRequest& request, SlangUInt entryPointIdx) noexcept
    {
        Slang::ComPtr<slang::IComponentType> comp;
        if (SLANG_FAILED(request.getEntryPoint(entryPointIdx, comp.writeRef())))
        {
            return unexpected<std::string>{ "failed to get compiled entry point" };
        }

        if (comp->getLayout()->getEntryPointCount() == 0)
        {
            return unexpected<std::string>{ "missing vertex entry point" };
        }

        auto entryPoint = comp->getLayout()->getEntryPointByIndex(0);
        if (entryPoint->getParameterCount() == 0)
        {
            return unexpected<std::string>{ "vertex entry point without parameters" };
        }

        auto param = entryPoint->getParameterByIndex(0);
        auto typeLayout = param->getTypeLayout();
        if (typeLayout->getType()->getKind() != slang::TypeReflection::Kind::Struct)
        {
            return unexpected<std::string>{ "vertex entry point param is not a struct" };
        }
		return typeLayout;
    }

    expected<protobuf::VertexLayout, std::string> SlangProgramCompilerImpl::createVertexLayout(slang::ICompileRequest& request, SlangUInt vertIdx) noexcept
    {
		auto typeLayoutResult = getStructParamLayout(request, vertIdx);
        if(!typeLayoutResult)
        {
            return unexpected<std::string>{ std::move(typeLayoutResult).error() };
		}
		auto typeLayout = typeLayoutResult.value();
        protobuf::VertexLayout vertexLayout;

        SlangInt fieldCount = typeLayout->getFieldCount();
        for (SlangInt f = 0; f < fieldCount; ++f)
        {
            auto& vertexAttrib = *vertexLayout.add_attributes();

            auto field = typeLayout->getFieldByIndex(f);
            auto fieldType = field->getTypeLayout()->getType();

            auto bgfxAttrib = getBgfxAttrib(field->getSemanticName(), field->getSemanticIndex());
            if (!bgfxAttrib)
            {
                return unexpected<std::string>{ fmt::format("unsupported attrib {} in vertex field {}", field->getSemanticName(), field->getName()) };
            }
            auto bgfxAttribType = getBgfxAttribType(fieldType->getScalarType());
            if (!bgfxAttribType)
            {
                return unexpected<std::string>{ fmt::format("unsupported attrib type {} in vertex field {}", fieldType->getScalarType(), field->getName()) };
            }
            vertexAttrib.set_bgfx_type(*bgfxAttribType);
            vertexAttrib.set_num(fieldType->getElementCount());
        }

        return vertexLayout;
    }

    expected<protobuf::FragmentLayout, std::string> SlangProgramCompilerImpl::createFragmentLayout(slang::ICompileRequest& request, SlangUInt fragIdx) noexcept
    {
        auto typeLayoutResult = getStructParamLayout(request, fragIdx);
        if (!typeLayoutResult)
        {
            return unexpected<std::string>{ std::move(typeLayoutResult).error() };
        }
        auto typeLayout = typeLayoutResult.value();

        protobuf::FragmentLayout fragmentLayout;

        SlangInt fieldCount = typeLayout->getFieldCount();
        for (SlangInt f = 0; f < fieldCount; ++f)
        {
            auto& fragAttrib = *fragmentLayout.add_attributes();

            auto field = typeLayout->getFieldByIndex(f);
            auto fieldType = field->getTypeLayout()->getType();

            auto bgfxAttrib = getBgfxAttrib(field->getSemanticName(), field->getSemanticIndex());
            if (!bgfxAttrib)
            {
                return unexpected<std::string>{ fmt::format("unsupported attrib {} in vertex field {}", field->getSemanticName(), field->getName()) };
            }
            fragAttrib.set_num(fieldType->getElementCount());
            fragAttrib.set_name(field->getName());
        }

        return fragmentLayout;
    }


    SlangProgramCompiler::SlangProgramCompiler(const Config& config) noexcept
        : _config{config}
    {
    }

    SlangProgramCompiler::~SlangProgramCompiler() noexcept = default;

    expected<protobuf::Program, std::string> SlangProgramCompiler::operator()(const Source& src) noexcept
    {
        if (!_impl)
        {
			auto result = SlangProgramCompilerImpl::create(_config);
            if (!result)
            {
				return unexpected{ std::move(result).error() };
            }
			_impl = std::make_unique<SlangProgramCompilerImpl>(std::move(result).value());
        }
		return (*_impl)(src);
    }

    void SlangProgramFileImporterImpl::addIncludePath(const std::filesystem::path& path) noexcept
    {
		_defaultConfig.includePaths.insert(path);
    }

    expected<void, std::string> SlangProgramFileImporterImpl::init(OptionalRef<std::ostream> log) noexcept
    {
        _defaultConfig.log = log;
        return {};
    }

    expected<SlangProgramFileImporterImpl::Effect, std::string> SlangProgramFileImporterImpl::prepare(const Input& input) noexcept
    {
        Effect effect;
        if (input.config.is_null())
        {
            auto filename = input.path.filename().string();
            auto ext = filename.substr(filename.find('.'));
            if (ext != ".slang")
            {
                return effect;
            }
        }

        _config = _defaultConfig;
        SlangProgramCompilerConfig::ReadConfig readConfig
        {
            .rootPath = input.basePath,
            .basePath = input.basePath / input.getRelativePath().parent_path()
        };

        _config->read(input.dirConfig, readConfig);
        _config->read(input.config, readConfig);

        _src.emplace();
		_src->set_name(input.getRelativePath().stem().string());
        auto readResult = StreamUtils::readString(input.path);
        if(!readResult)
        {
            return unexpected{ std::move(readResult).error() };
		}
		_src->set_data(std::move(readResult).value());

        effect.outputs.emplace_back(input.getOutputPath(protobuf::getExtension()), true);

        ShaderParser parser{ _config ? _config->includePaths : ShaderParser::IncludePaths{} };
        {
            std::istringstream in{ _src->data() };
            parser.getDependencies(in, effect.dependencies);
        }

        return effect;
    }

    expected<void, std::string> SlangProgramFileImporterImpl::operator()(const Input& input, ImportConfig& config) noexcept
    {
        if (!_src || !_config)
        {
            return {};
        }

        SlangProgramCompiler compiler{ _config.value() };
        auto compileResult = compiler(_src.value());
        if (!compileResult)
        {
            return unexpected{ "failed to compile program: " + compileResult.error() };
        }
        auto& def = compileResult.value();
        auto format = protobuf::getPathFormat(input.getOutputPath());

        for (auto& optOut : config.outputStreams)
        {
            if (!optOut)
            {
                continue;
            }
            auto writeResult = protobuf::write(def, *optOut, format);
            if (!writeResult)
            {
                return unexpected{ "failed to write program file" };
            }
        }
        return {};
    }

    const std::string& SlangProgramFileImporterImpl::getName() const noexcept
    {
        static const std::string name{ "slang" };
        return name;
    }

    SlangProgramFileImporter::SlangProgramFileImporter() noexcept
        : _impl{ std::make_unique<SlangProgramFileImporterImpl>() }
    {
    }

    SlangProgramFileImporter::~SlangProgramFileImporter() noexcept = default;

    const std::string& SlangProgramFileImporter::getName() const noexcept
    {
		return _impl->getName();
    }

    SlangProgramFileImporter& SlangProgramFileImporter::addIncludePath(const std::filesystem::path& path) noexcept
    {
		_impl->addIncludePath(path);
        return *this;
    }

    expected<void, std::string> SlangProgramFileImporter::init(OptionalRef<std::ostream> log) noexcept
    {
		return _impl->init(log);
    }

    expected<SlangProgramFileImporter::Effect, std::string> SlangProgramFileImporter::prepare(const Input& input) noexcept
    {
		return _impl->prepare(input);
    }

    expected<void, std::string> SlangProgramFileImporter::operator()(const Input& input, Config& config) noexcept
    {
		return (*_impl)(input, config);
    }
}