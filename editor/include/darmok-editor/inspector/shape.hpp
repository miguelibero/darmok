#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/shape.hpp>

namespace darmok::editor
{
    class CubeInspectorEditor final : public ITypeObjectEditor<Cube>
    {
    public:
        bool render(Cube& cube) noexcept override;
    };

    class SphereInspectorEditor final : public ITypeObjectEditor<Sphere>
    {
    public:
        bool render(Sphere& sphere) noexcept override;
    };
}
