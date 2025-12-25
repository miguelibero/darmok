#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/light.hpp>

namespace darmok::editor
{
    class PointLightInspectorEditor final : public EntityComponentObjectEditor<PointLight>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(PointLight::Definition& light) noexcept override;
    };

    class DirectionalLightInspectorEditor final : public EntityComponentObjectEditor<DirectionalLight>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(DirectionalLight::Definition& light) noexcept override;
    };

    class SpotLightInspectorEditor final : public EntityComponentObjectEditor<SpotLight>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(SpotLight::Definition& light) noexcept override;
    };

    class AmbientLightInspectorEditor final : public EntityComponentObjectEditor<AmbientLight>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(AmbientLight::Definition& light) noexcept override;
    };
}