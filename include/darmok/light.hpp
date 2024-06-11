#pragma once

#include <darmok/export.h>
#include <glm/glm.hpp>
#include <darmok/camera.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <unordered_map>

namespace darmok
{
    class Material;
    struct ProgramDefinition;

    class DARMOK_EXPORT PointLight final
    {
    public:
        PointLight(float intensity = 1.F) noexcept;

        PointLight& setIntensity(float intensity) noexcept;
        PointLight& setRadius(float radius) noexcept;
        PointLight& setAttenuation(const glm::vec3& attn) noexcept;
        PointLight& setColor(const Color3& color) noexcept;
        PointLight& setDiffuseColor(const Color3& color) noexcept;
        PointLight& setSpecularColor(const Color3& color) noexcept;

        [[nodiscard]] float getRadius() const noexcept;
        [[nodiscard]] float getIntensity() const noexcept;
        [[nodiscard]] const glm::vec3& getAttenuation() const noexcept;
        [[nodiscard]] const Color3& getDiffuseColor() const noexcept;
        [[nodiscard]] const Color3& getSpecularColor() const noexcept;
    private:
        float _intensity;
        glm::vec3 _attenuation;
        float _radius;
        Color3 _diffuseColor;
        Color3 _specularColor;
    };

    class DARMOK_EXPORT AmbientLight final
    {
    public:
        AmbientLight(float intensity = 1.F) noexcept;

        AmbientLight& setIntensity(float intensity) noexcept;
        AmbientLight& setColor(const Color3& color) noexcept;

        [[nodiscard]] const Color3& getColor() const noexcept;
        [[nodiscard]] float getIntensity() const noexcept;
    private:
        float _intensity;
        Color3 _color;
    };


    class DARMOK_EXPORT PhongLightingComponent final : public ICameraComponent
    {
    public:
        PhongLightingComponent() noexcept;
        ~PhongLightingComponent() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown()  noexcept override;
        void update(float deltaTime)  noexcept override;
        void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) noexcept override;

    private:
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;

        bgfx::UniformHandle _lightCountUniform;
        bgfx::UniformHandle _lightDataUniform;
        bgfx::UniformHandle _camPosUniform;
        bgfx::DynamicVertexBufferHandle _pointLightBuffer;

        bgfx::VertexLayout _pointLightsLayout;

        glm::vec4 _lightCount;
        glm::vec4 _lightData;
        glm::vec4 _camPos;
        Data _pointLights;

        size_t updatePointLights() noexcept;
        void updateAmbientLights() noexcept;
        void updateCamera() noexcept;

        void createHandles() noexcept;
        void destroyHandles() noexcept;
    };
}