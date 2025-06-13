#include <darmok-editor/inspector/shape.hpp>
#include <darmok-editor/utils.hpp>

#include <imgui.h>

namespace darmok::editor
{
    bool CubeInspectorEditor::renderType(Cube::Definition& cube) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Cube"))
        {
            if (ImguiUtils::drawProtobufInput("Size", "size", cube))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Origin", "origin", cube))
            {
                changed = true;
            }
        }
        return changed;
    }

    bool SphereInspectorEditor::renderType(Sphere::Definition& sphere) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Sphere"))
        {
            if (ImguiUtils::drawProtobufInput("Radius", "radius", sphere))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Origin", "origin", sphere))
            {
                changed = true;
            }
        }
        return changed;
    }
}
