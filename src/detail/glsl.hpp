#pragma once

#include "detail/program_core.hpp"
#include <darmok/expected.hpp>
#include <string>
#include <unordered_set>


namespace darmok
{
    struct ProgramCompilerConfig;

    namespace protobuf
    {
        class Varying;
    }

    namespace GlslShaderConverter
    {
        expected<std::string, std::string> convertToBgfx(std::string_view glsl, ShaderType type) noexcept;
        expected<std::string, std::string> compileToBgfx(std::string_view glsl, ShaderType type, const ProgramCompilerConfig& config, const protobuf::Varying& varying, const std::unordered_set<std::string>& defines = {}) noexcept;
    }
}

;
;