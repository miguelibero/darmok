#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/shape.hpp>

namespace darmok::editor
{
    class CubeInspectorEditor final : public ITypeObjectEditor<Cube>
    {
    public:
        bool renderType(Cube& cube) noexcept override;
    };

    class SphereInspectorEditor final : public ITypeObjectEditor<Sphere>
    {
    public:
        bool renderType(Sphere& sphere) noexcept override;
    };
}
