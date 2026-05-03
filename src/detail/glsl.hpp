#pragma once

#include "detail/program_core.hpp"
#include <darmok/expected.hpp>
#include <string>
#include <unordered_set>


namespace darmok
{
    namespace GlslCompiler
    {
        bool supports(bgfx::RendererType::Enum renderer) noexcept;
        expected<std::string, std::string> compileToBgfx(std::string_view spirv, ShaderType type, const std::string& profile = "150") noexcept;
    }
}

;
;