#include <darmok-editor/inspector/shape.hpp>
#include <darmok-editor/utils.hpp>

#include <imgui.h>

namespace darmok::editor
{

    std::string CubeInspectorEditor::getTitle() const noexcept
    {
        return "Cube";
    }

    bool CubeInspectorEditor::renderType(Cube::Definition& cube) noexcept
    {
        auto changed = false;

        if (ImguiUtils::drawProtobufInput("Size", "size", cube))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Origin", "origin", cube))
        {
            changed = true;
        }

        return changed;
    }

    std::string SphereInspectorEditor::getTitle() const noexcept
    {
        return "Sphere";
    }

    bool SphereInspectorEditor::renderType(Sphere::Definition& sphere) noexcept
    {
        auto changed = false;
 
        if (ImguiUtils::drawProtobufInput("Radius", "radius", sphere))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Origin", "origin", sphere))
        {
            changed = true;
        }

        return changed;
    }

    std::string CapsuleInspectorEditor::getTitle() const noexcept
    {
        return "Capsule";
    }

    bool CapsuleInspectorEditor::renderType(Capsule::Definition& capsule) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Cylinder height", "cylinder_height", capsule))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Radius", "radius", capsule))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Origin", "origin", capsule))
        {
            changed = true;
        }

        return changed;
    }

    std::string RectangleInspectorEditor::getTitle() const noexcept
    {
        return "Rectangle";
    }

    bool RectangleInspectorEditor::renderType(Rectangle::Definition& rect) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Size", "size", rect))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Origin", "origin", rect))
        {
            changed = true;
        }
        return changed;
    }
}
