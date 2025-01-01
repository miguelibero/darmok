#include <darmok-editor/inspector/light.hpp>
#include <darmok/light.hpp>
#include <darmok/utils.hpp>
#include <imgui.h>
#include <array>

namespace darmok::editor
{
    struct LightEditorUtils final
    {
        static const std::array<std::string, toUnderlying(ShadowType::Count)> _shadowTypeLabels;

        static bool renderShadowTypeInput(ShadowType& current) noexcept
        {
            auto changed = false;
            if (ImGui::BeginCombo("Shadow Type", _shadowTypeLabels[toUnderlying(current)].c_str()))
            {
                for (size_t i = 0; i < _shadowTypeLabels.size(); ++i)
                {
                    auto shadowType = static_cast<ShadowType>(i);;
                    auto selected = current == shadowType;
                    if (ImGui::Selectable(_shadowTypeLabels[i].c_str(), selected))
                    {
                        current = shadowType;
                        changed = true;
                    }
                    if (selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            return changed;
        }
    };

    const std::array<std::string, toUnderlying(ShadowType::Count)> LightEditorUtils::_shadowTypeLabels =
    {
        "No Shadows",
        "Hard Shadows",
        "Soft Shadows"
    };

    bool PointLightInspectorEditor::renderType(PointLight& light) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Point Light"))
        {
            float intensity = light.getIntensity();
            if (ImGui::InputFloat("Intensity", &intensity))
            {
                light.setIntensity(intensity);
                changed = true;
            }
            float range = light.getRange();
            if (ImGui::InputFloat("Range", &range))
            {
                light.setRange(range);
                changed = true;
            }
            auto color = Colors::normalize(light.getColor());
            if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
            {
                light.setColor(Colors::denormalize(color));
                changed = true;
            }
            auto shadowType = light.getShadowType();
            if (LightEditorUtils::renderShadowTypeInput(shadowType))
            {
                light.setShadowType(shadowType);
                changed = true;
            }
        }
        return changed;
    }

    bool DirectionalLightInspectorEditor::renderType(DirectionalLight& light) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Directional Light"))
        {
            float intensity = light.getIntensity();
            if (ImGui::InputFloat("Intensity", &intensity))
            {
                light.setIntensity(intensity);
                changed = true;
            }
            auto color = Colors::normalize(light.getColor());
            if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
            {
                light.setColor(Colors::denormalize(color));
                changed = true;
            }
            auto shadowType = light.getShadowType();
            if (LightEditorUtils::renderShadowTypeInput(shadowType))
            {
                light.setShadowType(shadowType);
                changed = true;
            }
        }
        return changed;
    }

    bool SpotLightInspectorEditor::renderType(SpotLight& light) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Spot Light"))
        {
            float intensity = light.getIntensity();
            if (ImGui::InputFloat("Intensity", &intensity))
            {
                light.setIntensity(intensity);
                changed = true;
            }
            float range = light.getRange();
            if (ImGui::InputFloat("Range", &range))
            {
                light.setRange(range);
                changed = true;
            }
            auto color = Colors::normalize(light.getColor());
            if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
            {
                light.setColor(Colors::denormalize(color));
                changed = true;
            }
            auto shadowType = light.getShadowType();
            if (LightEditorUtils::renderShadowTypeInput(shadowType))
            {
                light.setShadowType(shadowType);
                changed = true;
            }
        }
        return changed;
    }

    bool AmbientLightInspectorEditor::renderType(AmbientLight& light) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Ambient Light"))
        {
            float intensity = light.getIntensity();
            if (ImGui::InputFloat("Intensity", &intensity))
            {
                light.setIntensity(intensity);
                changed = true;
            }
            auto color = Colors::normalize(light.getColor());
            if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
            {
                light.setColor(Colors::denormalize(color));
                changed = true;
            }
        }
        return changed;
    }
}