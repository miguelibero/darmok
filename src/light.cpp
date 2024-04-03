#include <darmok/light.hpp>
#include <darmok/vertex.hpp>
#include <darmok/transform.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/material.hpp>

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

    const ProgramDefinition& LightRenderUpdater::getPhongProgramDefinition()
    {
        static auto def = ProgramDefinition::createFromJson(phong_lighting_progdef);
        return def;
    }

    LightRenderUpdater::LightRenderUpdater() noexcept
        : _countUniform{ bgfx::kInvalidHandle }
        , _ambientIntensityUniform{ bgfx::kInvalidHandle }
        , _lightCount{}
        , _ambientColor{}
    {
    }

    void LightRenderUpdater::init(Scene& scene, App& app) noexcept
    {
        _scene = scene;
        auto& progDef = getPhongProgramDefinition();
        {
            auto itr = progDef.buffers.find(ProgramBuffer::PointLights);
            if (itr != progDef.buffers.end())
            {
                _pointLightLayout = itr->second.createVertexLayout();
            }
        }
        {
            auto itr = progDef.uniforms.find(ProgramUniform::LightCount);
            if (itr != progDef.uniforms.end())
            {
                _countUniform = itr->second.createHandle();
            }
        }
        {
            auto itr = progDef.uniforms.find(ProgramUniform::AmbientLightColor);
            if (itr != progDef.uniforms.end())
            {
                _ambientIntensityUniform = itr->second.createHandle();
            }
        }
    }

    void LightRenderUpdater::shutdown() noexcept
    {
        if (isValid(_countUniform))
        {
            bgfx::destroy(_countUniform);
        }
        if (isValid(_ambientIntensityUniform))
        {
            bgfx::destroy(_ambientIntensityUniform);
        }
        for (auto& elm : _pointLightBuffers)
        {
            if (isValid(elm.second))
            {
                bgfx::destroy(elm.second);
            }
        }
    }

    size_t LightRenderUpdater::updatePointLights(Entity camEntity, const Camera& cam) noexcept
    {
        auto& registry = _scene->getRegistry();
        auto& pointLights = registry.storage<PointLight>();
        EntityRuntimeView camPointLights;
        camPointLights.iterate(pointLights);
        cam.filterEntityView(camPointLights);

        VertexDataWriter writer(_pointLightLayout, pointLights.size());

        auto itr = _pointLights.find(camEntity);
        if (itr != _pointLights.end())
        {
            writer.load(std::move(itr->second));
        }
        else
        {
            itr = _pointLights.emplace(camEntity, Data()).first;
        }

        size_t index = 0;
        for (auto entity : camPointLights)
        {
            auto& pointLight = registry.get<const PointLight>(entity);
            auto trans = registry.try_get<const Transform>(entity);
            if (trans != nullptr)
            {
                writer.set(bgfx::Attrib::Position, index, trans->getPosition());
            }
            writer.set(bgfx::Attrib::Color0, index, Colors::normalize(pointLight.getDiffuseColor()));
            writer.set(bgfx::Attrib::Color1, index, Colors::normalize(pointLight.getSpecularColor()));
            index++;
        }

        itr->second = writer.finish();

        if (index > 0)
        {
            auto itr2 = _pointLightBuffers.find(camEntity);
            bgfx::DynamicVertexBufferHandle buffer;
            if (itr2 == _pointLightBuffers.end())
            {
                buffer = bgfx::createDynamicVertexBuffer(index, _pointLightLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
                _pointLightBuffers.emplace(camEntity, buffer);
            }
            else
            {
                buffer = itr2->second;
            }
            bgfx::update(buffer, 0, itr->second.makeRef());
        }

        return index;
    }

    void LightRenderUpdater::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return;
        }
        auto& registry = _scene->getRegistry();
        auto cams = registry.view<const Camera>();

        for (auto [camEntity, cam] : cams.each())
        {
            _lightCount.x = updatePointLights(camEntity, cam);
        }

        _ambientColor = {};
        auto ambientLights = registry.view<const AmbientLight>();
        for (auto [entity, ambientLight] : ambientLights.each())
        {
            _ambientColor += glm::vec4(ambientLight.getIntensity(), 0);
        }
        _ambientColor /= Colors::white;
    }

    void LightRenderUpdater::bgfxConfig(const Camera& cam, const ProgramDefinition& progDef, bgfx::Encoder& encoder) const noexcept
    {
        if (!_scene)
        {
            return;
        }

        auto& lightProgDef = getPhongProgramDefinition();
        if (progDef.hasUniform(ProgramUniform::LightCount, lightProgDef.uniforms))
        {
            encoder.setUniform(_countUniform, glm::value_ptr(_lightCount));
        }

        if (progDef.hasUniform(ProgramUniform::AmbientLightColor, lightProgDef.uniforms))
        {
            encoder.setUniform(_ambientIntensityUniform, glm::value_ptr(_ambientColor));
        }

        auto& registry = _scene->getRegistry();
        if (_lightCount.x > 0)
        {
            auto& pointLightsBufferDef = lightProgDef.buffers.at(ProgramBuffer::PointLights);
            if (progDef.hasBuffer(ProgramBuffer::PointLights, pointLightsBufferDef))
            {
                auto camEntity = entt::to_entity(registry, cam);
                auto itr = _pointLightBuffers.find(camEntity);
                if (itr != _pointLightBuffers.end())
                {
                    bgfx::setBuffer(pointLightsBufferDef.stage, itr->second, bgfx::Access::Read);
                }
            }
        }
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
}