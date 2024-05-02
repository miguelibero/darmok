#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <CEGUI/ShaderWrapper.h>
#include <CEGUI/ShaderParameterBindings.h>
#include <bgfx/bgfx.h>

namespace darmok
{
    class Program;

    class CeguiShaderWrapper final : public CEGUI::ShaderWrapper
    {
    public:
        CeguiShaderWrapper(const std::shared_ptr<Program>& program) noexcept;
        ~CeguiShaderWrapper() noexcept;
        void prepareForRendering(const CEGUI::ShaderParameterBindings* shaderParameterBindings) noexcept override;
        const std::shared_ptr<Program>& getDarmokProgram() const noexcept;
    private:
        std::shared_ptr<Program> _program;
        std::unordered_map<std::string, bgfx::UniformHandle> _uniformHandles;

        static bgfx::UniformType::Enum getUniformType(CEGUI::ShaderParamType paramType) noexcept;
        static void setUniform(CEGUI::ShaderParameter& param, bgfx::UniformHandle handle) noexcept;
        void setUniforms(const CEGUI::ShaderParameterBindings& shaderParameterBindings) const noexcept;
        void updateUniformHandles(const CEGUI::ShaderParameterBindings& shaderParameterBindings) noexcept;
        void destroyUniformHandles() noexcept;
    };
}