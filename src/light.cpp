#include <darmok/light.hpp>
#include <darmok/transform.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/material.hpp>
#include <darmok/vertex.hpp>
#include <darmok/scene.hpp>
#include <darmok/math.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/glm_serialize.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/component_wise.hpp>
#include "detail/render_samplers.hpp"

namespace darmok
{
    PointLight::PointLight(float intensity, const Color3& color, float range) noexcept
        : _intensity{ intensity }
        , _color{ color }
        , _range{ range }
        , _shadow{ LightDefinition::NoShadow }
    {
    }

    PointLight& PointLight::setIntensity(float intensity) noexcept
    {
        _intensity = intensity;
        return *this;
    }

    PointLight& PointLight::setRange(float range) noexcept
    {
        _range = range;
        return *this;
    }

    PointLight& PointLight::setColor(const Color3& color) noexcept
    {
        _color = color;
        return *this;
    }

    PointLight& PointLight::setShadowType(ShadowType type) noexcept
    {
        _shadow = type;
        return *this;
    }

    float PointLight::getIntensity() const noexcept
    {
        return _intensity;
    }

    float PointLight::getRange() const noexcept
    {
        return _range;
    }

    const Color3& PointLight::getColor() const noexcept
    {
        return _color;
    }

    ShadowType PointLight::getShadowType() const noexcept
    {
        return _shadow;
    }

    expected<void, std::string> PointLight::load(const Definition& def)
    {
		setIntensity(def.intensity());
		setColor(convert<Color3>(def.color()));
		setRange(def.range());
		setShadowType(def.shadow_type());
        return {};
    }

    PointLight::Definition PointLight::createDefinition() noexcept
    {
        Definition def;
        def.set_intensity(1.f);
        *def.mutable_color() = convert<protobuf::Color3>(Colors::white3());
        return def;
    }

    DirectionalLight::DirectionalLight(float intensity) noexcept
        : _intensity{ intensity }
        , _color{ Colors::white3() }
        , _shadow{ LightDefinition::NoShadow }
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

