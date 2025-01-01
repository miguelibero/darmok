#include <darmok-editor/inspector/shape.hpp>
#include <darmok-editor/utils.hpp>

#include <imgui.h>

namespace darmok::editor
{
    bool CubeInspectorEditor::renderType(Cube& cube) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Cube"))
        {
            if (ImguiUtils::drawSizeInput("Size", cube.size))
            {
                changed = true;
            }
            if (ImguiUtils::drawPositionInput("Origin", cube.origin))
            {
                changed = true;
            }
        }
        return changed;
    }

    bool SphereInspectorEditor::renderType(Sphere& sphere) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Sphere"))
        {
            if (ImGui::InputFloat("Radius", &sphere.radius))
            {
                changed = true;
            }
            if (ImguiUtils::drawPositionInput("Origin", sphere.origin))
            {
                changed = true;
            }
        }
        return changed;
    }
}
