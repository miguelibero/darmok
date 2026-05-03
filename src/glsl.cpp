#include "detail/glsl.hpp"
#include "detail/program_core.hpp"
#include <darmok/stream.hpp>
#include <darmok/string.hpp>
#include <fmt/format.h>
#include <regex>
#include <unordered_map>

namespace darmok
{
    namespace GlslShaderConverter
    {
        const std::regex versionRegex(R"(^\s*#version.*)");
        const std::regex inRegex(R"(^\s*in\s+\w+\s+(\w+)\s*;)");
        const std::regex outRegex(R"(^\s*out\s+\w+\s+(\w+)\s*;)");
        const std::regex attribSuffixRegex(R"(_\d+$)");

        const std::unordered_set<std::string> attribPrefixes{"IN_", "OUT_", "entryPointParam_vert_"};

        std::string fixAttributeName(std::string name) noexcept
        {
            for (auto& prefix : attribPrefixes)
            {
                if (name.starts_with(prefix))
                {
                    name = name.substr(prefix.length());
                }
            }
            name = std::regex_replace(name, attribSuffixRegex, "");
            return name;
        }

        expected<std::string, std::string> convertToBgfx(std::string_view glsl, ShaderType type) noexcept
        {
            std::istringstream in{std::string{glsl}};
            std::ostringstream body;

            std::string line;

            std::unordered_set<std::string> inputs;
            std::unordered_set<std::string> outputs;
            std::unordered_map<std::string, std::string> attribs;

            while (std::getline(in, line))
            {
                std::smatch m;
                std::string t{StringUtils::trim(line)};

                if (std::regex_match(t, versionRegex))
                {
                    continue;
                }

                if (type == ShaderType::Vertex && std::regex_match(t, m, inRegex))
                {
                    std::string attrib{m[1]};
                    auto fattrib = fixAttributeName(attrib);
                    attribs[attrib] = fattrib;
                    inputs.insert(fattrib);
                    continue;
                }

                if (std::regex_match(t, m, outRegex))
                {
                    std::string attrib{m[1]};
                    auto fattrib = fixAttributeName(attrib);
                    attribs[attrib] = fattrib;
                    outputs.insert(fattrib);
                    continue;
                }

                if (t.starts_with("layout("))
                {
                    continue;
                }

                body << line << '\n';
            }

            std::ostringstream result;

            // Direct passthrough — no renaming, no mapping
            if (!inputs.empty())
            {
                result << "$input " << StringUtils::join(", ", inputs) << "\n";
            }

            if (!outputs.empty())
            {
                result << "$output " << StringUtils::join(", ", outputs) << "\n";
            }

            result << "#include <bgfx_shader.sh>\n";

            auto bodyStr = body.str();
            for (auto& [attrib, fattrib] : attribs)
            {
                StringUtils::replace(bodyStr, attrib, fattrib);
            }
            result << "\n" << bodyStr;

            return { result.str() };
        }

        expected<std::string, std::string> compileToBgfx(std::string_view glsl, ShaderType type, const ProgramCompilerConfig& programConfig, const protobuf::Varying& varying, const std::unordered_set<std::string>& defines) noexcept
        {
            const bgfx::RendererType::Enum renderer = bgfx::RendererType::OpenGL;
            const ShaderCompilerConfig config
            {
                .programConfig = programConfig,
                .path = getTempPath("bgfx_shader_input"),
                .varyingPath = getTempPath("bgfx_shader_varying"),
                .type = type,
            };

            auto convertResult = convertToBgfx(glsl, type);
            if (!convertResult)
            {
                return unexpected{fmt::format("converting glsl to bgfx: {}", convertResult.error())};
            }

            auto bgfxShader = std::move(convertResult).value();
            std::ofstream{config.path} << bgfxShader;
            ConstVaryingDefinitionWrapper{varying}.writeBgfx(config.varyingPath);

            const ShaderCompiler compiler{config};
            const ShaderCompilerOperation op
            {
                .renderer = renderer,
                .profile = ShaderParser::getRendererProfile(renderer),
                .defines = defines,
                .outputPath = getTempPath("bgfx_shader_output"),
            };

            auto cleanup = [&config, &op]() noexcept
            {
                std::filesystem::remove(config.path);
                std::filesystem::remove(config.varyingPath);
                std::filesystem::remove(op.outputPath);
            };

            auto result = compiler(op);
            if (!result)
            {
                cleanup();
                return unexpected{std::move(result).error()};
            }
            auto output = StreamUtils::readString(op.outputPath);
            cleanup();
            return output;
        }
    }
}