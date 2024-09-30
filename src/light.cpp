#include <darmok/light.hpp>
#include <darmok/transform.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/material.hpp>
#include <darmok/vertex.hpp>
#include <darmok/scene.hpp>
#include <darmok/math.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/component_wise.hpp>
#include "render_samplers.hpp"

namespace darmok
{
    PointLight::PointLight(float intensity, const Color3& color, float radius) noexcept
        : _intensity(intensity)
        , _color(color)
        , _radius(radius)
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

    PointLight& PointLight::setColor(const Color3& color) noexcept
    {
        _color = color;
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

    const Color3& PointLight::getColor() const noexcept
    {
        return _color;
    }

    DirectionalLight::DirectionalLight(float intensity) noexcept
        : _intensity(intensity)
        , _color(Colors::white3())
    {
    }

    DirectionalLight& DirectionalLight::setIntensity(float intensity) noexcept
    {
        _intensity = intensity;
        return *this;
    }

    DirectionalLight& DirectionalLight::setColor(const Color3& color) noexcept
    {
        _color = color;
        return *this;
    }

    const Color3& DirectionalLight::getColor() const noexcept
    {
        return _color;
    }

    float DirectionalLight::getIntensity() const noexcept
    {
        return _intensity;
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

    SpotLight::SpotLight(float intensity) noexcept
        : _intensity(intensity)
        , _color(Colors::white3())
    {
    }

    SpotLight& SpotLight::setIntensity(float intensity) noexcept
    {
        _intensity = intensity;
        return *this;
    }

    SpotLight& SpotLight::setColor(const Color3& color) noexcept
    {
        _color = color;
        return *this;
    }

    const Color3& SpotLight::getColor() const noexcept
    {
        return _color;
    }

    float SpotLight::getIntensity() const noexcept
    {
        return _intensity;
    }

    LightingRenderComponent::LightingRenderComponent() noexcept
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
            .end();

        _dirLightsLayout.begin()
            .add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .end();

        createHandles();
    }

    LightingRenderComponent::~LightingRenderComponent() noexcept
    {
        destroyHandles();
    }

    void LightingRenderComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _cam = cam;
    }

    void LightingRenderComponent::createHandles() noexcept
    {
        // TODO: does not work in OpenGL as the unform handles need to be created before the program
        _pointLightBuffer = bgfx::createDynamicVertexBuffer(1, _pointLightsLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
        _dirLightBuffer = bgfx::createDynamicVertexBuffer(1, _dirLightsLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
        _lightCountUniform = bgfx::createUniform("u_lightCountVec", bgfx::UniformType::Vec4);
        _lightDataUniform = bgfx::createUniform("u_ambientLightIrradiance", bgfx::UniformType::Vec4);
        _camPosUniform = bgfx::createUniform("u_camPos", bgfx::UniformType::Vec4);
        _normalMatrixUniform = bgfx::createUniform("u_normalMatrix", bgfx::UniformType::Mat3);
    }

    void LightingRenderComponent::destroyHandles() noexcept
    {
        std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms = {
            _lightCountUniform, _lightDataUniform, _camPosUniform, _normalMatrixUniform
        };
        for (auto& uniform : uniforms)
        {
            if (isValid(uniform))
            {
                bgfx::destroy(uniform);
                uniform.get().idx = bgfx::kInvalidHandle;
            }
        }
        std::vector<std::reference_wrapper<bgfx::DynamicVertexBufferHandle>> buffers = {
            _pointLightBuffer, _dirLightBuffer
        };
        for (auto& buffer : buffers)
        {
            if (isValid(buffer))
            {
                bgfx::destroy(buffer);
                buffer.get().idx = bgfx::kInvalidHandle;
            }
        }
    }

    void LightingRenderComponent::shutdown() noexcept
    {
        destroyHandles();
    }

    size_t LightingRenderComponent::updateDirLights() noexcept
    {
        auto lights = _cam->getEntities<DirectionalLight>();
        VertexDataWriter writer(_dirLightsLayout, uint32_t(lights.size_hint()));
        uint32_t index = 0;

        for (auto entity : lights)
        {
            auto& light = _scene->getComponent<const DirectionalLight>(entity).value();
            auto trans = _scene->getComponent<const Transform>(entity);
            if (trans)
            {
                auto dir = trans->getWorldRotation() * glm::vec3(0, 0, 1);
                writer.write(bgfx::Attrib::Normal, index, dir);
            }
            auto intensity = light.getIntensity();
            glm::vec4 c(Colors::normalize(light.getColor()) * intensity, 0);
            writer.write(bgfx::Attrib::Color0, index, c);

            ++index;
        }
        auto data = writer.finish();
        if (!data.empty())
        {
            bgfx::update(_dirLightBuffer, 0, data.copyMem());
        }

        return index;
    }

    size_t LightingRenderComponent::updatePointLights() noexcept
    {
        auto entities = _cam->getEntities<PointLight>();
        VertexDataWriter writer(_pointLightsLayout, uint32_t(entities.size_hint()));
        uint32_t index = 0;

        for (auto entity : entities)
        {
            auto& light = _scene->getComponent<const PointLight>(entity).value();
            auto trans = _scene->getComponent<const Transform>(entity);
            float scale = 1.F;
            if (trans)
            {
                auto pos = trans->getWorldPosition();
                scale = glm::compMax(trans->getWorldScale());
                writer.write(bgfx::Attrib::Position, index, pos);
            }
            auto radius = light.getRadius() * scale;
            auto intensity = light.getIntensity();
            glm::vec4 c(Colors::normalize(light.getColor()) * intensity, radius);
            writer.write(bgfx::Attrib::Color0, index, c);
            ++index;
        }
        auto data = writer.finish();
        if (!data.empty())
        {
            bgfx::update(_pointLightBuffer, 0, data.copyMem());
        }

        return index;
    }

    void LightingRenderComponent::updateAmbientLights() noexcept
    {
        auto entities = _cam->getEntities<AmbientLight>();
        _lightData = glm::vec4(0.F);
        for (auto entity : entities)
        {
            auto& ambientLight = _scene->getComponent<const AmbientLight>(entity).value();
            auto c = Colors::normalize(ambientLight.getColor()) * ambientLight.getIntensity();
            _lightData += glm::vec4(c, 0.F);
        }
    }

    void LightingRenderComponent::updateCamera() noexcept
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

    void LightingRenderComponent::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return;
        }
        _lightCount.x = float(updatePointLights());
        _lightCount.y = float(updateDirLights());
        updateAmbientLights();
        updateCamera();
    }

    void LightingRenderComponent::beforeRenderEntity(Entity entity, IRenderGraphContext& context) noexcept
    {
        auto& encoder = context.getEncoder();
        encoder.setUniform(_lightCountUniform, glm::value_ptr(_lightCount));
        encoder.setUniform(_lightDataUniform, glm::value_ptr(_lightData));
        encoder.setBuffer(RenderSamplers::LIGHTS_POINT, _pointLightBuffer, bgfx::Access::Read);
        encoder.setBuffer(RenderSamplers::LIGHTS_DIR, _dirLightBuffer, bgfx::Access::Read);
        encoder.setUniform(_camPosUniform, glm::value_ptr(_camPos));

        glm::mat3 normalMatrix(1);
        if (auto trans = _scene->getComponent<Transform>(entity))
        {
            normalMatrix = glm::transpose(glm::adjugate(glm::mat3(trans->getWorldMatrix())));
        }
        encoder.setUniform(_normalMatrixUniform, glm::value_ptr(normalMatrix));
    }
}