#include "shader.hpp"
#include "texture.hpp"
#include <bgfx/bgfx.h>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    CeguiShaderWrapper::CeguiShaderWrapper(const std::shared_ptr<Program>& program) noexcept
        : _program(program)
    {
    }

    CeguiShaderWrapper::~CeguiShaderWrapper() noexcept
    {
        destroyUniformHandles();
    }

    void CeguiShaderWrapper::destroyUniformHandles() noexcept
    {
        for (auto& elm : _uniformHandles)
        {
            if (isValid(elm.second))
            {
                bgfx::destroy(elm.second);
            }
        }
        _uniformHandles.clear();
    }

    bgfx::UniformType::Enum CeguiShaderWrapper::getUniformType(CEGUI::ShaderParamType paramType) noexcept
    {
        switch (paramType)
        {
        case CEGUI::ShaderParamType::Texture:
            return bgfx::UniformType::Sampler;
        case CEGUI::ShaderParamType::Matrix4X4:
            return bgfx::UniformType::Mat4;
        }
        return bgfx::UniformType::Vec4;
    }

    void CeguiShaderWrapper::setUniform(CEGUI::ShaderParameter& param, bgfx::UniformHandle handle) noexcept
    {
        switch (param.getType())
        {
            case CEGUI::ShaderParamType::Int:
            {
                glm::vec3 data(static_cast<CEGUI::ShaderParameterInt&>(param).d_parameterValue);
                bgfx::setUniform(handle, glm::value_ptr(data));
                break;
            }
            case CEGUI::ShaderParamType::Float:
            {
                glm::vec3 data(static_cast<CEGUI::ShaderParameterFloat&>(param).d_parameterValue);
                bgfx::setUniform(handle, glm::value_ptr(data));
                break;
            }
            case CEGUI::ShaderParamType::Texture:
            {
                auto ceguiTex = static_cast<CEGUI::ShaderParameterTexture&>(param).d_parameterValue;
                auto texHandle = static_cast<const CeguiTexture*>(ceguiTex)->getBgfxHandle();
                bgfx::setTexture(0, handle, texHandle);
                break;
            }
            case CEGUI::ShaderParamType::Matrix4X4:
            {
                auto& data = static_cast<CEGUI::ShaderParameterMatrix&>(param).d_parameterValue;
                bgfx::setUniform(handle, glm::value_ptr(data));
                break;
            }
        }
    }

    void CeguiShaderWrapper::prepareForRendering(const CEGUI::ShaderParameterBindings* shaderParameterBindings) noexcept
    {
        updateUniformHandles(*shaderParameterBindings);
    }

    void CeguiShaderWrapper::setUniforms(const CEGUI::ShaderParameterBindings& shaderParameterBindings) const noexcept
    {
        for (auto& elm : shaderParameterBindings.getShaderParameterBindings())
        {
            auto type = getUniformType(elm.second->getType());
            auto itr = _uniformHandles.find(elm.first);
            if (itr != _uniformHandles.end())
            {
                setUniform(*elm.second, itr->second);
            }
        }
    }

    void CeguiShaderWrapper::updateUniformHandles(const CEGUI::ShaderParameterBindings& shaderParameterBindings) noexcept
    {
        destroyUniformHandles();
        for (auto& elm : shaderParameterBindings.getShaderParameterBindings())
        {
            auto type = getUniformType(elm.second->getType());
            auto handle = bgfx::createUniform(elm.first.c_str(), type);
            _uniformHandles.emplace(elm.first, handle);
        }
    }

    const std::shared_ptr<Program>& CeguiShaderWrapper::getDarmokProgram() const noexcept
    {
        return _program;
    }
}