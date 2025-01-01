#include <darmok-editor/inspector/transform.hpp>
#include <darmok-editor/utils.hpp>

#include <darmok/glm.hpp>
#include <darmok/transform.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    bool TransformInspectorEditor::renderType(Transform& trans) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Transform"))
        {
            {
                std::string name = trans.getName();
                if (ImGui::InputText("Name", &name))
                {
                    trans.setName(name);
                    changed = true;
                }
            }

            {
                auto pos = trans.getPosition();
                if (ImguiUtils::drawPositionInput("Position", pos))
                {
                    trans.setPosition(pos);
                    changed = true;
                }
            }
            {
                auto rot = trans.getRotation();
                if (ImguiUtils::drawRotationInput("Rotation", rot))
                {
                    trans.setRotation(rot);
                    changed = true;
                }
            }
            {
                auto scale = trans.getScale();
                if (ImguiUtils::drawScaleInput("Scale", scale))
                {
                    trans.setScale(scale);
                    changed = true;
                }
            }
        }
        return changed;
    }
}