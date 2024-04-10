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
        PointLight(const glm::vec3& intensity = glm::vec3(0, 0, 1), float radius = 1.F) noexcept;

        PointLight& setRadius(float radius) noexcept;
        PointLight& setIntensity(const glm::vec3& intensity) noexcept;
        PointLight& setColor(const Color3& color) noexcept;
        PointLight& setDiffuseColor(const Color3& color) noexcept;
        PointLight& setSpecularColor(const Color3& color) noexcept;

        [[nodiscard]] float getRadius() const noexcept;
        [[nodiscard]] const glm::vec3& getIntensity() const noexcept;
        [[nodiscard]] const Color3& getDiffuseColor() const noexcept;
        [[nodiscard]] const Color3& getSpecularColor() const noexcept;
    private:
        glm::vec3 _intensity;
        float _radius;
        Color3 _diffuseColor;
        Color3 _specularColor;
    };

    class AmbientLight final
    {
    public:
        AmbientLight(const glm::vec3& intensity = glm::vec3(1)) noexcept;

        AmbientLight& setColor(const Color3& color) noexcept;
        AmbientLight& setIntensity(const glm::vec3& intensity) noexcept;

        [[nodiscard]] const glm::vec3& getIntensity() const noexcept;
        [[nodiscard]] const Color3& getColor() const noexcept;
    private:
        glm::vec3 _intensity;
        Color3 _color;
    };


    class BX_NO_VTABLE ILightingComponent : public ICameraComponent
    {
    public:
        virtual void bgfxConfig(bgfx::Encoder& encoder, bgfx::ViewId viewId) const = 0;
    };

    class PhongLightingComponent final : public ILightingComponent
    {
    public:
        PhongLightingComponent() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown()  noexcept override;
        void update(float deltaTime)  noexcept override;
        void bgfxConfig(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept override;

    private:
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;

        bgfx::UniformHandle _lightCountUniform;
        bgfx::UniformHandle _lightDataUniform;
        bgfx::DynamicVertexBufferHandle _pointLightBuffer;

        bgfx::VertexLayout _pointLightsLayout;

        glm::vec4 _lightCount;
        glm::vec4 _lightData;
        Data _pointLights;

        size_t updatePointLights() noexcept;
    };
}