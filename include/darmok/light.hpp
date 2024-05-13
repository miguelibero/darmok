#pragma once

#include <glm/glm.hpp>
#include <darmok/camera.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <unordered_map>

namespace darmok
{
    class Material;
    struct ProgramDefinition;

    class PointLight final
    {
    public:
        DLLEXPORT PointLight(float intensity = 1.F) noexcept;

        DLLEXPORT PointLight& setIntensity(float intensity) noexcept;
        DLLEXPORT PointLight& setRadius(float radius) noexcept;
        DLLEXPORT PointLight& setAttenuation(const glm::vec3& attn) noexcept;
        DLLEXPORT PointLight& setColor(const Color3& color) noexcept;
        DLLEXPORT PointLight& setDiffuseColor(const Color3& color) noexcept;
        DLLEXPORT PointLight& setSpecularColor(const Color3& color) noexcept;

        [[nodiscard]] DLLEXPORT float getRadius() const noexcept;
        [[nodiscard]] DLLEXPORT float getIntensity() const noexcept;
        [[nodiscard]] DLLEXPORT const glm::vec3& getAttenuation() const noexcept;
        [[nodiscard]] DLLEXPORT const Color3& getDiffuseColor() const noexcept;
        [[nodiscard]] DLLEXPORT const Color3& getSpecularColor() const noexcept;
    private:
        float _intensity;
        glm::vec3 _attenuation;
        float _radius;
        Color3 _diffuseColor;
        Color3 _specularColor;
    };

    class AmbientLight final
    {
    public:
        DLLEXPORT AmbientLight(float intensity = 1.F) noexcept;

        DLLEXPORT AmbientLight& setIntensity(float intensity) noexcept;
        DLLEXPORT AmbientLight& setColor(const Color3& color) noexcept;

        [[nodiscard]] DLLEXPORT const Color3& getColor() const noexcept;
        [[nodiscard]] DLLEXPORT float getIntensity() const noexcept;
    private:
        float _intensity;
        Color3 _color;
    };


    class PhongLightingComponent final : public ICameraComponent
    {
    public:
        DLLEXPORT PhongLightingComponent() noexcept;
        DLLEXPORT ~PhongLightingComponent() noexcept;
        DLLEXPORT void init(Camera& cam, Scene& scene, App& app) noexcept override;
        DLLEXPORT void shutdown()  noexcept override;
        DLLEXPORT void update(float deltaTime)  noexcept override;
        DLLEXPORT void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept override;

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