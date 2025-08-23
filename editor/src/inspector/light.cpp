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

    std::string PointLightInspectorEditor::getTitle() const noexcept
    {
		return "Point Light";
    }

    PointLightInspectorEditor::RenderResult PointLightInspectorEditor::renderType(PointLight::Definition& light) noexcept
    {
        auto changed = false;
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
        return changed;
    }

    std::string DirectionalLightInspectorEditor::getTitle() const noexcept
    {
        return "Directional Light";
    }

    DirectionalLightInspectorEditor::RenderResult DirectionalLightInspectorEditor::renderType(DirectionalLight::Definition& light) noexcept
    {
        auto changed = false;
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
        return changed;
    }

    std::string SpotLightInspectorEditor::getTitle() const noexcept
    {
        return "Spot Light";
    }

    SpotLightInspectorEditor::RenderResult SpotLightInspectorEditor::renderType(SpotLight::Definition& light) noexcept
    {
        auto changed = false;
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
        return changed;
    }

    std::string AmbientLightInspectorEditor::getTitle() const noexcept
    {
        return "Ambient Light";
    }

    AmbientLightInspectorEditor::RenderResult AmbientLightInspectorEditor::renderType(AmbientLight::Definition& light) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Intensity", "intensity", light))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Color", "color", light))
        {
            changed = true;
        }
        return changed;
    }
}