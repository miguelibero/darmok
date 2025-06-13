#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/light.hpp>

namespace darmok::editor
{
    class PointLightInspectorEditor final : public ITypeObjectEditor<PointLight::Definition>
    {
    public:
        bool renderType(PointLight::Definition& light) noexcept override;
    };

    class DirectionalLightInspectorEditor final : public ITypeObjectEditor<DirectionalLight::Definition>
    {
    public:
        bool renderType(DirectionalLight::Definition& light) noexcept override;
    };

    class SpotLightInspectorEditor final : public ITypeObjectEditor<SpotLight::Definition>
    {
    public:
        bool renderType(SpotLight::Definition& light) noexcept override;
    };

    class AmbientLightInspectorEditor final : public ITypeObjectEditor<AmbientLight::Definition>
    {
    public:
        bool renderType(AmbientLight::Definition& light) noexcept override;
    };
}