#include <darmok-editor/inspector/shape.hpp>
#include <imgui.h>

namespace darmok::editor
{
    bool CubeInspectorEditor::render(Cube& cube) noexcept
    {
        if (ImGui::CollapsingHeader("Cube"))
        {
        }
        return true;
    }

    bool SphereInspectorEditor::render(Sphere& sphere) noexcept
    {
        if (ImGui::CollapsingHeader("Cube"))
        {
        }
        return true;
    }
}
