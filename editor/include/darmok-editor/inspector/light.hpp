#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/light.hpp>

namespace darmok::editor
{
    class PointLightInspectorEditor final : public ITypeObjectEditor<PointLight>
    {
    public:
        bool renderType(PointLight& light) noexcept override;
    };

    class DirectionalLightInspectorEditor final : public ITypeObjectEditor<DirectionalLight>
    {
    public:
        bool renderType(DirectionalLight& light) noexcept override;
    };

    class SpotLightInspectorEditor final : public ITypeObjectEditor<SpotLight>
    {
    public:
        bool renderType(SpotLight& light) noexcept override;
    };

    class AmbientLightInspectorEditor final : public ITypeObjectEditor<AmbientLight>
    {
    public:
        bool renderType(AmbientLight& light) noexcept override;
    };
}