#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <unordered_map>
#include <darmok/glm.hpp>

namespace darmok
{
    class Material;
    struct ProgramDefinition;

    class DARMOK_EXPORT PointLight final
    {
    public:
        PointLight(float intensity = 1.F, const Color3& color = Colors::white3(), float radius = 1.F) noexcept;

        PointLight& setIntensity(float intensity) noexcept;
        PointLight& setRadius(float radius) noexcept;

        PointLight& setColor(const Color3& color) noexcept;

        [[nodiscard]] float getRadius() const noexcept;
        [[nodiscard]] float getIntensity() const noexcept;
        [[nodiscard]] const Color3& getColor() const noexcept;
    private:
        float _intensity;
        float _radius;
        Color3 _color;
    };

    class DARMOK_EXPORT DirectionalLight final
    {
    public:
        DirectionalLight(float intensity = 1.F) noexcept;

        DirectionalLight& setIntensity(float intensity) noexcept;
        DirectionalLight& setColor(const Color3& color) noexcept;

        [[nodiscard]] const Color3& getColor() const noexcept;
        [[nodiscard]] float getIntensity() const noexcept;
    private:
        float _intensity;
        Color3 _color;
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

    class DARMOK_EXPORT SpotLight final
    {
    public:
        SpotLight(float intensity = 1.F) noexcept;

        SpotLight& setIntensity(float intensity) noexcept;
        SpotLight& setColor(const Color3& color) noexcept;

        [[nodiscard]] const Color3& getColor() const noexcept;
        [[nodiscard]] float getIntensity() const noexcept;
    private:
        float _intensity;
        Color3 _color;
    };

    class DARMOK_EXPORT LightingCameraComponent final : public ICameraComponent
    {
    public:
        LightingCameraComponent() noexcept;
        ~LightingCameraComponent() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime)  noexcept override;
        void beforeRenderEntity(Entity entity, IRenderGraphContext& context) noexcept override;

    private:
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;

        bgfx::UniformHandle _lightCountUniform;
        bgfx::UniformHandle _lightDataUniform;
        bgfx::UniformHandle _camPosUniform;
        bgfx::UniformHandle _normalMatrixUniform;
        bgfx::DynamicVertexBufferHandle _pointLightBuffer;
        bgfx::DynamicVertexBufferHandle _dirLightBuffer;

        bgfx::VertexLayout _pointLightsLayout;
        bgfx::VertexLayout _dirLightsLayout;

        glm::vec4 _lightCount;
        glm::vec4 _lightData;
        glm::vec4 _camPos;

        size_t updatePointLights() noexcept;
        size_t updateDirLights() noexcept;
        void updateAmbientLights() noexcept;
        void updateCamera() noexcept;

        void createHandles() noexcept;
        void destroyHandles() noexcept;
    };
}