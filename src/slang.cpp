#include "detail/slang.hpp"
#include "detail/glsl.hpp"
#include "detail/program_core.hpp"
#include <bx/bx.h>
#include <c++/16.1.1/algorithm>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/slang.hpp>
#include <darmok/stream.hpp>
#include <darmok/utils.hpp>
#include <fmt/format.h>
#include <magic_enum/magic_enum_format.hpp>

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
        constexpr uint8_t uniformFragmentBit = 0x10;
        constexpr uint8_t uniformReadOnlyBit = 0x40;

        constexpr uint16_t storageBufferDescriptor = 0x0007;
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

        std::string getDiagnosticsString(slang::IBlob *diagnostics) noexcept
        {
            if (!diagnostics)
            {
                return {};
            }
            return {static_cast<const char *>(diagnostics->getBufferPointer()), diagnostics->getBufferSize()};
        }

        expected<std::optional<protobuf::Bgfx::Attrib>, std::string> getBgfxAttrib(std::string_view semanticName, size_t semanticIndex) noexcept
        {
            auto consecutive = [semanticIndex](protobuf::Bgfx::Attrib base, int max) -> expected<std::optional<protobuf::Bgfx::Attrib>, std::string>
            {
                if (semanticIndex >= max)
                {
                    return unexpected{fmt::format("unsupported index {} for attrib type {}", semanticIndex, base)};
                }
                return static_cast<protobuf::Bgfx::Attrib>(static_cast<int>(base) + semanticIndex);
            };
            if (semanticName == "POSITION")
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
            if (semanticName.starts_with("SV_"))
            {
                return std::nullopt;
            }
            return unexpected{fmt::format("could not deduce attrib type {}", semanticName)};
        }

        std::optional<protobuf::Bgfx::AttribType> getBgfxAttribType(slang::TypeReflection::ScalarType scalarType) noexcept
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

        expected<slang::TypeLayoutReflection *, std::string> getStructParamLayout(slang::EntryPointReflection &entryPoint) noexcept
        {
            if (entryPoint.getParameterCount() == 0)
            {
                return unexpected<std::string>{"vertex entry point without parameters"};
            }
            auto param = entryPoint.getParameterByIndex(0);
            auto typeLayout = param->getTypeLayout();
            if (typeLayout->getType()->getKind() != slang::TypeReflection::Kind::Struct)
            {
                return unexpected<std::string>{"vertex entry point param is not a struct"};
            }
            return typeLayout;
        }

        expected<protobuf::VertexLayout, std::string> createVertexLayout(slang::EntryPointReflection &entryPoint) noexcept
        {
            if (entryPoint.getStage() != SLANG_STAGE_VERTEX)
            {
                return unexpected<std::string>{"entry point is not a vertex state"};
            }
            auto typeLayoutResult = getStructParamLayout(entryPoint);
            if (!typeLayoutResult)
            {
                return unexpected<std::string>{std::move(typeLayoutResult).error()};
            }
            auto typeLayout = typeLayoutResult.value();
            protobuf::VertexLayout vertexLayout;

            SlangInt fieldCount = typeLayout->getFieldCount();
            for (SlangInt f = 0; f < fieldCount; ++f)
            {

                auto field = typeLayout->getFieldByIndex(f);
                auto fieldType = field->getTypeLayout()->getType();

                auto bgfxAttribResult = getBgfxAttrib(field->getSemanticName(), field->getSemanticIndex());
                if (!bgfxAttribResult)
                {
                    return unexpected<std::string>{fmt::format("unsupported attrib {} in vertex field {}: {}", field->getSemanticName(), field->getName(), bgfxAttribResult.error())};
                }
                auto& bgfxAttrib = bgfxAttribResult.value();
                if (!bgfxAttrib)
                {
                    continue;
                }
                auto bgfxAttribType = getBgfxAttribType(fieldType->getScalarType());
                if (!bgfxAttribType)
                {
                    return unexpected<std::string>{fmt::format("unsupported attrib type {} in vertex field {}", fieldType->getScalarType(), field->getName())};
                }

                auto &vertexAttrib = *vertexLayout.add_attributes();
                vertexAttrib.set_bgfx(*bgfxAttrib);
                vertexAttrib.set_bgfx_type(*bgfxAttribType);
                vertexAttrib.set_num(fieldType->getElementCount());
            }

            return vertexLayout;
        }

        expected<protobuf::FragmentLayout, std::string> createFragmentLayout(slang::EntryPointReflection &entryPoint) noexcept
        {
            if (entryPoint.getStage() != SLANG_STAGE_FRAGMENT)
            {
                return unexpected<std::string>{"entry point is not a fragment state"};
            }
            auto typeLayoutResult = getStructParamLayout(entryPoint);
            if (!typeLayoutResult)
            {
                return unexpected<std::string>{std::move(typeLayoutResult).error()};
            }
            auto typeLayout = typeLayoutResult.value();

            protobuf::FragmentLayout fragmentLayout;

            SlangInt fieldCount = typeLayout->getFieldCount();
            for (SlangInt f = 0; f < fieldCount; ++f)
            {
                auto &fragAttrib = *fragmentLayout.add_attributes();

                auto field = typeLayout->getFieldByIndex(f);
                auto fieldType = field->getTypeLayout()->getType();

                auto bgfxAttrib = getBgfxAttrib(field->getSemanticName(), field->getSemanticIndex());
                if (!bgfxAttrib)
                {
                    return unexpected<std::string>{fmt::format("unsupported attrib {} in vertex field {}", field->getSemanticName(), field->getName())};
                }
                fragAttrib.set_num(fieldType->getElementCount());
                fragAttrib.set_name(field->getName());
            }

            return fragmentLayout;
        }

        using TargetProfileMap = std::unordered_map<SlangCompileTarget, std::string>;
        using RendererTargetMap = std::unordered_map<bgfx::RendererType::Enum, SlangCompileTarget>;

        const TargetProfileMap _targetProfileMap{
            {SlangCompileTarget::SLANG_DXBC, "sm_5_0"},
            {SlangCompileTarget::SLANG_DXIL, "sm_6_0"}, // -disable-payload-qualifiers option error
            {SlangCompileTarget::SLANG_METAL, "metallib_2_4"},
            {SlangCompileTarget::SLANG_SPIRV, "spirv_1_3"},
        };

        const std::vector<SlangCompileTarget> _supportedTargets{
#if BX_PLATFORM_WINDOWS
            SlangCompileTarget::SLANG_DXBC,
            SlangCompileTarget::SLANG_DXIL,
#elif BX_PLATFORM_OSX
            SlangCompileTarget::SLANG_METAL,
#endif
            SlangCompileTarget::SLANG_SPIRV,
        };

        const RendererTargetMap _rendererTargets{
            {
                bgfx::RendererType::Direct3D11,
                SlangCompileTarget::SLANG_DXBC,
            },
            {
                bgfx::RendererType::Direct3D12,
                SlangCompileTarget::SLANG_DXIL,
            },
            {
                bgfx::RendererType::Metal,
                SlangCompileTarget::SLANG_METAL,
            },
            {
                bgfx::RendererType::Vulkan,
                SlangCompileTarget::SLANG_SPIRV,
            },
            {
                bgfx::RendererType::OpenGL,
                SlangCompileTarget::SLANG_SPIRV,
            },
            {
                bgfx::RendererType::OpenGLES,
                SlangCompileTarget::SLANG_SPIRV,
            },
        };

        struct LayoutParam final
        {
            std::string name;
            std::string qualifiedName;
            bgfx::Attrib::Enum attrib;
        };

        enum class TextureComponentType : uint8_t
        {
            Float,
            Int,
            Uint,
            Depth,
            UnfilterableFloat,

            Unknown,
        };

        enum class TextureDimension : uint8_t
        {
            Dimension1D,
            Dimension2D,
            Dimension2DArray,
            DimensionCube,
            DimensionCubeArray,
            Dimension3D,

            Unknown,
        };

        struct Uniform final
        {
            std::string name;
            bgfx::UniformType::Enum type;
            uint8_t num;
            uint16_t regIndex;
            uint16_t regCount;
            TextureComponentType texComponent = TextureComponentType::Float;
            TextureDimension texDimension = TextureDimension::Dimension1D;
            bgfx::TextureFormat::Enum texFormat = bgfx::TextureFormat::BC1;
        };

        struct UniformData final
        {
            std::vector<Uniform> uniforms;
            uint16_t bufferSize = 0;
        };

        expected<bgfx::UniformType::Enum, std::string> convertUniformType(slang::TypeReflection& type, bool isCompute) noexcept
        {
            auto kind = type.getKind();
            switch (kind)
            {
                case slang::TypeReflection::Kind::Resource:
                {
                    if (isCompute || type.getResourceShape() == SlangResourceShape::SLANG_STRUCTURED_BUFFER)
                    {
                        if (type.getResourceAccess() == SlangResourceAccess::SLANG_RESOURCE_ACCESS_READ)
                        {
                            return static_cast<bgfx::UniformType::Enum>(uniformReadOnlyBit | static_cast<uint8_t>(bgfx::UniformType::Count));
                        }
                        return bgfx::UniformType::Count;
                    }
                    return bgfx::UniformType::Sampler;
                }
                case slang::TypeReflection::Kind::SamplerState:
                {
                    // skip sampler states
                    return bgfx::UniformType::Count;
                }
                case slang::TypeReflection::Kind::Vector:
                {
                    auto count = type.getElementCount();
                    if (count == 4)
                    {
                        return bgfx::UniformType::Vec4;
                    }
                    return unexpected{fmt::format("unsupported uniform vector count {}", count)};
                }
                case slang::TypeReflection::Kind::Matrix:
                {
                    auto count = type.getRowCount();
                    switch (count)
                    {
                    case 3:
                        return bgfx::UniformType::Mat3;
                    case 4:
                        return bgfx::UniformType::Mat4;
                    }
                    return unexpected{fmt::format("unsupported uniform matrix count {}", count)};
                }
            default:
                return unexpected{fmt::format("unsupported uniform kind {}", kind)};
            }
        }

        TextureComponentType convertTextureComponentType(slang::TypeReflection& type) noexcept
        {
            auto texType = type.getResourceResultType()->getScalarType();
            switch (texType)
            {
            case slang::TypeReflection::ScalarType::Float16:
            case slang::TypeReflection::ScalarType::Float32:
            case slang::TypeReflection::ScalarType::Float64:
                return TextureComponentType::Float;
            case slang::TypeReflection::ScalarType::Int8:
            case slang::TypeReflection::ScalarType::Int16:
            case slang::TypeReflection::ScalarType::Int32:
            case slang::TypeReflection::ScalarType::Int64:
                return TextureComponentType::Int;
            case slang::TypeReflection::ScalarType::UInt8:
            case slang::TypeReflection::ScalarType::UInt16:
            case slang::TypeReflection::ScalarType::UInt32:
            case slang::TypeReflection::ScalarType::UInt64:
                return TextureComponentType::Uint;
            default:
                return TextureComponentType::Float;
            }
        }

        TextureDimension convertTextureDimension(slang::TypeReflection& type) noexcept
        {
            auto shape = type.getResourceShape();
            switch (shape)
            {
            case SLANG_TEXTURE_1D:
                return TextureDimension::Dimension1D;
            case SLANG_TEXTURE_2D: // original bgfx compiler vulkan compiler reports simple BgfxSampler as array so we're doing same here
            case SLANG_TEXTURE_2D_ARRAY:
                return TextureDimension::Dimension2DArray;
            case SLANG_TEXTURE_CUBE:
                return TextureDimension::DimensionCube;
            case SLANG_TEXTURE_CUBE_ARRAY:
                return TextureDimension::DimensionCubeArray;
            case SLANG_TEXTURE_3D:
                return TextureDimension::Dimension3D;
            default:
                return TextureDimension::Unknown;
            }
        }

        ShaderType convertShaderType(SlangStage stage) noexcept
        {
            switch (stage)
            {
            case SLANG_STAGE_FRAGMENT:
                return ShaderType::Fragment;
            case SLANG_STAGE_VERTEX:
                return ShaderType::Vertex;
            case SLANG_STAGE_COMPUTE:
                return ShaderType::Compute;
            default:
                return ShaderType::Unknown;
            }
        }

        bgfx::TextureFormat::Enum convertTextureFormat(SlangImageFormat format) noexcept
        {
            switch (format)
            {
            case SLANG_IMAGE_FORMAT_unknown:
                return  bgfx::TextureFormat::Unknown;
            case SLANG_IMAGE_FORMAT_rgba32f:
                return  bgfx::TextureFormat::RGBA32F;
            case SLANG_IMAGE_FORMAT_rgba16f:
                return  bgfx::TextureFormat::RGBA16F;
            case SLANG_IMAGE_FORMAT_rg32f:
                return  bgfx::TextureFormat::RG32F;
            case SLANG_IMAGE_FORMAT_rg16f:
                return  bgfx::TextureFormat::RG16F;
            case SLANG_IMAGE_FORMAT_r11f_g11f_b10f:
                return  bgfx::TextureFormat::RG11B10F;
            case SLANG_IMAGE_FORMAT_r32f:
                return  bgfx::TextureFormat::R32F;
            case SLANG_IMAGE_FORMAT_r16f:
                return  bgfx::TextureFormat::R16F;
            case SLANG_IMAGE_FORMAT_rgba16:
                return  bgfx::TextureFormat::RGBA16;
            case SLANG_IMAGE_FORMAT_rgb10_a2:
                return  bgfx::TextureFormat::RGB10A2;
            case SLANG_IMAGE_FORMAT_rgba8:
                return  bgfx::TextureFormat::RGBA8;
            case SLANG_IMAGE_FORMAT_rg16:
                return  bgfx::TextureFormat::RG16;
            case SLANG_IMAGE_FORMAT_rg8:
                return  bgfx::TextureFormat::RG8;
            case SLANG_IMAGE_FORMAT_r16:
                return  bgfx::TextureFormat::R16;
            case SLANG_IMAGE_FORMAT_r8:
                return  bgfx::TextureFormat::R8;
            case SLANG_IMAGE_FORMAT_rgba16_snorm:
                return  bgfx::TextureFormat::RGBA16S;
            case SLANG_IMAGE_FORMAT_rgba8_snorm:
                return  bgfx::TextureFormat::RGBA8S;
            case SLANG_IMAGE_FORMAT_rg16_snorm:
                return  bgfx::TextureFormat::RG16S;
            case SLANG_IMAGE_FORMAT_rg8_snorm:
                return  bgfx::TextureFormat::RG8S;
            case SLANG_IMAGE_FORMAT_r16_snorm:
                return  bgfx::TextureFormat::R16S;
            case SLANG_IMAGE_FORMAT_r8_snorm:
                return  bgfx::TextureFormat::R8S;
            case SLANG_IMAGE_FORMAT_rgba32i:
                return  bgfx::TextureFormat::RGBA32I;
            case SLANG_IMAGE_FORMAT_rgba16i:
                return  bgfx::TextureFormat::RGBA16I;
            case SLANG_IMAGE_FORMAT_rgba8i:
                return  bgfx::TextureFormat::RGBA8I;
            case SLANG_IMAGE_FORMAT_rg32i:
                return  bgfx::TextureFormat::RG32I;
            case SLANG_IMAGE_FORMAT_rg16i:
                return  bgfx::TextureFormat::RG16I;
            case SLANG_IMAGE_FORMAT_rg8i:
                return  bgfx::TextureFormat::RG8I;
            case SLANG_IMAGE_FORMAT_r32i:
                return  bgfx::TextureFormat::R32I;
            case SLANG_IMAGE_FORMAT_r16i:
                return  bgfx::TextureFormat::R16I;
            case SLANG_IMAGE_FORMAT_r8i:
                return  bgfx::TextureFormat::R8I;
            case SLANG_IMAGE_FORMAT_rgba32ui:
                return  bgfx::TextureFormat::RGBA32U;
            case SLANG_IMAGE_FORMAT_rgba16ui:
                return  bgfx::TextureFormat::RGBA16U;
            case SLANG_IMAGE_FORMAT_rgb10_a2ui:
                return  bgfx::TextureFormat::RGB10A2;
            case SLANG_IMAGE_FORMAT_rgba8ui:
                return  bgfx::TextureFormat::RGBA8U;
            case SLANG_IMAGE_FORMAT_rg32ui:
                return  bgfx::TextureFormat::RG32U;
            case SLANG_IMAGE_FORMAT_rg16ui:
                return  bgfx::TextureFormat::RG16U;
            case SLANG_IMAGE_FORMAT_rg8ui:
                return  bgfx::TextureFormat::RG8U;
            case SLANG_IMAGE_FORMAT_r32ui:
                return  bgfx::TextureFormat::R32U;
            case SLANG_IMAGE_FORMAT_r16ui:
                return  bgfx::TextureFormat::R16U;
            case SLANG_IMAGE_FORMAT_r8ui:
                return  bgfx::TextureFormat::R8U;
            case SLANG_IMAGE_FORMAT_r64ui:
            case SLANG_IMAGE_FORMAT_r64i:
                return  bgfx::TextureFormat::Unknown;
            case SLANG_IMAGE_FORMAT_bgra8:
                return  bgfx::TextureFormat::BGRA8;
            default:
                return  bgfx::TextureFormat::Unknown;
            }
        }

        std::unordered_map<bgfx::Attrib::Enum, uint16_t> attribToIdMap = {
            {bgfx::Attrib::Position, 0x0001},
            {bgfx::Attrib::Normal, 0x0002},
            {bgfx::Attrib::Tangent, 0x0003},
            {bgfx::Attrib::Bitangent, 0x0004},
            {bgfx::Attrib::Color0, 0x0005},
            {bgfx::Attrib::Color1, 0x0006},
            {bgfx::Attrib::Color2, 0x0018},
            {bgfx::Attrib::Color3, 0x0019},
            {bgfx::Attrib::Indices, 0x000e},
            {bgfx::Attrib::Weight, 0x000f},
            {bgfx::Attrib::TexCoord0, 0x0010},
            {bgfx::Attrib::TexCoord1, 0x0011},
            {bgfx::Attrib::TexCoord2, 0x0012},
            {bgfx::Attrib::TexCoord3, 0x0013},
            {bgfx::Attrib::TexCoord4, 0x0014},
            {bgfx::Attrib::TexCoord5, 0x0015},
            {bgfx::Attrib::TexCoord6, 0x0016},
            {bgfx::Attrib::TexCoord7, 0x0017},
        };

        uint16_t attribToId(bgfx::Attrib::Enum attr)
        {
            return attribToIdMap.at(attr);
        }

        uint16_t layoutParamToId(const LayoutParam &param, SlangCompileTarget target)
        {
            if (target == SLANG_SPIRV && param.name.find("data") != std::string::npos)
            {
                return std::numeric_limits<uint16_t>::max();
            }
            return attribToId(param.attrib);
        }

        expected<std::vector<LayoutParam>, std::string> getLayoutParams(slang::VariableLayoutReflection& layout, const std::string &prefix = "") noexcept
        {
            std::vector<LayoutParam> params;
            auto *typeLayout = layout.getTypeLayout();

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
                break;
            }
            case slang::TypeReflection::Kind::Vector:
            case slang::TypeReflection::Kind::Scalar:
            {
                if (layout.getSemanticName() == nullptr)
                {
                    return unexpected{fmt::format("no semantic name specified for var: {}", layout.getName())};
                }
                auto attribResult = getBgfxAttrib(layout.getSemanticName(), layout.getSemanticIndex());
                if (!attribResult)
                {
                    return unexpected{fmt::format("unsupported semantic name: {}", layout.getSemanticName())};
                }
                if (auto &attrib = attribResult.value())
                {
                    params.emplace_back(layout.getName(), prefix + layout.getName(), static_cast<bgfx::Attrib::Enum>(*attrib));
                }
                break;
            }
            default:
                return unexpected{fmt::format("Unsupported type of param: {}", layout.getName())};
            }
            return params;
        }

        expected<std::vector<Uniform>, std::string> getUniforms(slang::VariableLayoutReflection& param, SlangStage stage)
        {
            auto& paramType = *param.getType();
            auto paramKind = paramType.getKind();
            auto isCompute = stage == SLANG_STAGE_COMPUTE;
            auto paramName = param.getName();

            std::vector<Uniform> uniforms;

            if (paramKind == slang::TypeReflection::Kind::ConstantBuffer)
            {
                auto& paramLayout = *param.getTypeLayout();
                auto &elementsVarLayout = *paramLayout.getElementVarLayout();
                auto& elementsTypeLayout = *elementsVarLayout.getTypeLayout();

                auto paramCount = elementsTypeLayout.getFieldCount();
                for (unsigned int i = 0; i < paramCount; i++)
                {
                    auto& subparam = *elementsTypeLayout.getFieldByIndex(i);
                    auto fieldResult = getUniforms(subparam, stage);
                    if (!fieldResult)
                    {
                        return unexpected{fmt::format("getting uniform for field {}: {}", subparam.getName(), fieldResult.error())};
                    }
                    uniforms.insert(uniforms.end(), fieldResult->begin(), fieldResult->end());
                }
                return uniforms;
            }

            bool isArray = paramKind == slang::TypeReflection::Kind::Array;
            auto& elementType = isArray ? *paramType.getElementType() : paramType;
            auto convertResult = convertUniformType(elementType, isCompute);
            if (!convertResult)
            {
                return unexpected{fmt::format("faied to convert param {}: {}", paramName, convertResult.error())};
            }
            auto convertedType = *convertResult;
            if (convertedType == bgfx::UniformType::Count)
            {
                return uniforms;
            }
            bool isBuffer = elementType.getKind() == slang::TypeReflection::Kind::Resource &&
                            elementType.getResourceShape() == SlangResourceShape::SLANG_STRUCTURED_BUFFER;
            bool isSampler = convertedType == bgfx::UniformType::Sampler && !isCompute;
            bool isStorageImage = elementType.getKind() == slang::TypeReflection::Kind::Resource && !isBuffer && !isCompute;

            auto& uniform = uniforms.emplace_back();

            uniform.name = paramName;
            uniform.type = convertedType;
            uniform.num = isArray ? paramType.getElementCount() : 1;

            if (isSampler)
            {
                uniform.regIndex = param.getBindingIndex();
                uniform.regCount = uniform.num;
            }
            else if (isBuffer)
            {
                uniform.regIndex = param.getBindingIndex();
                uniform.regCount = storageBufferDescriptor;
            }
            else if (isStorageImage)
            {
                uniform.regIndex = param.getBindingIndex();
                uniform.regCount = storageImageDescriptor;
            }
            else
            {
                uniform.regIndex = param.getOffset();
                uniform.regCount = elementType.getRowCount() * uniform.num;
            }

            if (isSampler || isStorageImage)
            {
                uniform.texComponent = convertTextureComponentType(paramType);
                uniform.texDimension = convertTextureDimension(paramType);
                uniform.texFormat = convertTextureFormat(param.getImageFormat());
            }

            return uniforms;
        }

        expected<UniformData, std::string> getUniforms(slang::ProgramLayout& layout, SlangStage stage) noexcept
        {
            auto& varsLayout = *layout.getGlobalParamsVarLayout();
            auto& typeLayout = *varsLayout.getTypeLayout();
            UniformData data;

            auto& elementsVarLayout = *typeLayout.getElementVarLayout();
            auto& elementsTypeLayout = *elementsVarLayout.getTypeLayout();

            auto paramCount = elementsTypeLayout.getFieldCount();
            for (unsigned int i = 0; i < paramCount; i++)
            {
                auto &param = *elementsTypeLayout.getFieldByIndex(i);
                auto fieldResult = getUniforms(param, stage);
                if (!fieldResult)
                {
                    return unexpected{fmt::format("getting uniform for field {}: {}", param.getName(), fieldResult.error())};
                }
                data.uniforms.insert(data.uniforms.end(), fieldResult->begin(), fieldResult->end());
            }

            data.bufferSize = static_cast<uint16_t>(elementsTypeLayout.getSize());
            return data;
        }

        uint32_t hashLayoutParams(const std::vector<LayoutParam>& params) noexcept
        {
            uint32_t hash = 0;
            std::vector<std::string> names(params.size());
            std::transform(params.begin(), params.end(), names.begin(), [](const auto &p)
                           { return p.name; });
            std::sort(names.begin(), names.end());
            std::hash<std::string> hasher;
            for (const auto &name : names)
            {
                hash ^= hasher(name);
            }
            return hash;
        }

        struct SlangShaderContext final
        {
            SlangInt entryPointIdx = 0;
            SlangInt targetIdx = 0;
            SlangCompileTarget target;
        };

        struct DarmokShaderContext final
        {
            std::unordered_set<std::string> defines;
            std::optional<bgfx::RendererType::Enum> bgfxRenderer;
            SlangInt vertEntryPointIdx = -1;
            SlangInt fragEntryPointIdx = -1;
        };

        expected<protobuf::Shader, std::string> createShader(slang::IComponentType& linkedProgram, const DarmokShaderContext& darmokCtx, const SlangShaderContext& slangCtx) noexcept
        {
            protobuf::Shader shader;

            for (auto &define : darmokCtx.defines)
            {
                *shader.add_defines() = define;
            }

            Slang::ComPtr<slang::IBlob> data;
            Slang::ComPtr<slang::IBlob> diagnostics;
            if (SLANG_FAILED(linkedProgram.getEntryPointCode(slangCtx.entryPointIdx, slangCtx.targetIdx, data.writeRef(), diagnostics.writeRef())))
            {
                return unexpected{getDiagnosticsString(diagnostics)};
            }
            std::string shaderData{static_cast<const char*>(data->getBufferPointer()), data->getBufferSize()};

            auto* programLayout = linkedProgram.getLayout(slangCtx.targetIdx, diagnostics.writeRef());
            if (programLayout == nullptr)
            {
                return unexpected{getDiagnosticsString(diagnostics)};
            }
            auto* entryPoint = programLayout->getEntryPointByIndex(slangCtx.entryPointIdx);
            auto stage = entryPoint->getStage();

            std::vector<LayoutParam> inputParams;
            {
                for (int i = 0; i < entryPoint->getParameterCount(); i++)
                {
                    auto *varLayout = entryPoint->getParameterByIndex(i);
                    auto result = getLayoutParams(*varLayout);
                    if (!result)
                    {
                        return unexpected<std::string>{fmt::format("could not get input param {} layout: {}", i, result.error())};
                    }
                    inputParams.insert(inputParams.begin(), result->begin(), result->end());
                }
            }

            std::vector<LayoutParam> outputParams;
            {
                auto *varLayout = entryPoint->getResultVarLayout();
                if (!varLayout)
                {
                    return unexpected<std::string>{"could not get result layout"};
                }
                auto result = getLayoutParams(*varLayout);
                if (!result)
                {
                    return unexpected<std::string>{fmt::format("could not get result layout: {}", result.error())};
                }
                outputParams = std::move(result).value();
            }

            Slang::ComPtr<slang::IMetadata> metadata;
            if (SLANG_FAILED(linkedProgram.getEntryPointMetadata(slangCtx.entryPointIdx, slangCtx.targetIdx, metadata.writeRef(), diagnostics.writeRef())))
            {
                return unexpected{getDiagnosticsString(diagnostics)};
            }

            auto uniformsResult = getUniforms(*programLayout, stage);
            if (!uniformsResult)
            {
                return unexpected{fmt::format("getting uniforms: {}", uniformsResult.error())};
            }
            auto uniformData = std::move(uniformsResult).value();

            if (darmokCtx.bgfxRenderer && GlslCompiler::supports(*darmokCtx.bgfxRenderer))
            {
                auto profile = ShaderParser::getRendererProfile(*darmokCtx.bgfxRenderer);
                auto bgfxResult = GlslCompiler::compileToBgfx(shaderData, convertShaderType(stage), profile);
                if (!bgfxResult)
                {
                    return unexpected{fmt::format("failed to compile bgfx glsl profile {}: {}", profile, bgfxResult.error())};
                }
                shaderData = std::move(bgfxResult).value();
            }

            Data bgfxShaderData;
            DataOutputStream out{bgfxShaderData};

            out.writebin(SlangBgfxShaderUtils::getMagic(stage));

            if (stage == SLANG_STAGE_FRAGMENT)
            {
                out.writebin(hashLayoutParams(inputParams));
                out.writebin<uint32_t>(0);
            }
            else
            {
                out.writebin<uint32_t>(0);
                out.writebin(hashLayoutParams(outputParams));
            }

            out.writebin(static_cast<uint16_t>(uniformData.uniforms.size()));

            const uint8_t fragmentBit = stage == SLANG_STAGE_FRAGMENT ? uniformFragmentBit : 0;

            for (const auto& uniform : uniformData.uniforms)
            {
                out.writebin<uint8_t>(uniform.name.size());
                out.writebin(&uniform.name.front(), sizeof(std::string::value_type) * uniform.name.size());
                out.writebin(static_cast<uint8_t>(uniform.type | fragmentBit));
                out.writebin(uniform.num);
                out.writebin(uniform.regIndex);
                out.writebin(uniform.regCount);
                out.writebin(static_cast<uint8_t>(uniform.texComponent));
                out.writebin(static_cast<uint8_t>(uniform.texDimension));
                out.writebin(static_cast<uint16_t>(uniform.texFormat));
            }

            out.writebin<uint32_t>(shaderData.size());
            out.writebin(&shaderData.front(), shaderData.size());
            out.writebin<uint8_t>(0);

            out.writebin<uint8_t>(inputParams.size());
            for (const auto &param : inputParams)
            {
                out.writebin(layoutParamToId(param, slangCtx.target));
            }
            out.writebin(uniformData.bufferSize);

            auto dataStr = bgfxShaderData.view(0, out.tellp()).toString();
            shader.set_data(std::move(dataStr));

            return shader;
        }

        expected<Slang::ComPtr<slang::IComponentType>, std::string> compileProgram(const SlangProgramCompiler::Source& src, slang::ISession& session, OptionalRef<std::ostream> log) noexcept
        {
            auto addLog = [log](const std::string &title, const std::string &msg)
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

            std::vector<slang::IComponentType *> components;
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

        expected<bool, std::string> updateShader(protobuf::RendererProgram& rendererProg, slang::IComponentType& linkedProgram, const SlangShaderContext& slangCtx, const DarmokShaderContext& darmokCtx) noexcept
        {
            auto result = createShader(linkedProgram, darmokCtx, slangCtx);
            if (!result)
            {
                return unexpected{fmt::format("failed to create shader: {}", result.error())};
            }
            auto shader = std::move(result).value();
            google::protobuf::RepeatedPtrField<protobuf::Shader>* shaders = nullptr;
            if (slangCtx.entryPointIdx == darmokCtx.vertEntryPointIdx)
            {
                shaders = rendererProg.mutable_vertex_shaders();
            }
            else
            {
                shaders = rendererProg.mutable_fragment_shaders();
            }

            /* possible optimization
            for (auto& oldShader : *shaders)
            {
                if (oldShader.data() == shader.data())
                {
                    return false;
                }
            }
            */

            shaders->Add(std::move(shader));
            return true;
        }

        expected<void, std::string> updateRendererProgram(protobuf::RendererProgram& rendererProg, slang::IComponentType& linkedProgram, SlangShaderContext slangCtx, const DarmokShaderContext& darmokCtx) noexcept
        {
            slangCtx.entryPointIdx = darmokCtx.vertEntryPointIdx;
            auto result = updateShader(rendererProg, linkedProgram, slangCtx, darmokCtx);
            if (!result)
            {
                return unexpected{fmt::format("failed to update vertex shader: {}", result.error())};
            }
            slangCtx.entryPointIdx = darmokCtx.fragEntryPointIdx;
            result = updateShader(rendererProg, linkedProgram, slangCtx, darmokCtx);
            if (!result)
            {
                return unexpected{fmt::format("failed to update fragment shader: {}", result.error())};
            }
            return {};
        }

        std::vector<slang::CompilerOptionEntry> getCompilerOptions(bgfx::RendererType::Enum renderer) noexcept
        {
            return {};
        }

        expected<DarmokShaderContext, std::string> updateVarying(protobuf::Varying &varying, slang::ProgramLayout &layout)
        {
            DarmokShaderContext ctx;
            for (SlangInt i = 0; i < layout.getEntryPointCount(); i++)
            {
                auto entryPoint = layout.getEntryPointByIndex(i);
                if (entryPoint->getStage() == SLANG_STAGE_VERTEX)
                {
                    auto result = createVertexLayout(*entryPoint);
                    if (!result)
                    {
                        return unexpected<std::string>{"failed to create vertex layout: " + result.error()};
                    }
                    *varying.mutable_vertex() = std::move(result).value();
                    ctx.vertEntryPointIdx = i;
                }
                if (entryPoint->getStage() == SLANG_STAGE_FRAGMENT)
                {
                    auto result = createFragmentLayout(*entryPoint);
                    if (!result)
                    {
                        return unexpected<std::string>{"failed to create fragment layout: " + result.error()};
                    }
                    *varying.mutable_fragment() = std::move(result).value();
                    ctx.fragEntryPointIdx = i;
                }
                if (ctx.vertEntryPointIdx >= 0 && ctx.fragEntryPointIdx >= 0)
                {
                    break;
                }
            }
            if (ctx.vertEntryPointIdx < 0)
            {
                return unexpected<std::string>{"missing vertex entry point"};
            }
            if (ctx.fragEntryPointIdx < 0)
            {
                return unexpected<std::string>{"missing fragment entry point"};
            }
            return ctx;
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
    }

    expected<SlangProgramCompilerImpl, std::string> SlangProgramCompilerImpl::create(const Config& config) noexcept
    {
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        SlangGlobalSessionDesc globalDesc = {};
        SLANG_TRY("creating global slang session",
            slang::createGlobalSession(&globalDesc, globalSession.writeRef()));

		return SlangProgramCompilerImpl{ std::move(globalSession), config };
    }

    expected<Slang::ComPtr<slang::ISession>, std::string> SlangProgramCompilerImpl::createSession(bgfx::RendererType::Enum renderer, const std::unordered_set<std::string>& defines) noexcept
    {
        using namespace SlangBgfxShaderUtils;
        auto sessionDesc = _sessionDesc;
        std::vector<slang::PreprocessorMacroDesc> macros;

        std::unordered_set<std::string> fdefines;
        std::transform(defines.begin(), defines.end(), std::inserter(fdefines, fdefines.end()), [](const std::string &define)
                       { return "DARMOK_VARIANT_" + define; });
        for (auto& define : fdefines)
        {
            macros.emplace_back(define.c_str(), "1");
        }

        sessionDesc.preprocessorMacroCount = macros.size();
        sessionDesc.preprocessorMacros = macros.data();

        auto options = getCompilerOptions(renderer);
        sessionDesc.compilerOptionEntryCount = options.size();
        sessionDesc.compilerOptionEntries = options.data();

        auto itr = _rendererTargets.find(renderer);
        if (itr == _rendererTargets.end())
        {
            return unexpected{fmt::format("unsupported renderer: {}", renderer)};
        }
        auto target = itr->second;
        slang::TargetDesc targetDesc{.format = target};
        auto itr2 = _targetProfileMap.find(target);
        if (itr2 != _targetProfileMap.end())
        {
            targetDesc.profile = _globalSession->findProfile(itr2->second.c_str());
        }

        std::vector<slang::TargetDesc> targetDescs{targetDesc};
        sessionDesc.targetCount = targetDescs.size();
        sessionDesc.targets = targetDescs.data();

        Slang::ComPtr<slang::ISession> session;

        SLANG_TRY("creating slang session",
                  _globalSession->createSession(sessionDesc, session.writeRef()));

        return session;
    }

    expected<void, std::string> SlangProgramCompilerImpl::compileRendererProgram(const Source& src, protobuf::Program& progDef, bgfx::RendererType::Enum renderer, const std::unordered_set<std::string>& defines) noexcept
    {
        using namespace SlangBgfxShaderUtils;

        auto itr = _rendererTargets.find(renderer);
        if (itr == _rendererTargets.end())
        {
            return unexpected{fmt::format("unsupported renderer: {}", renderer)};
        }
        auto target = itr->second;

        auto sessionResult = createSession(renderer, defines);
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

        auto varyingResult = updateVarying(*progDef.mutable_varying(), *layout);
        if (!varyingResult)
        {
            return unexpected{fmt::format("failed to update varying: {}", varyingResult.error())};
        }

        auto darmokCtx = std::move(varyingResult).value();
        darmokCtx.bgfxRenderer = renderer;
        darmokCtx.defines = defines;
        SlangShaderContext slangCtx{
            .targetIdx = 0,
            .target = target,
        };

        auto& rendererProg = ProgramDefinitionWrapper{progDef}.getRendererProgram(renderer);
        auto result = updateRendererProgram(rendererProg, *linkedProgram, slangCtx, darmokCtx);
        if (!result)
        {
            return unexpected{fmt::format("failed to update program for renderer {}: {}", renderer, result.error())};
        }
        return {};
    }

    expected<protobuf::Program, std::string> SlangProgramCompilerImpl::operator()(const Source& src) noexcept
    {
        using namespace SlangBgfxShaderUtils;

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
            for (auto& [renderer, target] : _rendererTargets)
            {
                auto itr = std::find(_supportedTargets.begin(), _supportedTargets.end(), target);
                if (itr == _supportedTargets.end())
                {
                    continue;
                }
                auto result = compileRendererProgram(src, programDef, renderer, defineComb);
                if (!result)
                {
                    return unexpected{fmt::format("failed to compile for renderer {}: {}", renderer, result.error())};
                }
            }
        }

        return programDef;
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
