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
    namespace SlangBgfxShaderUtils
    {
        constexpr uint8_t kUniformFragmentBit = 0x10;
        constexpr uint8_t kUniformReadOnlyBit = 0x40;

        constexpr uint16_t storageBufferDesrcriptor = 0x0007;
        constexpr uint16_t storageImageDescriptor = 0x0003;

        constexpr uint8_t version = 11;

        constexpr uint32_t composeMagic(char a, char b, char c, uint8_t ver)
        {
            return (static_cast<uint32_t>(a)) | (static_cast<uint32_t>(b) << 8) | (static_cast<uint32_t>(c) << 16) |
                   static_cast<uint32_t>(ver) << 24;
        }

        constexpr uint32_t magicCsh = composeMagic('C', 'S', 'H', version);
        constexpr uint32_t magicFsh = composeMagic('F', 'S', 'H', version);
        constexpr uint32_t magicVsh = composeMagic('V', 'S', 'H', version);

        constexpr uint32_t getMagic(SlangStage stage)
        {
            switch (stage) {
            case SLANG_STAGE_VERTEX:
                return magicVsh;
            case SLANG_STAGE_FRAGMENT:
                return magicFsh;
            case SLANG_STAGE_COMPUTE:
                return magicCsh;
            default:
                return 0;
            }
        }
    }

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
        {SlangCompileTarget::SLANG_DXIL, "sm_6_0"}, // -disable-payload-qualifiers option error
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

        _sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
        _sessionDesc.searchPaths = &_searchPathChars.front();
        _sessionDesc.searchPathCount = _searchPathChars.size();

        for (auto& target : _supportedTargets)
        {
            slang::TargetDesc targetDesc{ .format = target };
            auto itr = _targetProfileMap.find(target);
            if (itr != _targetProfileMap.end())
            {
                targetDesc.profile = _globalSession->findProfile(itr->second.c_str());
            }
            _targetDescs.push_back(targetDesc);
        }
        _sessionDesc.targetCount = _targetDescs.size();
        _sessionDesc.targets = _targetDescs.data();
    }

    expected<SlangProgramCompilerImpl, std::string> SlangProgramCompilerImpl::create(const Config& config) noexcept
    {
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        SlangGlobalSessionDesc globalDesc = {};
        SLANG_TRY("creating global slang session",
            slang::createGlobalSession(&globalDesc, globalSession.writeRef()));

		return SlangProgramCompilerImpl{ std::move(globalSession), config };
    }

    expected<void, std::string> SlangProgramCompilerImpl::updateShader(protobuf::Shader& shader, slang::IComponentType& linkedProgram, const std::unordered_set<std::string>& defines, SlangInt entryPointIdx, SlangInt targetIdx) noexcept
    {
        Slang::ComPtr<slang::IBlob> data;
        Slang::ComPtr<slang::IBlob> diagnostics;
        if (SLANG_FAILED(linkedProgram.getEntryPointCode(entryPointIdx, targetIdx, data.writeRef(), diagnostics.writeRef())))
        {
            return unexpected{ getDiagnosticsString(diagnostics) };
        }
        shader.mutable_defines()->Clear();
        for (auto& define : defines)
        {
            *shader.add_defines() = define;
        }
        auto* programLayout = linkedProgram.getLayout(targetIdx, diagnostics.writeRef());
        if (!programLayout)
        {
            return unexpected{ getDiagnosticsString(diagnostics) };
        }
        auto* entryPoint = programLayout->getEntryPointByIndex(entryPointIdx);
        auto stage = entryPoint->getStage();

        std::vector<LayoutParam> params;
        if (stage == SLANG_STAGE_VERTEX)
        {
            auto* varLayout = entryPoint->getResultVarLayout();
            if (!varLayout)
            {
                return unexpected<std::string>{"could not get vertex result layout"};
            }
            auto result = getLayoutParams(*varLayout);
            if (!result)
            {
                return unexpected{ fmt::format("could not get vertex result layout: {}", result.error()) };
            }
            params = std::move(result).value();
        }
        else if (stage == SLANG_STAGE_FRAGMENT)
        {
            for (int i = 0; i < entryPoint->getParameterCount(); i++)
            {
                auto *varLayout = entryPoint->getParameterByIndex(i);
                auto result = getLayoutParams(*varLayout);
                if (!result)
                {
                    return unexpected{ fmt::format("could not get fragment param {} layout: {}", i, result.error()) };
                }
                params.insert(params.begin(), result->begin(), result->end());
            }
        }
        else
        {
            return unexpected{ fmt::format("unsupported entry point stage: {}", stage) };
        }

        std::ostringstream out;
        out << SlangBgfxShaderUtils::getMagic(stage);
        out << DataView{ data->getBufferPointer(), data->getBufferSize() }.toString();
        shader.set_data(out.str());

        return {};
    }

    std::string SlangProgramCompilerImpl::getDiagnosticsString(slang::IBlob* diagnostics) noexcept
    {
        if (!diagnostics)
        {
            return {};
        }
        return {static_cast<const char*>(diagnostics->getBufferPointer()), diagnostics->getBufferSize()};
    }

    expected<Slang::ComPtr<slang::ISession>, std::string> SlangProgramCompilerImpl::createSession(const std::unordered_set<std::string>& defines) noexcept
    {
        auto sessionDesc = _sessionDesc;
        std::vector<slang::PreprocessorMacroDesc> macros;
        for (auto& define : defines)
        {
            macros.emplace_back(define.c_str(), "1");
        }
        sessionDesc.preprocessorMacroCount = macros.size();
        sessionDesc.preprocessorMacros = macros.data();

        Slang::ComPtr<slang::ISession> session;

        SLANG_TRY("creating slang session",
            _globalSession->createSession(sessionDesc, session.writeRef()));

        return session;
    }

    expected<std::vector<SlangProgramCompilerImpl::LayoutParam>, std::string> SlangProgramCompilerImpl::getLayoutParams(slang::VariableLayoutReflection& layout, const std::string& prefix) noexcept
    {
        std::vector<LayoutParam> params;
        auto* typeLayout = layout.getTypeLayout();

        switch (typeLayout->getKind())
        {
        case slang::TypeReflection::Kind::Struct:
        {
            for (int i = 0; i < typeLayout->getFieldCount(); i++)
            {
                auto *field = typeLayout->getFieldByIndex(i);
                auto subprefix = layout.getName() != nullptr ? prefix + layout.getName() + "." : "";
                auto subparams = getLayoutParams(*field, subprefix);
                if (!subparams)
                {
                    return unexpected{fmt::format("failed in field {}: {}", subprefix, subparams.error())};
                }
                params.insert(params.end(), subparams->begin(), subparams->end());
            }
            return params;
        }
        case slang::TypeReflection::Kind::Vector:
        case slang::TypeReflection::Kind::Scalar:
        {
            if (layout.getSemanticName() == nullptr)
            {
                return unexpected{fmt::format("no semantic name specified for var: {}",layout.getName())};
            }
            auto attrib = getBgfxAttrib(layout.getSemanticName(), layout.getSemanticIndex());
            if (!attrib)
            {
                return unexpected{fmt::format("unsupported semantic name: {}",layout.getSemanticName())};
            }
            params.emplace_back(layout.getName(), prefix + layout.getName(), bgfx::Attrib::Enum(*attrib));
            return params;
        }
        default:
            return unexpected{fmt::format("Unsupported type of param: {}",layout.getName())};
        }
        return params;
    }

    uint32_t SlangProgramCompilerImpl::hashLayoutParams(const std::vector<LayoutParam> &params) noexcept
    {
        uint32_t hash = 0;
        std::vector<std::string> names(params.size());
        std::transform(params.begin(), params.end(), names.begin(), [](const auto& p) { return p.name; });
        std::sort(names.begin(), names.end());
        std::hash<std::string> hasher;
        for (const auto &name : names)
        {
            hash ^= hasher(name);
        }
        return hash;
    }

    expected<Slang::ComPtr<slang::IComponentType>, std::string> SlangProgramCompilerImpl::compileProgram(const Source& src, slang::ISession& session, OptionalRef<std::ostream> log) noexcept
    {
        auto addLog = [log](const std::string& title, const std::string& msg)
        {
            if (!log || msg.empty())
            {
                return;
            }
            *log << title << "\n";
            *log << ">>>\n";
            *log << msg << "\n";
            *log << ">>>\n";
        };

        auto path = src.name() + ".slang";
        Slang::ComPtr<slang::IBlob> diagnostics;
        Slang::ComPtr<slang::IModule> module{session.loadModuleFromSourceString(src.name().c_str(), path.c_str(), src.data().c_str(), diagnostics.writeRef())};
        std::string msg = getDiagnosticsString(diagnostics);
        if (module == nullptr)
        {
            return unexpected{msg};
        }
        addLog("loading program source", msg);

        std::vector<slang::IComponentType*> components;
        components.reserve(module->getDefinedEntryPointCount() + 1);
        components.push_back(module);
        for (SlangInt32 i = 0; i < module->getDefinedEntryPointCount(); i++)
        {
            Slang::ComPtr<slang::IEntryPoint> entryPoint;
            module->getDefinedEntryPoint(i, entryPoint.writeRef());
            components.push_back(entryPoint);
        }

        Slang::ComPtr<slang::IComponentType> composite;
        SlangResult result =
            session.createCompositeComponentType(components.data(), components.size(), composite.writeRef(), diagnostics.writeRef());
        msg = getDiagnosticsString(diagnostics);
        if (SLANG_FAILED(result))
        {
            return unexpected{msg};
        }
        addLog("creating composite component type", msg);

        Slang::ComPtr<slang::IComponentType> program;
        result = composite->link(program.writeRef(), diagnostics.writeRef());
        msg = getDiagnosticsString(diagnostics);
        if (SLANG_FAILED(result))
        {
            return unexpected{msg};
        }
        addLog("linking program", msg);
        return program;
    }

    expected<protobuf::Program, std::string> SlangProgramCompilerImpl::operator()(const Source& src) noexcept
    {
        ShaderParser::Defines defines;
        {
            ShaderParser shaderParser{ _config.includePaths };
            std::istringstream in{ src.data() };
            shaderParser.getDefines(in, defines);
        }

        protobuf::Program programDef;
        programDef.set_name(src.name());
        ProgramDefinitionWrapper progWrap{ programDef };

        for (auto& defineComb : CollectionUtils::combinations(defines))
        {
            auto sessionResult = createSession(defineComb);
            if (!sessionResult)
            {
                return unexpected{fmt::format("failed to create session: {}", sessionResult.error())};
            }
            auto session = sessionResult.value();
            auto compileResult = compileProgram(src, *session, _config.log);
            if (!compileResult)
            {
                return unexpected{fmt::format("failed to compile program: {}", compileResult.error())};
            }
            auto linkedProgram = std::move(compileResult).value();
            auto layout = linkedProgram->getLayout();

            SlangInt vertIdx = -1;
            SlangInt fragIdx = -1;
            for (SlangInt i = 0; i < layout->getEntryPointCount(); i++)
            {
                auto entryPoint = layout->getEntryPointByIndex(i);
                if (entryPoint->getStage() == SLANG_STAGE_VERTEX)
                {
                    auto result = createVertexLayout(*entryPoint);
                    if (!result)
                    {
                        return unexpected<std::string>{ "failed to create vertex layout: " + result.error() };
                    }
                    *programDef.mutable_varying()->mutable_vertex() = std::move(result).value();
                    vertIdx = i;
                }
                if (entryPoint->getStage() == SLANG_STAGE_FRAGMENT)
                {
                    auto result = createFragmentLayout(*entryPoint);
                    if (!result)
                    {
                        return unexpected<std::string>{ "failed to create fragment layout: " + result.error() };
                    }
                    *programDef.mutable_varying()->mutable_fragment() = std::move(result).value();
                    fragIdx = i;
                }
                if (vertIdx >= 0 && fragIdx >= 0)
                {
                    break;
                }
            }
            if (vertIdx < 0)
            {
                return unexpected<std::string>{ "missing vertex entry point" };
            }
            if (fragIdx < 0)
            {
                return unexpected<std::string>{ "missing fragment entry point" };
            }

            for (SlangInt targetIdx = 0; targetIdx < _supportedTargets.size(); targetIdx++)
            {
                auto itr = _targetRenderers.find(_supportedTargets.at(targetIdx));
                if (itr == _targetRenderers.end())
                {
                    continue;
                }
                auto renderer = itr->second;
                auto& programRenderer = progWrap.getRendererProgram(renderer);
                auto updateResult = updateShader(*programRenderer.add_vertex_shaders(), *linkedProgram, defineComb, vertIdx, targetIdx);
                if (!updateResult)
                {
                    return unexpected{fmt::format("failed to update vertex shader: {}", updateResult.error())};
                }
                updateResult = updateShader(*programRenderer.add_fragment_shaders(), *linkedProgram, defineComb, fragIdx, targetIdx);
                if (!updateResult)
                {
                    return unexpected{fmt::format("failed to update fragment shader: {}", updateResult.error())};
                }
            }
        }

        return programDef;
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
        case slang::TypeReflection::ScalarType::UInt8:
            return protobuf::Bgfx::Uint8;
        case slang::TypeReflection::ScalarType::Int16:
            return protobuf::Bgfx::Int16;
        case slang::TypeReflection::ScalarType::Float16:
            return protobuf::Bgfx::Half;
        case slang::TypeReflection::ScalarType::Float32:
            return protobuf::Bgfx::Float;
        default:
            return std::nullopt;
        }
    }

    expected<slang::TypeLayoutReflection*, std::string> SlangProgramCompilerImpl::getStructParamLayout(slang::EntryPointReflection& entryPoint) noexcept
    {
        if (entryPoint.getParameterCount() == 0)
        {
            return unexpected<std::string>{ "vertex entry point without parameters" };
        }
        auto param = entryPoint.getParameterByIndex(0);
        auto typeLayout = param->getTypeLayout();
        if (typeLayout->getType()->getKind() != slang::TypeReflection::Kind::Struct)
        {
            return unexpected<std::string>{ "vertex entry point param is not a struct" };
        }
		return typeLayout;
    }

    expected<protobuf::VertexLayout, std::string> SlangProgramCompilerImpl::createVertexLayout(slang::EntryPointReflection& entryPoint) noexcept
    {
        if (entryPoint.getStage() != SLANG_STAGE_VERTEX)
        {
            return unexpected<std::string>{ "entry point is not a vertex state" };
        }
		auto typeLayoutResult = getStructParamLayout(entryPoint);
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

    expected<protobuf::FragmentLayout, std::string> SlangProgramCompilerImpl::createFragmentLayout(slang::EntryPointReflection& entryPoint) noexcept
    {
        if (entryPoint.getStage() != SLANG_STAGE_FRAGMENT)
        {
            return unexpected<std::string>{ "entry point is not a fragment state" };
        }
        auto typeLayoutResult = getStructParamLayout(entryPoint);
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