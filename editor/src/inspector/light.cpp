#include <darmok-editor/inspector/light.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/light.hpp>
#include <darmok/glm_serialize.hpp>
#include <imgui.h>

namespace darmok::editor
{
    namespace LightEditorUtils
    {
        static const std::vector<std::string> shadowTypeLabels =
        {
            "No Shadows",
            "Hard Shadows",
            "Soft Shadows"
        };;
    };

    bool PointLightInspectorEditor::renderType(PointLight::Definition& light) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Point Light"))
        {
            if (ImguiUtils::drawProtobufInput("Intensity", "intensity", light))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Range", "range", light))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Color", "color", light))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufEnumInput("Shadow", "shadow_type", light, LightEditorUtils::shadowTypeLabels))
            {
                changed = true;
            }
        }
        return changed;
    }

    bool DirectionalLightInspectorEditor::renderType(DirectionalLight::Definition& light) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Directional Light"))
        {
            if (ImguiUtils::drawProtobufInput("Intensity", "intensity", light))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Color", "color", light))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufEnumInput("Shadow", "shadow_type", light, LightEditorUtils::shadowTypeLabels))
            {
                changed = true;
            }
        }
        return changed;
    }

    bool SpotLightInspectorEditor::renderType(SpotLight::Definition& light) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Spot Light"))
        {
            if (ImguiUtils::drawProtobufInput("Intensity", "intensity", light))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Range", "range", light))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Color", "color", light))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufEnumInput("Shadow", "shadow_type", light, LightEditorUtils::shadowTypeLabels))
            {
                changed = true;
            }
        }
        return changed;
    }

    bool AmbientLightInspectorEditor::renderType(AmbientLight::Definition& light) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Ambient Light"))
        {
            if (ImguiUtils::drawProtobufInput("Intensity", "intensity", light))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Color", "color", light))
            {
                changed = true;
            }
        }
        return changed;
    }
}