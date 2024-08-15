#include <darmok/light.hpp>
#include <darmok/transform.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/material.hpp>
#include <darmok/vertex.hpp>
#include <darmok/scene.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/component_wise.hpp>
#include "render_samplers.hpp"

namespace darmok
{
    PointLight::PointLight(float intensity) noexcept
        : _intensity(intensity)
        , _diffuseColor(Colors::white3())
        , _specularColor(Colors::white3())
    {
    }

    PointLight& PointLight::setIntensity(float intensity) noexcept
    {
        _intensity = intensity;
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
        // could be calculated from the attenuation
        float v = glm::max(1.0f, 0.05f * _intensity) / _intensity;
        return 1.0f / sqrtf(v);
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
        , _normalMatrixUniform{ bgfx::kInvalidHandle }
        , _lightCount(0)
        , _lightData(0)
        , _camPos(0)
    {
        _pointLightsLayout.begin()
            .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Float)
            .end();

        createHandles();
    }

    PhongLightingComponent::~PhongLightingComponent() noexcept
    {
        destroyHandles();
    }

    void PhongLightingComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _cam = cam;
    }

    void PhongLightingComponent::createHandles() noexcept
    {
        // TODO: does not work in OpenGL as the unfirm handles need to be created before the program
        _pointLightBuffer = bgfx::createDynamicVertexBuffer(1, _pointLightsLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
        _lightCountUniform = bgfx::createUniform("u_lightCountVec", bgfx::UniformType::Vec4);
        _lightDataUniform = bgfx::createUniform("u_ambientLightIrradiance", bgfx::UniformType::Vec4);
        _camPosUniform = bgfx::createUniform("u_camPos", bgfx::UniformType::Vec4);
        _normalMatrixUniform = bgfx::createUniform("u_normalMatrix", bgfx::UniformType::Mat3);
    }

    void PhongLightingComponent::destroyHandles() noexcept
    {
        std::vector<std::reference_wrapper<bgfx::UniformHandle>> handles = {
            _lightCountUniform, _lightDataUniform, _camPosUniform, _normalMatrixUniform
        };
        for (auto& handle : handles)
        {
            if (isValid(handle))
            {
                bgfx::destroy(handle);
                handle.get().idx = bgfx::kInvalidHandle;
            }
        }
        if (isValid(_lightDataUniform))
        {
            bgfx::destroy(_lightDataUniform);
            _lightDataUniform.idx = bgfx::kInvalidHandle;
        }
        if (isValid(_pointLightBuffer))
        {
            bgfx::destroy(_pointLightBuffer);
            _pointLightBuffer.idx = bgfx::kInvalidHandle;
        }
    }

    void PhongLightingComponent::shutdown() noexcept
    {
        destroyHandles();
    }

    size_t PhongLightingComponent::updatePointLights() noexcept
    {
        auto& registry = _scene->getRegistry();
        auto pointLights = _cam->createEntityView<PointLight>(registry);

        // TODO: not sure if size_hint is accurate
        VertexDataWriter writer(_pointLightsLayout, uint32_t(pointLights.size_hint()));
        writer.load(std::move(_pointLights));

        uint32_t index = 0;
        for (auto entity : pointLights)
        {
            auto& pointLight = registry.get<const PointLight>(entity);
            auto trans = registry.try_get<const Transform>(entity);
            if (trans != nullptr)
            {
                auto pos = trans->getWorldPosition();
                writer.write(bgfx::Attrib::Position, index, pos);
            }
            glm::vec4 c(
                Colors::normalize(pointLight.getDiffuseColor()) * pointLight.getIntensity(),
                pointLight.getRadius());
            writer.write(bgfx::Attrib::Color0, index, c);
            c = glm::vec4(
                Colors::normalize(pointLight.getSpecularColor()) * pointLight.getIntensity(),
                pointLight.getRadius());
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
            auto c = Colors::normalize(ambientLight.getColor()) * ambientLight.getIntensity();
            _lightData += glm::vec4(c, 0.F);
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
        auto camEntity = entt::to_entity(registry.storage<Camera>(), _cam.value());
        if (camEntity != entt::null)
        {
            auto camTrans = registry.try_get<const Transform>(camEntity);
            if (camTrans != nullptr)
            {
                _camPos = glm::vec4(camTrans->getWorldPosition(), 0);
            }
        }
    }

    void PhongLightingComponent::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return;
        }
        _lightCount.x = float(updatePointLights());
        updateAmbientLights();
        updateCamera();
    }

    void PhongLightingComponent::beforeRenderEntity(Entity entity, bgfx::Encoder& encoder) noexcept
    {
        encoder.setUniform(_lightCountUniform, glm::value_ptr(_lightCount));
        encoder.setUniform(_lightDataUniform, glm::value_ptr(_lightData));
        encoder.setBuffer(RenderSamplers::LIGHTS_POINTLIGHTS, _pointLightBuffer, bgfx::Access::Read);
        encoder.setUniform(_camPosUniform, glm::value_ptr(_camPos));

        if (auto trans = _scene->getComponent<Transform>(entity))
        {
            auto mtx = glm::transpose(glm::adjugate(glm::mat3(trans->getWorldMatrix())));
            encoder.setUniform(_normalMatrixUniform, glm::value_ptr(mtx));
        }
    }
}