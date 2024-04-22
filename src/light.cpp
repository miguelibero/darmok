#include <darmok/light.hpp>
#include <darmok/transform.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/material.hpp>
#include <darmok/vertex.hpp>

namespace darmok
{
    PointLight::PointLight(float intensity) noexcept
        : _intensity(intensity)
        , _attenuation(0)
        , _radius(0)
        , _diffuseColor(Colors::white3())
        , _specularColor(Colors::white3())
    {
    }

    PointLight& PointLight::setIntensity(float intensity) noexcept
    {
        _intensity = intensity;
        return *this;
    }

    PointLight& PointLight::setRadius(float radius) noexcept
    {
        _radius = radius;
        return *this;
    }

    PointLight& PointLight::setAttenuation(const glm::vec3& attn) noexcept
    {
        _attenuation = attn;
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

    float PointLight::getIntensity() const noexcept
    {
        return _intensity;
    }

    float PointLight::getRadius() const noexcept
    {
        return _radius;
    }

    const glm::vec3& PointLight::getAttenuation() const noexcept
    {
        return _attenuation;
    }

    const Color3& PointLight::getDiffuseColor() const noexcept
    {
        return _diffuseColor;
    }

    const Color3& PointLight::getSpecularColor() const noexcept
    {
        return _specularColor;
    }

    AmbientLight::AmbientLight(float intensity) noexcept
        : _intensity(intensity)
        , _color(Colors::white3())
    {
    }

    AmbientLight& AmbientLight::setIntensity(float intensity) noexcept
    {
        _intensity = intensity;
        return *this;
    }

    AmbientLight& AmbientLight::setColor(const Color3& color) noexcept
    {
        _color = color;
        return *this;
    }

    const Color3& AmbientLight::getColor() const noexcept
    {
        return _color;
    }

    float AmbientLight::getIntensity() const noexcept
    {
        return _intensity;
    }

    PhongLightingComponent::PhongLightingComponent() noexcept
        : _lightCountUniform{ bgfx::kInvalidHandle }
        , _lightDataUniform{ bgfx::kInvalidHandle }
        , _camPosUniform{ bgfx::kInvalidHandle }
        , _pointLightBuffer{ bgfx::kInvalidHandle }
        , _lightCount(0)
        , _lightData(0)
        , _camPos(0)
    {
        _pointLightsLayout.begin()
            .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Float)
            .end();
    }

    const uint8_t _phongPointLightsBufferStage = 6;
    const std::string _phongLightCountUniformName = "u_lightCount";
    const std::string _phongLightDataUniformName = "u_lightingData";
    const std::string _phongCamPosUniformName = "u_camPos";

    void PhongLightingComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _cam = cam;

        _pointLightBuffer = bgfx::createDynamicVertexBuffer(1, _pointLightsLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
        _lightCountUniform = bgfx::createUniform(_phongLightCountUniformName.c_str(), bgfx::UniformType::Vec4);
        _lightDataUniform = bgfx::createUniform(_phongLightDataUniformName.c_str(), bgfx::UniformType::Vec4);
        _camPosUniform = bgfx::createUniform(_phongCamPosUniformName.c_str(), bgfx::UniformType::Vec4);
    }

    void PhongLightingComponent::shutdown() noexcept
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
        if (isValid(_camPosUniform))
        {
            bgfx::destroy(_camPosUniform);
        }
    }

    size_t PhongLightingComponent::updatePointLights() noexcept
    {
        auto& registry = _scene->getRegistry();
        auto pointLights = _cam->createEntityView<PointLight>(registry);

        // TODO: not sure if size_hint is accurate
        VertexDataWriter writer(_pointLightsLayout, pointLights.size_hint());
        writer.load(std::move(_pointLights));

        size_t index = 0;
        for (auto entity : pointLights)
        {
            auto& pointLight = registry.get<const PointLight>(entity);
            auto trans = registry.try_get<const Transform>(entity);
            if (trans != nullptr)
            {
                writer.write(bgfx::Attrib::Position, index, trans->getPosition());
            }
            auto c = Colors::normalize(pointLight.getDiffuseColor()) * pointLight.getIntensity();
            writer.write(bgfx::Attrib::Color0, index, c);
            c = Colors::normalize(pointLight.getSpecularColor()) * pointLight.getIntensity();
            writer.write(bgfx::Attrib::Color1, index, c);
            index++;
        }
        _pointLights = writer.finish();
        auto ptr = _pointLights.ptr();
        if (ptr != nullptr)
        {
            auto size = _pointLightsLayout.getSize(index);
            bgfx::update(_pointLightBuffer, 0, bgfx::makeRef(ptr, size));
        }

        return index;
    }

    void PhongLightingComponent::updateAmbientLights() noexcept
    {
        auto& registry = _scene->getRegistry();
        auto ambientLights = _cam->createEntityView<AmbientLight>(registry);
        _lightData = glm::vec4(0.F);
        for (auto entity : ambientLights)
        {
            auto& ambientLight = registry.get<const AmbientLight>(entity);
            auto c = Colors::normalize(ambientLight.getColor());
            _lightData += glm::vec4(c, 0.F) * ambientLight.getIntensity();
        }
    }

    void PhongLightingComponent::updateCamera() noexcept
    {
        _camPos = glm::vec4(0);
        if (!_cam || !_scene)
        {
            return;
        }
        auto& registry = _scene->getRegistry();
        auto camEntity = entt::to_entity(registry, _cam.value());
        auto camTrans = registry.try_get<const Transform>(camEntity);
        if (camTrans != nullptr)
        {
            _camPos = glm::vec4(camTrans->getPosition(), 0);
        }
    }


    void PhongLightingComponent::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return;
        }
        _lightCount.x = updatePointLights();
        updateAmbientLights();
        updateCamera();
    }

    void PhongLightingComponent::bgfxConfig(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept
    {
        if (!_scene)
        {
            return;
        }
        if (isValid(_lightCountUniform))
        {
            encoder.setUniform(_lightCountUniform, glm::value_ptr(_lightCount));
        }
        if (isValid(_lightDataUniform))
        {
            encoder.setUniform(_lightDataUniform, glm::value_ptr(_lightData));
        }
        if (isValid(_camPosUniform))
        {
            encoder.setUniform(_camPosUniform, glm::value_ptr(_camPos));
        }
        if (isValid(_pointLightBuffer))
        {
            bgfx::setBuffer(_phongPointLightsBufferStage, _pointLightBuffer, bgfx::Access::Read);
        }
    }
}