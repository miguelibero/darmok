#include "shader.hpp"

namespace darmok
{
    CeguiShaderWrapper::CeguiShaderWrapper(const std::shared_ptr<Program>& program) noexcept
        : _program(program)
    {
    }

    void CeguiShaderWrapper::prepareForRendering(const CEGUI::ShaderParameterBindings* shaderParameterBindings)
    {
    }
}