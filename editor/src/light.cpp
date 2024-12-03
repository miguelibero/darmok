#include <darmok-editor/light.hpp>
#include <darmok/light.hpp>
#include <imgui.h>

namespace darmok::editor
{
    bool PointLightInspectorEditor::render(PointLight& light) noexcept
    {
        if (ImGui::CollapsingHeader("Point Light"))
        {
        }
        return true;
    }

    bool DirectionalLightInspectorEditor::render(DirectionalLight& light) noexcept
    {
        if (ImGui::CollapsingHeader("Directional Light"))
        {
        }
        return true;
    }

    bool SpotLightInspectorEditor::render(SpotLight& light) noexcept
    {
        if (ImGui::CollapsingHeader("Spot Light"))
        {
        }
        return true;
    }

    bool AmbientLightInspectorEditor::render(AmbientLight& light) noexcept
    {
        if (ImGui::CollapsingHeader("Ambient Light"))
        {
        }
        return true;
    }
}