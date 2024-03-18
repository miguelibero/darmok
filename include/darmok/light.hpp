#pragma once

#include <glm/glm.hpp>
#include <darmok/scene.hpp>
#include <darmok/data.hpp>
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

        [[nodiscard]] float getRadius() const noexcept;
        [[nodiscard]] const glm::vec3& getIntensity() const noexcept;
    private:
        glm::vec3 _intensity;
        float _radius;
    };

    class AmbientLight final
    {
    public:
        AmbientLight(const glm::vec3& intensity = glm::vec3(1)) noexcept;

        AmbientLight& setIntensity(const glm::vec3& intensity) noexcept;

        [[nodiscard]] const glm::vec3& getIntensity() const noexcept;
    private:
        glm::vec3 _intensity;
    };

    class LightRenderUpdater final : public ISceneLogicUpdater
    {
    public:
        LightRenderUpdater() noexcept;
        void init(Scene& scene, App& app) noexcept override;
        void shutdown()  noexcept override;
        void update(float deltaTime)  noexcept override;
        bool bgfxConfig(const Camera& cam, const Material& mat, bgfx::Encoder& encoder) noexcept;

        static const ProgramDefinition& getProgramDefinition() noexcept;

    private:
        OptionalRef<Scene> _scene;
        bgfx::UniformHandle _countUniform;
        bgfx::UniformHandle _ambientIntensityUniform;
        bgfx::DynamicVertexBufferHandle _pointLightsBuffer;
        Entity _pointLightsBufferCamera;
        std::unordered_map<Entity, Data> _pointLights;
        glm::vec4 _lightCount;
        glm::vec4 _ambientIntensity;
    };
}