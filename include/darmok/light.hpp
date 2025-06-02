#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <darmok/shadow_fwd.hpp>
#include <darmok/protobuf/light.pb.h>

#include <unordered_map>

namespace darmok
{
    class Material;

    using LightDefinition = protobuf::Light;
	using ShadowType = protobuf::Light::ShadowType;

    class DARMOK_EXPORT PointLight final
    {
    public:
        PointLight(float intensity = 1.F, const Color3& color = Colors::white3(), float range = 10.F) noexcept;

        PointLight& setIntensity(float intensity) noexcept;
        PointLight& setRange(float range) noexcept;
        PointLight& setColor(const Color3& color) noexcept;
        PointLight& setShadowType(ShadowType type) noexcept;

        [[nodiscard]] float getRange() const noexcept;
        [[nodiscard]] float getIntensity() const noexcept;
        [[nodiscard]] const Color3& getColor() const noexcept;
        [[nodiscard]] ShadowType getShadowType() const noexcept;

        // serialization
        using Definition = protobuf::PointLight;
        expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt);

    private:
        float _intensity;
        float _range;
        Color3 _color;
        ShadowType _shadow;
    };

    class DARMOK_EXPORT DirectionalLight final
    {
    public:
        DirectionalLight(float intensity = 1.F) noexcept;

        DirectionalLight& setIntensity(float intensity) noexcept;
        DirectionalLight& setColor(const Color3& color) noexcept;
        DirectionalLight& setShadowType(ShadowType type) noexcept;

        [[nodiscard]] const Color3& getColor() const noexcept;
        [[nodiscard]] float getIntensity() const noexcept;
        [[nodiscard]] ShadowType getShadowType() const noexcept;

        // serialization
        using Definition = protobuf::DirectionalLight;
        expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt);

    private:
        float _intensity;
        Color3 _color;
        ShadowType _shadow;
    };

    class DARMOK_EXPORT SpotLight final
    {
    public:
        SpotLight(float intensity = 1.F, const Color3& color = Colors::white3(), float range = 10.F) noexcept;

        SpotLight& setIntensity(float intensity) noexcept;
        SpotLight& setRange(float range) noexcept;
        SpotLight& setColor(const Color3& color) noexcept;
        SpotLight& setConeAngle(float angle) noexcept;
        SpotLight& setInnerConeAngle(float angle) noexcept;
        SpotLight& setShadowType(ShadowType type) noexcept;

        [[nodiscard]] const Color3& getColor() const noexcept;
        [[nodiscard]] float getIntensity() const noexcept;
        [[nodiscard]] float getRange() const noexcept;
        [[nodiscard]] float getConeAngle() const noexcept;
        [[nodiscard]] float getInnerConeAngle() const noexcept;
        [[nodiscard]] ShadowType getShadowType() const noexcept;

        // serialization
        using Definition = protobuf::SpotLight;
        expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt);

    private:
        float _intensity;
        float _range;
        Color3 _color;
        float _coneAngle;
        float _innerConeAngle;
        ShadowType _shadow;
    };

    class DARMOK_EXPORT AmbientLight final
    {
    public:
        AmbientLight(float intensity = 1.F) noexcept;

        AmbientLight& setIntensity(float intensity) noexcept;
        AmbientLight& setColor(const Color3& color) noexcept;

        [[nodiscard]] const Color3& getColor() const noexcept;
        [[nodiscard]] float getIntensity() const noexcept;

        // serialization
        using Definition = protobuf::AmbientLight;
        expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt);

    private:
        float _intensity;
        Color3 _color;
    };

    class DARMOK_EXPORT LightingRenderComponent final : public ITypeCameraComponent<LightingRenderComponent>
    {
    public:
        LightingRenderComponent() noexcept;
        ~LightingRenderComponent() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime)  noexcept override;
        void beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;

    private:
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;

        bgfx::UniformHandle _lightCountUniform;
        bgfx::UniformHandle _lightDataUniform;
        bgfx::UniformHandle _camPosUniform;
        bgfx::UniformHandle _normalMatrixUniform;
        bgfx::DynamicVertexBufferHandle _pointLightBuffer;
        bgfx::DynamicVertexBufferHandle _dirLightBuffer;
        bgfx::DynamicVertexBufferHandle _spotLightBuffer;

        bgfx::VertexLayout _pointLightsLayout;
        bgfx::VertexLayout _dirLightsLayout;
        bgfx::VertexLayout _spotLightsLayout;

        glm::vec4 _lightCount;
        glm::vec4 _lightData;
        glm::vec4 _camPos;

        size_t updatePointLights() noexcept;
        size_t updateSpotLights() noexcept;
        size_t updateDirLights() noexcept;
        void updateAmbientLights() noexcept;
        void updateCamera() noexcept;

        void createHandles() noexcept;
        void destroyHandles() noexcept;
    };
}