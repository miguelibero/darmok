#pragma once

#include <CEGUI/ShaderWrapper.h>
#include <memory>

namespace darmok
{
    class Program;

    class CeguiShaderWrapper final : public CEGUI::ShaderWrapper
    {
    public:
        CeguiShaderWrapper(const std::shared_ptr<Program>& program) noexcept;
        void prepareForRendering(const CEGUI::ShaderParameterBindings* shaderParameterBindings) override;
    private:
        std::shared_ptr<Program> _program;
    };
}