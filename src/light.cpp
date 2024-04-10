#include <darmok/light.hpp>
#include <darmok/transform.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/material.hpp>

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


    PhongLightingComponent::PhongLightingComponent() noexcept
        : _lightCountUniform{ bgfx::kInvalidHandle }
        , _lightDataUniform{ bgfx::kInvalidHandle }
    {
        _pointLightsLayout.begin()
            .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float, true)
            .add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Float, true)
            .end();
    }

    const uint8_t _phongPointLightsBufferStage = 6;
    const std::string _phongLightCountUniformName = "u_lightCount";
    const std::string _phongLightDataUniformName = "u_lightingData";

    void PhongLightingComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _cam = cam;

        _pointLightBuffer = bgfx::createDynamicVertexBuffer(1, _pointLightsLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
        _lightCountUniform = bgfx::createUniform(_phongLightCountUniformName.c_str(), bgfx::UniformType::Vec4);
        _lightDataUniform = bgfx::createUniform(_phongLightDataUniformName.c_str(), bgfx::UniformType::Vec4);
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
    }

    size_t PhongLightingComponent::updatePointLights() noexcept
    {
        auto& registry = _scene->getRegistry();
        auto pointLights = _cam->createEntityView<PointLight>(registry);

        auto size = _pointLightsLayout.getSize(pointLights.size_hint());
        if (_pointLights.size() < size)
        {
            _pointLights = Data(size);
        }

        size_t index = 0;
        for (auto entity : pointLights)
        {
            auto& pointLight = registry.get<const PointLight>(entity);
            auto trans = registry.try_get<const Transform>(entity);
            if (trans != nullptr)
            {
                bgfx::vertexPack(glm::value_ptr(trans->getPosition()), false, bgfx::Attrib::Position, _pointLightsLayout, _pointLights.ptr(), index);
            }
            auto c = Colors::normalize(pointLight.getDiffuseColor());
            bgfx::vertexPack(glm::value_ptr(c), true, bgfx::Attrib::Color0, _pointLightsLayout, _pointLights.ptr(), index);
            c = Colors::normalize(pointLight.getSpecularColor());
            bgfx::vertexPack(glm::value_ptr(c), true, bgfx::Attrib::Color1, _pointLightsLayout, _pointLights.ptr(), index);
            index++;
        }
        size = _pointLightsLayout.getSize(index);
        bgfx::update(_pointLightBuffer, 0, bgfx::makeRef(_pointLights.ptr(), size));

        return index;
    }

    void PhongLightingComponent::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return;
        }
        _lightCount.x = updatePointLights();


        auto& registry = _scene->getRegistry();
        auto ambientLights = _cam->createEntityView<AmbientLight>(registry);
        _lightData = glm::vec4(0.F);
        for (auto entity : ambientLights)
        {
            auto& ambientLight = registry.get<const AmbientLight>(entity);
            _lightData += glm::vec4(ambientLight.getIntensity(), 0.F);
        }
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
        if (isValid(_pointLightBuffer))
        {
            bgfx::setBuffer(_phongPointLightsBufferStage, _pointLightBuffer, bgfx::Access::Read);
        }
    }
}