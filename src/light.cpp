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

    float PointLight::getRadius() const noexcept
    {
        return _radius;
    }

    const glm::vec3& PointLight::getIntensity() const noexcept
    {
        return _intensity;
    }

    const ProgramDefinition& LightRenderUpdater::getProgramDefinition() noexcept
    {
        static ProgramDefinition def{
            {},
            {
                {ProgramUniform::LightCount, { "u_lightCount", bgfx::UniformType::Vec4 }},
                {ProgramUniform::AmbientLightIntensity, { "u_ambientLightIntensity", bgfx::UniformType::Vec4 }}
            },
            {
                {ProgramBuffer::PointLights, { 6, {
                    { bgfx::Attrib::TexCoord0, { bgfx::AttribType::Float, 4} },
                    { bgfx::Attrib::TexCoord1, { bgfx::AttribType::Float, 4} },
                }}
            }}
        };
        return def;
    }

    static bgfx::VertexLayout _pointLightLayout = LightRenderUpdater::getProgramDefinition().buffers.at(ProgramBuffer::PointLights).createVertexLayout();

    LightRenderUpdater::LightRenderUpdater() noexcept
        : _pointLightsBuffer{ bgfx::kInvalidHandle }
        , _countUniform{ bgfx::kInvalidHandle }
        , _ambientIntensityUniform{ bgfx::kInvalidHandle }
        , _lightCount{}
        , _ambientIntensity{}
    {
    }

    void LightRenderUpdater::init(Scene& scene, App& app) noexcept
    {
        _scene = scene;
        auto& progDef = LightRenderUpdater::getProgramDefinition();
        _countUniform = progDef.uniforms.at(ProgramUniform::LightCount).createHandle();
        _ambientIntensityUniform = progDef.uniforms.at(ProgramUniform::AmbientLightIntensity).createHandle();
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
        _pointLightsBufferCamera = entt::null;
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
                    auto v = glm::vec4(trans->getPosition(), 0);
                    writer.set(bgfx::Attrib::TexCoord0, index, glm::value_ptr(v));
                }
                auto v = glm::vec4(pointLight.getIntensity(), pointLight.getRadius());
                writer.set(bgfx::Attrib::TexCoord1, index, glm::value_ptr(v));
                index++;
            }
            _lightCount.x = index;
            
            _pointLights.emplace(std::make_pair(camEntity, std::move(writer.release())));
        }

        _ambientIntensity = {};
        auto ambientLights = registry.view<const AmbientLight>();
        for (auto [entity, ambientLight] : ambientLights.each())
        {
            _ambientIntensity += glm::vec4(ambientLight.getIntensity(), 0);
        }
    }

    bool LightRenderUpdater::bgfxConfig(const Camera& cam, const Material& mat, bgfx::Encoder& encoder) noexcept
    {
        auto& progDef = mat.getProgramDefinition();
        auto& lightProgDef = LightRenderUpdater::getProgramDefinition();

        if (progDef.hasUniform(ProgramUniform::LightCount, lightProgDef.uniforms))
        {
            encoder.setUniform(_countUniform, glm::value_ptr(_lightCount));
        }

        if (progDef.hasUniform(ProgramUniform::AmbientLightIntensity, lightProgDef.uniforms))
        {
            encoder.setUniform(_ambientIntensityUniform, glm::value_ptr(_ambientIntensity));
        }

        if (_scene)
        {
            auto& registry = _scene->getRegistry();
            auto& pointLightsBufferDef = LightRenderUpdater::getProgramDefinition().buffers.at(ProgramBuffer::PointLights);
            if (progDef.hasBuffer(ProgramBuffer::PointLights, pointLightsBufferDef))
            {
                auto camEntity = entt::to_entity(registry, cam);
                auto itr = _pointLights.find(camEntity);
                if (itr != _pointLights.end())
                {
                    if (_pointLightsBufferCamera != camEntity)
                    {
                        bgfx::update(_pointLightsBuffer, 0, itr->second.makeRef());
                        _pointLightsBufferCamera = camEntity;
                    }
                    bgfx::setBuffer(pointLightsBufferDef.stage, _pointLightsBuffer, bgfx::Access::Read);
                }
            }
        }

        return false;
    }

    AmbientLight::AmbientLight(const glm::vec3& intensity) noexcept
        : _intensity(intensity)
    {
    }

    AmbientLight& AmbientLight::setIntensity(const glm::vec3& intensity) noexcept
    {
        _intensity = intensity;
        return *this;
    }

    [[nodiscard]] const glm::vec3& AmbientLight::getIntensity() const noexcept
    {
        return _intensity;
    }
}