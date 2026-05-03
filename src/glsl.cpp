#include "detail/glsl.hpp"
#include <darmok/stream.hpp>
#include <darmok/string.hpp>
#include <fmt/format.h>
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

        expected<std::string, std::string> compileToBgfx(std::string_view glsl, ShaderType type, const std::string& profile) noexcept
        {
            spirv_cross::CompilerGLSL compiler{reinterpret_cast<const uint32_t*>(&glsl.front()), glsl.size() / 4};
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

            try
            {
                auto source = compiler.compile();
                return source;
            }
            catch(const std::exception &e)
            {
                return unexpected<std::string>{e.what()};
            }
        }
    }
}