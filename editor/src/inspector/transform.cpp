#include <darmok-editor/inspector/transform.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/project.hpp>

#include <darmok/glm.hpp>
#include <darmok/transform.hpp>
#include <darmok/protobuf/scene.pb.h>

#include <imgui.h>

namespace darmok::editor
{
    std::string TransformInspectorEditor::getTitle() const noexcept
    {
		return "Transform";
    }

    TransformInspectorEditor::RenderResult TransformInspectorEditor::renderType(Transform::Definition& trans) noexcept
    {
        static const std::unordered_map<std::string, std::string> labels
        {
            {"name", "Name"},
            {"position", "Position"},
            {"rotation", "Rotation"},
            {"scale", "Scale"},
        };

        auto changed = false;
        if (ImguiUtils::drawProtobufInputs(labels, trans))
        {
            changed = true;
        }
        auto& sceneDef = getProject().getSceneDefinition();
        auto result = ImguiUtils::drawProtobufEntityReferenceInput("Parent", "parent", trans, sceneDef);
        if (result == ReferenceInputAction::Changed)
        {
            changed = true;
        }
        return changed;
    }
}