#pragma once

#include <glm/glm.hpp>
#include <darmok/scene.hpp>
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

    class LightRenderUpdater final : public ISceneLogicUpdater
    {
    public:
        LightRenderUpdater() noexcept;
        void init(Scene& scene, App& app) noexcept override;
        void shutdown()  noexcept override;
        void update(float deltaTime)  noexcept override;
        bool bgfxConfig(const Camera& cam, const ProgramDefinition& progDef, bgfx::Encoder& encoder) const noexcept;

        static const ProgramDefinition& getPhongProgramDefinition() noexcept;

    private:
        OptionalRef<Scene> _scene;
        bgfx::UniformHandle _countUniform;
        bgfx::UniformHandle _ambientIntensityUniform;
        bgfx::DynamicVertexBufferHandle _pointLightsBuffer;
        std::unordered_map<Entity, Data> _pointLights;
        glm::vec4 _lightCount;
        glm::vec4 _ambientColor;
    };
}