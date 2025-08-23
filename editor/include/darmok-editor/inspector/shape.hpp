#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/shape.hpp>

namespace darmok::editor
{
    class CubeInspectorEditor final : public ObjectEditor<Cube::Definition>
    {
    public:
		std::string getTitle() const noexcept override;
        RenderResult renderType(Cube::Definition& cube) noexcept override;
    };

    class SphereInspectorEditor final : public ObjectEditor<Sphere::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Sphere::Definition& sphere) noexcept override;
    };

    class CapsuleInspectorEditor final : public ObjectEditor<Capsule::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Capsule::Definition& capsule) noexcept override;
    };

    class RectangleInspectorEditor final : public ObjectEditor<Rectangle::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Rectangle::Definition& rect) noexcept override;
    };
}
