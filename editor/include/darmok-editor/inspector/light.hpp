#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/light.hpp>

namespace darmok::editor
{
    class PointLightInspectorEditor final : public ComponentObjectEditor<PointLight::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(PointLight::Definition& light) noexcept override;
    };

    class DirectionalLightInspectorEditor final : public ComponentObjectEditor<DirectionalLight::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(DirectionalLight::Definition& light) noexcept override;
    };

    class SpotLightInspectorEditor final : public ComponentObjectEditor<SpotLight::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(SpotLight::Definition& light) noexcept override;
    };

    class AmbientLightInspectorEditor final : public ComponentObjectEditor<AmbientLight::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(AmbientLight::Definition& light) noexcept override;
    };
}