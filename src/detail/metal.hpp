#pragma once

#include <bgfx/bgfx.h>
#include <darmok/expected.hpp>
#include <string>

namespace darmok
{
    namespace MetalCompiler
    {
        bool supports(bgfx::RendererType::Enum renderer) noexcept;

        // Post-process Slang-generated MSL to match bgfx's Metal renderer conventions.
        // entryName is the original Slang entry point name (renamed to "xlatMtlMain").
        expected<std::string, std::string> postProcess(std::string msl, std::string_view entryName) noexcept;
    }
}
