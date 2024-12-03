#include <darmok-editor/light.hpp>
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

    bool PointLightInspectorEditor::render(PointLight& light) noexcept
    {
        if (ImGui::CollapsingHeader("Point Light"))
        {
            float intensity = light.getIntensity();
            if (ImGui::InputFloat("Intensity", &intensity))
            {
                light.setIntensity(intensity);
            }
            float range = light.getRange();
            if (ImGui::InputFloat("Range", &range))
            {
                light.setRange(range);
            }
            auto color = Colors::normalize(light.getColor());
            if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
            {
                light.setColor(Colors::denormalize(color));
            }
            auto shadowType = light.getShadowType();
            if (LightEditorUtils::renderShadowTypeInput(shadowType))
            {
                light.setShadowType(shadowType);
            }
        }
        return true;
    }

    bool DirectionalLightInspectorEditor::render(DirectionalLight& light) noexcept
    {
        if (ImGui::CollapsingHeader("Directional Light"))
        {
            float intensity = light.getIntensity();
            if (ImGui::InputFloat("Intensity", &intensity))
            {
                light.setIntensity(intensity);
            }
            auto color = Colors::normalize(light.getColor());
            if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
            {
                light.setColor(Colors::denormalize(color));
            }
            auto shadowType = light.getShadowType();
            if (LightEditorUtils::renderShadowTypeInput(shadowType))
            {
                light.setShadowType(shadowType);
            }
        }
        return true;
    }

    bool SpotLightInspectorEditor::render(SpotLight& light) noexcept
    {
        if (ImGui::CollapsingHeader("Spot Light"))
        {
            float intensity = light.getIntensity();
            if (ImGui::InputFloat("Intensity", &intensity))
            {
                light.setIntensity(intensity);
            }
            float range = light.getRange();
            if (ImGui::InputFloat("Range", &range))
            {
                light.setRange(range);
            }
            auto color = Colors::normalize(light.getColor());
            if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
            {
                light.setColor(Colors::denormalize(color));
            }
            auto shadowType = light.getShadowType();
            if (LightEditorUtils::renderShadowTypeInput(shadowType))
            {
                light.setShadowType(shadowType);
            }
        }
        return true;
    }

    bool AmbientLightInspectorEditor::render(AmbientLight& light) noexcept
    {
        if (ImGui::CollapsingHeader("Ambient Light"))
        {
            float intensity = light.getIntensity();
            if (ImGui::InputFloat("Intensity", &intensity))
            {
                light.setIntensity(intensity);
            }
            auto color = Colors::normalize(light.getColor());
            if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
            {
                light.setColor(Colors::denormalize(color));
            }
        }
        return true;
    }
}