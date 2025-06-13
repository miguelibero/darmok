#include <darmok-editor/inspector/transform.hpp>
#include <darmok-editor/utils.hpp>

#include <darmok/glm.hpp>
#include <darmok/transform.hpp>
#include <darmok/protobuf/scene.pb.h>

#include <imgui.h>

namespace darmok::editor
{
    
    bool TransformInspectorEditor::renderType(Transform::Definition& trans) noexcept
    {
        static const std::unordered_map<std::string, std::string> labels
        {
            {"name", "Name"},
            {"position", "Position"},
            {"rotation", "Rotation"},
            {"scale", "Scale"},
        };

        auto changed = false;
        if (ImGui::CollapsingHeader("Transform"))
        {
            if (ImguiUtils::drawProtobufInputs(labels, trans))
            {
                changed = true;
            }
        }
        return changed;
    }
}