    DirectionalLight& DirectionalLight::setShadowType(ShadowType type) noexcept
    {
        _shadow = type;
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

    ShadowType DirectionalLight::getShadowType() const noexcept
    {
        return _shadow;
    }

    expected<void, std::string> DirectionalLight::load(const Definition& def)
    {
        setIntensity(def.intensity());
        setColor(convert<Color3>(def.color()));
        setShadowType(def.shadow_type());
        return {};
    }

    DirectionalLight::Definition DirectionalLight::createDefinition() noexcept
    {
        Definition def;
        def.set_intensity(1.f);
        *def.mutable_color() = convert<protobuf::Color3>(Colors::white3());
        return def;
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

    expected<void, std::string> AmbientLight::load(const Definition& def)
    {
        setIntensity(def.intensity());
        setColor(convert<Color3>(def.color()));
        return {};
    }

    AmbientLight::Definition AmbientLight::createDefinition() noexcept
    {
        Definition def;
        def.set_intensity(1.f);
        *def.mutable_color() = convert<protobuf::Color3>(Colors::white3());
        return def;
    }

    SpotLight::SpotLight(float intensity, const Color3& color, float range) noexcept
        : _intensity{ intensity }
        , _color{ Colors::white3() }
        , _range{ range }
        , _coneAngle{ glm::radians(30.f) }
        , _innerConeAngle{ 0.f }
        , _shadow{ LightDefinition::NoShadow }
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

    SpotLight& SpotLight::setRange(float range) noexcept
    {
        _range = range;
        return *this;
    }

    SpotLight& SpotLight::setConeAngle(float angle) noexcept
    {
        _coneAngle = angle;
        return *this;
    }

    SpotLight& SpotLight::setInnerConeAngle(float angle) noexcept
    {
        _innerConeAngle = angle;
        return *this;
    }

    SpotLight& SpotLight::setShadowType(ShadowType type) noexcept
    {
        _shadow = type;
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

    float SpotLight::getRange() const noexcept
    {
        return _range;
    }

    float SpotLight::getConeAngle() const noexcept
    {
        return _coneAngle;
    }

    float SpotLight::getInnerConeAngle() const noexcept
    {
        return _innerConeAngle;
    }

    ShadowType SpotLight::getShadowType() const noexcept
    {
        return _shadow;
    }

    expected<void, std::string> SpotLight::load(const Definition& def)
    {
        setIntensity(def.intensity());
        setColor(convert<Color3>(def.color()));
        setRange(def.range());
        setInnerConeAngle(def.inner_cone_angle());
        setConeAngle(def.cone_angle());
        setShadowType(def.shadow_type());
        return {};
    }

    SpotLight::Definition SpotLight::createDefinition() noexcept
    {
        Definition def;
        def.set_intensity(1.f);
        *def.mutable_color() = convert<protobuf::Color3>(Colors::white3());
        return def;
    }

    LightingRenderComponent::Definition LightingRenderComponent::createDefinition() noexcept
    {
        Definition def;
        return def;
    }

    LightingRenderComponent::LightingRenderComponent() noexcept
        : _lightCount(0)
        , _lightData(0)
        , _camPos(0)
    {
        _pointLightsLayout.begin()
            .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .end();

        _spotLightsLayout.begin()
            .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Weight, 4, bgfx::AttribType::Float)
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

    expected<void, std::string> LightingRenderComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _cam = cam;
        return {};
    }

    void LightingRenderComponent::createHandles() noexcept
    {
        _pointLightBuffer = { 1, _pointLightsLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE };
        _dirLightBuffer = { 1, _dirLightsLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE };
        _spotLightBuffer = { 1, _spotLightsLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE };
        _lightCountUniform = { "u_lightCountVec", bgfx::UniformType::Vec4 };
        _lightDataUniform = { "u_ambientLightIrradiance", bgfx::UniformType::Vec4 };
        _camPosUniform = { "u_camPos", bgfx::UniformType::Vec4 };
        _normalMatrixUniform = { "u_normalMatrix", bgfx::UniformType::Mat3 };
    }

    void LightingRenderComponent::destroyHandles() noexcept
    {
        _lightCountUniform.reset();
        _lightDataUniform.reset();
        _camPosUniform.reset();
        _normalMatrixUniform.reset();
        _pointLightBuffer.reset();
        _dirLightBuffer.reset();
        _spotLightBuffer.reset();
    }

    expected<void, std::string> LightingRenderComponent::shutdown() noexcept
    {
        destroyHandles();
        return {};
    }

    struct DirectionalLightBufferElement final
    {
        glm::vec4 dir{ 0.f };
        glm::vec3 color{ 1.f };
    };

    size_t LightingRenderComponent::updateDirLights() noexcept
    {
        std::vector<DirectionalLightBufferElement> elms;
        for (auto entity : _cam->getEntities<DirectionalLight>())
        {
            auto& elm = elms.emplace_back();
            auto trans = _scene->getComponent<const Transform>(entity);
            if (trans)
            {
                // elm.dir = -glm::normalize(trans->getWorldPosition());
                elm.dir = glm::vec4{ trans->getWorldDirection(), 0.f };
            }
            elm.dir.w = float(entity);
            auto& light = _scene->getComponent<const DirectionalLight>(entity).value();
            auto intensity = light.getIntensity();
            elm.color = Colors::normalize(light.getColor()) * intensity;
        }

        VertexDataWriter writer{ _dirLightsLayout, uint32_t(elms.size()) };
        uint32_t index = 0;

        for (auto& elm : elms)
        {
            writer.write(bgfx::Attrib::Normal, index, elm.dir);
            writer.write(bgfx::Attrib::Color0, index, elm.color);
            ++index;
        }
        auto data = writer.finish();
        if (!data.empty())
        {
            bgfx::update(_dirLightBuffer, 0, data.copyMem());
        }

        return index;
    }

    struct PointLightBufferElement final
    {
        glm::vec4 pos{ 0.f };
        glm::vec4 color{ 1.f };
    };

    size_t LightingRenderComponent::updatePointLights() noexcept
    {
        std::vector<PointLightBufferElement> elms;
        for (auto entity : _cam->getEntities<PointLight>())
        {
            auto& elm = elms.emplace_back();
            auto& light = _scene->getComponent<const PointLight>(entity).value();
            auto trans = _scene->getComponent<const Transform>(entity);
            float scale = 1.f;
            if (trans)
            {
                elm.pos = glm::vec4{ trans->getWorldPosition(), 0.f };
                scale = glm::compMax(trans->getWorldScale());
            }
            elm.pos.w = float(entity);
            auto range = light.getRange() * scale;
            auto intensity = light.getIntensity();
            elm.color = glm::vec4{ Colors::normalize(light.getColor()) * intensity, range };
        }

        VertexDataWriter writer{ _pointLightsLayout, static_cast<uint32_t>(elms.size()) };
        uint32_t index = 0;
        for (auto& elm : elms)
        {
            writer.write(bgfx::Attrib::Position, index, elm.pos);
            writer.write(bgfx::Attrib::Color0, index, elm.color);
            ++index;
        }
        auto data = writer.finish();
        if (!data.empty())
        {
            bgfx::update(_pointLightBuffer, 0, data.copyMem());
        }

        return index;
    }

    struct SpotLightBufferElement final
    {
        glm::vec4 pos{ 0.f };
        glm::vec3 direction{ 0.f, 0.f, 1.f };
        glm::vec4 color{ 1.f };
        glm::vec4 data{ 0.f };
    };

    size_t LightingRenderComponent::updateSpotLights() noexcept
    {
        std::vector<SpotLightBufferElement> elms;
        for (auto entity : _cam->getEntities<SpotLight>())
        {
            auto& elm = elms.emplace_back();
            auto& light = _scene->getComponent<const SpotLight>(entity).value();
            auto trans = _scene->getComponent<const Transform>(entity);
            float scale = 1.f;
            if (trans)
            {
                elm.pos = glm::vec4(trans->getWorldPosition(), 0.f);
                scale = glm::compMax(trans->getWorldScale());
                elm.direction = trans->getWorldDirection();
            }
            elm.pos.w = static_cast<float>(entity);
            auto range = light.getRange() * scale;
            auto intensity = light.getIntensity();
            elm.color = glm::vec4{ Colors::normalize(light.getColor()) * intensity, range };
            elm.data = glm::vec4{ light.getConeAngle(), light.getInnerConeAngle(), 0.f, 0.f };
        }

        VertexDataWriter writer{ _spotLightsLayout, static_cast<uint32_t>(elms.size()) };
        uint32_t index = 0;
        for (auto& elm : elms)
        {
            writer.write(bgfx::Attrib::Position, index, elm.pos);
            writer.write(bgfx::Attrib::Normal, index, elm.direction);
            writer.write(bgfx::Attrib::Color0, index, elm.color);
            writer.write(bgfx::Attrib::Weight, index, elm.data);
            ++index;
        }
        auto data = writer.finish();
        if (!data.empty())
        {
            bgfx::update(_spotLightBuffer, 0, data.copyMem());
        }

        return index;
    }

    void LightingRenderComponent::updateAmbientLights() noexcept
    {
        auto entities = _cam->getEntities<AmbientLight>();
        _lightData = glm::vec4{ 0.f };
        for (auto entity : entities)
        {
            const auto& ambientLight = _scene->getComponent<const AmbientLight>(entity).value();
            auto c = Colors::normalize(ambientLight.getColor()) * ambientLight.getIntensity();
            _lightData += glm::vec4{ c, 0.f };
        }
    }

    void LightingRenderComponent::updateCamera() noexcept
    {
        _camPos = glm::vec4{ 0.f };
        if (!_cam || !_scene)
        {
            return;
        }
        auto camEntity = _scene->getEntity(_cam.value());
        if (camEntity != entt::null)
        {
            if (auto camTrans = _scene->getComponent<const Transform>(camEntity))
            {
                _camPos = glm::vec4{ camTrans->getWorldPosition(), 0.f };
            }
        }
    }

    expected<void, std::string> LightingRenderComponent::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return unexpected<std::string>{ "scene not loaded" };
        }
        _lightCount.x = static_cast<float>(updatePointLights());
        _lightCount.y = static_cast<float>(updateDirLights());
        _lightCount.z = static_cast<float>(updateSpotLights());
        updateAmbientLights();
        updateCamera();
        return {};
    }

    expected<void, std::string> LightingRenderComponent::beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        encoder.setUniform(_lightCountUniform, glm::value_ptr(_lightCount));
        encoder.setUniform(_lightDataUniform, glm::value_ptr(_lightData));
        encoder.setBuffer(RenderSamplers::LIGHTS_POINT, _pointLightBuffer, bgfx::Access::Read);
        encoder.setBuffer(RenderSamplers::LIGHTS_DIR, _dirLightBuffer, bgfx::Access::Read);
        encoder.setBuffer(RenderSamplers::LIGHTS_SPOT, _spotLightBuffer, bgfx::Access::Read);
        encoder.setUniform(_camPosUniform, glm::value_ptr(_camPos));

        glm::mat3 normalMatrix{ 1.f };
        if (auto trans = _scene->getComponent<Transform>(entity))
        {
            normalMatrix = glm::transpose(glm::adjugate(glm::mat3(trans->getWorldMatrix())));
        }
        encoder.setUniform(_normalMatrixUniform, glm::value_ptr(normalMatrix));

        return {};
    }
}