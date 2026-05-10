#include "detail/glsl.hpp"
#include <darmok/stream.hpp>
#include <darmok/string.hpp>
#include <fmt/format.h>
#include <magic_enum/magic_enum_format.hpp>
#include <regex>
#include <unordered_map>
#include <spirv_glsl.hpp>

namespace darmok
{
    namespace GlslCompiler
    {
        bool supports(bgfx::RendererType::Enum renderer) noexcept
        {
            return renderer == bgfx::RendererType::OpenGL || renderer == bgfx::RendererType::OpenGLES;
        }

        const std::string uniformsPlaceholder{"[uniforms]"};

        std::string typeToString(const spirv_cross::SPIRType& type, spirv_cross::CompilerGLSL& compiler)
        {
            using T = spirv_cross::SPIRType;

            switch (type.basetype)
            {
            case T::Float:
            {
                switch (type.columns)
                {
                    case 1:
                    {
                        switch (type.vecsize)
                        {
                        case 1: return "float";
                        case 2: return "vec2";
                        case 3: return "vec3";
                        case 4: return "vec4";
                        }
                    }
                case 2: return "mat2";
                case 3: return "mat3";
                case 4: return "mat4";
                }
            }
            case T::Int:
            {
                switch (type.columns)
                {
                case 1:
                {
                    switch (type.vecsize)
                    {
                    case 1: return "int";
                    case 2: return "ivec2";
                    case 3: return "ivec3";
                    case 4: return "ivec4";
                    }
                }
                case 2: return "imat2";
                case 3: return "imat3";
                case 4: return "imat4";
                }
            }
            case T::UInt:
            {
                switch (type.columns)
                {
                case 1:
                {
                    switch (type.vecsize)
                    {
                    case 1: return "uint";
                    case 2: return "uvec2";
                    case 3: return "uvec3";
                    case 4: return "uvec4";
                    }
                }
                case 2: return "umat2";
                case 3: return "umat3";
                case 4: return "umat4";
                }
            }
            case T::Struct:
                return compiler.get_name(type.self);
            default:
                break;
            }
            return "";
        }

        expected<std::string, std::string> stripGlslUbo(std::string glsl, spirv_cross::CompilerGLSL& compiler, const spirv_cross::SmallVector<spirv_cross::Resource>& ubos) noexcept
        {
            std::ostringstream out;
            for (auto& ubo : ubos)
            {
                std::regex defRegex{"struct " + ubo.name + "\\s*\\{[^}]+\\};"};
                std::regex declRegex{"uniform\\s+" + ubo.name + ".*;"};

                const auto& bufferName = compiler.get_name(ubo.id);

                glsl = std::regex_replace(glsl, defRegex, "");
                glsl = std::regex_replace(glsl, declRegex, uniformsPlaceholder);

                auto& type = compiler.get_type(ubo.type_id);
                auto memberCount = type.member_types.size();

                for (size_t i = 0; i < memberCount; i++)
                {
                    const auto& memberName = compiler.get_member_name(ubo.base_type_id, i);
                    const auto& memberType = compiler.get_type(type.member_types[i]);
                    auto typeStr = typeToString(memberType, compiler);
                    if (typeStr.empty())
                    {
                        return unexpected{ fmt::format("failed to convert uniform {} to string", memberName) };
                    }
                    out << "uniform " << typeStr << " " << memberName << ";\n";

                    StringUtils::replace(glsl, bufferName + "." + memberName, memberName);
                }
                StringUtils::replace(glsl, uniformsPlaceholder, out.str());
                out.clear();
            }

            return glsl;
        }

        expected<std::string, std::string> compileToBgfx(std::string_view spirv, ShaderType type, const std::string& profile) noexcept
        {
            spirv_cross::CompilerGLSL compiler{reinterpret_cast<const uint32_t*>(&spirv.front()), spirv.size() / 4};
            spirv_cross::CompilerGLSL::Options options;
            options.version = std::stoi(profile);
            options.es = profile.ends_with("_es");
            options.emit_uniform_buffer_as_plain_uniforms = true;
            options.enable_420pack_extension = false;
            options.fragment.default_float_precision = spirv_cross::CompilerGLSL::Options::Precision::Highp;
            compiler.set_common_options(options);

            compiler.build_dummy_sampler_for_combined_images();
            compiler.build_combined_image_samplers();
            for (const auto &sampler : compiler.get_combined_image_samplers())
            {
                compiler.set_name(sampler.combined_id, compiler.get_name(sampler.image_id));
            }

            auto resources = compiler.get_shader_resources();

            if (type == ShaderType::Vertex)
            {
                for (auto& input : resources.stage_inputs)
                {
                    std::string_view name = input.name;
                    const auto lastDotPos = name.rfind('.');
                    if (lastDotPos != std::string::npos)
                    {
                        name = name.substr(lastDotPos + 1);
                        compiler.set_name(input.id, std::string{name});
                    }
                }
            }

            try
            {
                auto source = compiler.compile();
                auto result = stripGlslUbo(source, compiler, resources.uniform_buffers);
                if (!result)
                {
                    return unexpected{ "failed to strip glsl uniform buffer objects: " + result.error() };
                }
                source = std::move(result).value();
                return source;
            }
            catch(const std::exception &e)
            {
                return unexpected<std::string>{e.what()};
            }
        }
    }
}