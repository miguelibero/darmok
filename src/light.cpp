#include <darmok/light.hpp>
#include <darmok/vertex.hpp>
#include <darmok/transform.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/material.hpp>

namespace darmok
{
    PointLight::PointLight(const glm::vec3& intensity, float radius) noexcept
        : _intensity(intensity)
        , _radius(radius)
        , _diffuseColor(Colors::black)
        , _specularColor(Colors::black)
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

    const ProgramDefinition& LightRenderUpdater::getPhongProgramDefinition() noexcept
    {
        static ProgramDefinition def{
            {},
            {
                {ProgramUniform::LightCount, { "u_lightCount", bgfx::UniformType::Vec4 }},
                {ProgramUniform::AmbientLightColor, { "u_ambientLightColor", bgfx::UniformType::Vec4 }}
            },
            {},
            {
                {ProgramBuffer::PointLights, { 6, {
                    { bgfx::Attrib::Position,   { bgfx::AttribType::Float, 3} },
                    { bgfx::Attrib::Color0,     { bgfx::AttribType::Float, 3, true} },
                    { bgfx::Attrib::Color1,     { bgfx::AttribType::Float, 3, true} },
                }}
            }}
        };
        return def;
    }

    static bgfx::VertexLayout _pointLightLayout = LightRenderUpdater::getPhongProgramDefinition().buffers.at(ProgramBuffer::PointLights).createVertexLayout();

    LightRenderUpdater::LightRenderUpdater() noexcept
        : _pointLightsBuffer{ bgfx::kInvalidHandle }
        , _countUniform{ bgfx::kInvalidHandle }
        , _ambientIntensityUniform{ bgfx::kInvalidHandle }
        , _lightCount{}
        , _ambientColor{}
    {
    }

    void LightRenderUpdater::init(Scene& scene, App& app) noexcept
    {
        _scene = scene;
        auto& progDef = getPhongProgramDefinition();
        _countUniform = progDef.uniforms.at(ProgramUniform::LightCount).createHandle();
        _ambientIntensityUniform = progDef.uniforms.at(ProgramUniform::AmbientLightColor).createHandle();
        _pointLightsBuffer = bgfx::createDynamicVertexBuffer(
            1, _pointLightLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
    }

    void LightRenderUpdater::shutdown() noexcept
    {
        bgfx::destroy(_countUniform);
        bgfx::destroy(_ambientIntensityUniform);
        bgfx::destroy(_pointLightsBuffer);
    }

    void LightRenderUpdater::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return;
        }
        auto& registry = _scene->getRegistry();
        auto cams = registry.view<const Camera>();
        auto& pointLights = registry.storage<PointLight>();

        for (auto [camEntity, cam] : cams.each())
        {
            EntityRuntimeView camPointLights;
            camPointLights.iterate(pointLights);
            cam.filterEntityView(camPointLights);

            uint32_t index = 0;
            VertexDataWriter writer(_pointLightLayout, pointLights.size());

            for (auto entity : camPointLights)
            {
                auto& pointLight = registry.get<const PointLight>(entity);
                auto trans = registry.try_get<const Transform>(entity);
                if (trans != nullptr)
                {
                    writer.set(bgfx::Attrib::Position, index, trans->getPosition());
                }
                writer.set(bgfx::Attrib::Color0, index, pointLight.getDiffuseColor());
                writer.set(bgfx::Attrib::Color1, index, pointLight.getSpecularColor());
                index++;
            }
            _lightCount.x = index;
            
            _pointLights.emplace(camEntity, writer.finish());
        }

        _ambientColor = {};
        auto ambientLights = registry.view<const AmbientLight>();
        for (auto [entity, ambientLight] : ambientLights.each())
        {
            _ambientColor += glm::vec4(ambientLight.getIntensity(), 0);
        }
        _ambientColor /= Colors::white;
    }

    bool LightRenderUpdater::bgfxConfig(const Camera& cam, const ProgramDefinition& progDef, bgfx::Encoder& encoder) const noexcept
    {
        auto& lightProgDef = getPhongProgramDefinition();

        if (progDef.hasUniform(ProgramUniform::LightCount, lightProgDef.uniforms))
        {
            encoder.setUniform(_countUniform, glm::value_ptr(_lightCount));
        }

        if (progDef.hasUniform(ProgramUniform::AmbientLightColor, lightProgDef.uniforms))
        {
            encoder.setUniform(_ambientIntensityUniform, glm::value_ptr(_ambientColor));
        }

        if (_scene)
        {
            auto& registry = _scene->getRegistry();
            auto& pointLightsBufferDef = lightProgDef.buffers.at(ProgramBuffer::PointLights);
            if (progDef.hasBuffer(ProgramBuffer::PointLights, pointLightsBufferDef))
            {
                auto camEntity = entt::to_entity(registry, cam);
                auto itr = _pointLights.find(camEntity);
                if (itr != _pointLights.end())
                {
                    bgfx::update(_pointLightsBuffer, 0, itr->second.makeRef());
                    bgfx::setBuffer(pointLightsBufferDef.stage, _pointLightsBuffer, bgfx::Access::Read);
                }
            }
        }

        return false;
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