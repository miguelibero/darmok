#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/shape.hpp>

namespace darmok::editor
{
    class CubeInspectorEditor final : public ITypeObjectEditor<Cube::Definition>
    {
    public:
        bool renderType(Cube::Definition& cube) noexcept override;
    };

    class SphereInspectorEditor final : public ITypeObjectEditor<Sphere::Definition>
    {
    public:
        bool renderType(Sphere::Definition& sphere) noexcept override;
    };
}
