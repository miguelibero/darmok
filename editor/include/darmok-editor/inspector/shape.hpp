#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/shape.hpp>

namespace darmok::editor
{
    class CubeInspectorEditor final : public ITypeObjectEditor<Cube::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        bool renderType(Cube::Definition& cube) noexcept override;
    };

    class SphereInspectorEditor final : public ITypeObjectEditor<Sphere::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        bool renderType(Sphere::Definition& sphere) noexcept override;
    };

    class CapsuleInspectorEditor final : public ITypeObjectEditor<Capsule::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        bool renderType(Capsule::Definition& capsule) noexcept override;
    };

    class RectangleInspectorEditor final : public ITypeObjectEditor<Rectangle::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        bool renderType(Rectangle::Definition& rect) noexcept override;
    };
}
