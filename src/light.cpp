#include <darmok/light.hpp>
#include <darmok/vertex.hpp>
#include <darmok/transform.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/material.hpp>
#include <darmok/uniform.hpp>
#include <darmok/buffer.hpp>

#include "program_def.hpp"
#include "generated/shaders/phong_lighting_progdef.h"

namespace darmok
{
    PointLight::PointLight(const glm::vec3& intensity, float radius) noexcept
        : _intensity(intensity)
        , _radius(radius)
        , _diffuseColor(Colors::white)
        , _specularColor(Colors::white)
    {
    }

    PointLight& PointLight::setRadius(float radius) noexcept
    {
        _radius = radius;
        return *this;
    }

    PointLight& PointLight::setIntensity(const glm::vec3& intensity) noexcept
    {
        _intensity = intensity;
        return *this;
    }

    PointLight& PointLight::setColor(const Color3& color) noexcept
    {
        _diffuseColor = color;
        _specularColor = color;
        return *this;
    }

    PointLight& PointLight::setDiffuseColor(const Color3& color) noexcept
    {
        _diffuseColor = color;
        return *this;
    }

    PointLight& PointLight::setSpecularColor(const Color3& color) noexcept
    {
        _specularColor = color;
        return *this;
    }

    float PointLight::getRadius() const noexcept
    {
        return _radius;
    }

    const glm::vec3& PointLight::getIntensity() const noexcept
    {
        return _intensity;
    }

    const Color3& PointLight::getDiffuseColor() const noexcept
    {
        return _diffuseColor;
    }

    const Color3& PointLight::getSpecularColor() const noexcept
    {
        return _specularColor;
    }

    AmbientLight::AmbientLight(const glm::vec3& intensity) noexcept
        : _intensity(intensity)
        , _color(Colors::black)
    {
    }

    AmbientLight& AmbientLight::setColor(const Color3& color) noexcept
    {
        _color = color;
        return *this;
    }

    AmbientLight& AmbientLight::setIntensity(const glm::vec3& intensity) noexcept
    {
        _intensity = intensity;
        return *this;
    }

    const glm::vec3& AmbientLight::getIntensity() const noexcept
    {
        return _intensity;
    }

    [[nodiscard]] const Color3& AmbientLight::getColor() const noexcept
    {
        return _color;
    }


    const ProgramDefinition& PhongLightRenderer::getProgramDefinition()
    {
        static auto def = ProgramDefinition::createFromJson(phong_lighting_progdef);
        return def;
    }

    PhongLightRenderer::PhongLightRenderer() noexcept
        : _lightCountUniform{ bgfx::kInvalidHandle }
        , _lightDataUniform{ bgfx::kInvalidHandle }
    {
    }

    const std::string PhongLightRenderer::_pointLightsBufferName = "pointLights";
    const std::string PhongLightRenderer::_lightCountUniformName = "lightCount";
    const std::string PhongLightRenderer::_lightDataUniformName = "phongLightingData";

    void PhongLightRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _cam = cam;
        auto& progDef = getProgramDefinition();
        {
            auto buffer = progDef.getBuffer(_pointLightsBufferName);
            if (buffer)
            {
                _pointLightBuffer = buffer->createHandle();
            }
        }
        {
            auto uniform = progDef.getUniform(_lightCountUniformName);
            if (uniform)
            {
                _lightCountUniform = uniform.value().createHandle();
            }
        }
        {
            auto uniform = progDef.getUniform(_lightDataUniformName);
            if (uniform)
            {
                _lightDataUniform = uniform.value().createHandle();
            }
        }
    }

    void PhongLightRenderer::shutdown() noexcept
    {
        if (isValid(_lightCountUniform))
        {
            bgfx::destroy(_lightCountUniform);
        }
        if (isValid(_lightDataUniform))
        {
            bgfx::destroy(_lightDataUniform);
        }
        if (isValid(_pointLightBuffer))
        {
            bgfx::destroy(_pointLightBuffer);
        }
    }

    size_t PhongLightRenderer::updatePointLights() noexcept
    {
        auto& registry = _scene->getRegistry();
        auto pointLights = _cam->createEntityView<PointLight>(registry);

        auto& progDef = getProgramDefinition();
        auto writer = BufferDataWriter(progDef.getBuffer(_pointLightsBufferName).value());
        writer.load(std::move(_pointLights));

        size_t index = 0;
        for (auto entity : pointLights)
        {
            auto& pointLight = registry.get<const PointLight>(entity);
            auto trans = registry.try_get<const Transform>(entity);
            if (trans != nullptr)
            {
                writer.set("position", index, trans->getPosition());
            }
            writer.set("diffuseColor", index, Colors::normalize(pointLight.getDiffuseColor()));
            writer.set("specularColor", index, Colors::normalize(pointLight.getSpecularColor()));
            index++;
        }

        _pointLights = writer.finish();
        bgfx::update(_pointLightBuffer, 0, _pointLights.makeRef());

        return index;
    }

    void PhongLightRenderer::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return;
        }
        auto& registry = _scene->getRegistry();
        auto& progDef = getProgramDefinition();

        {
            auto writer = UniformDataWriter(progDef.getUniform(_lightCountUniformName).value());
            writer.load(std::move(_lightCount));
            writer.set("pointLights", updatePointLights());
            _lightCount = std::move(writer.finish());
        }

        {
            auto writer = UniformDataWriter(progDef.getUniform(_lightDataUniformName).value());
            writer.load(std::move(_lightData));
            auto& registry = _scene->getRegistry();
            auto ambientLights = _cam->createEntityView<AmbientLight>(registry);
            glm::vec3 ambientColor;
            for (auto entity : ambientLights)
            {
                auto& ambientLight = registry.get<const AmbientLight>(entity);
                ambientColor += ambientLight.getIntensity();
            }
            writer.set("ambientLightColor", ambientColor);
            _lightData = std::move(writer.finish());
        }
    }

    bgfx::ViewId PhongLightRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept
    {
        if (!_scene)
        {
            return;
        }

        auto& progDef = getProgramDefinition();
        if (isValid(_lightCountUniform))
        {
            encoder.setUniform(_lightCountUniform, _lightCount.ptr());
        }

        if (isValid(_lightDataUniform))
        {
            encoder.setUniform(_lightDataUniform, _lightData.ptr());
        }
        if (isValid(_pointLightBuffer))
        {
            auto buffer = progDef.getBuffer(_pointLightsBufferName);
            if (buffer)
            {
                bgfx::setBuffer(buffer->stage, _pointLightBuffer, bgfx::Access::Read);
            }
        }
    }
}