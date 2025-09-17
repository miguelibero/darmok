#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/light.hpp>

namespace darmok::editor
{
    class PointLightInspectorEditor final : public ComponentObjectEditor<PointLight>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(PointLight::Definition& light) noexcept override;
    };

    class DirectionalLightInspectorEditor final : public ComponentObjectEditor<DirectionalLight>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(DirectionalLight::Definition& light) noexcept override;
    };

    class SpotLightInspectorEditor final : public ComponentObjectEditor<SpotLight>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(SpotLight::Definition& light) noexcept override;
    };

    class AmbientLightInspectorEditor final : public ComponentObjectEditor<AmbientLight>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(AmbientLight::Definition& light) noexcept override;
    };
}