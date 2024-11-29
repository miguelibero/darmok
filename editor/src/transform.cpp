#include <darmok-editor/transform.hpp>
#include <darmok/glm.hpp>
#include <darmok/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void TransformEditor::render(Transform& trans) noexcept
    {
        if (ImGui::CollapsingHeader("Transform"))
        {
            {
                std::string name = trans.getName();
                if (ImGui::InputText("Name", &name))
                {
                    trans.setName(name);
                }
            }

            {
                auto pos = trans.getPosition();
                if (ImGui::InputFloat3("Position", glm::value_ptr(pos)))
                {
                    trans.setPosition(pos);
                }
            }
            {
                auto rot = trans.getRotation();
                if (ImGui::InputFloat4("Rotation", glm::value_ptr(rot)))
                {
                    trans.setRotation(rot);
                }
            }
            {
                auto scale = trans.getScale();
                if (ImGui::InputFloat3("Scale", glm::value_ptr(scale)))
                {
                    trans.setScale(scale);
                }
            }
        }
    }
}