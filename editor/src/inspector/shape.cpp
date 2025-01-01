#include <darmok-editor/inspector/shape.hpp>
#include <darmok-editor/utils.hpp>

#include <imgui.h>

namespace darmok::editor
{
    bool CubeInspectorEditor::render(Cube& cube) noexcept
    {
        if (ImGui::CollapsingHeader("Cube"))
        {
            ImguiUtils::drawSizeInput("Size", cube.size);
            ImguiUtils::drawPositionInput("Origin", cube.origin);
        }
        return true;
    }

    bool SphereInspectorEditor::render(Sphere& sphere) noexcept
    {
        if (ImGui::CollapsingHeader("Sphere"))
        {
        }
        return true;
    }
}
