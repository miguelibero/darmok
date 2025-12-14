#include <darmok-editor/inspector/scene.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/scene_serialize.hpp>
#include <imgui.h>

namespace darmok::editor
{
    std::string SceneInspectorEditor::getTitle() const noexcept
    {
        return "Scene";
    }

    SceneInspectorEditor::RenderResult SceneInspectorEditor::renderType(Scene::Definition& scene) noexcept
    {
        auto changed = false;
 
        if (ImguiUtils::drawProtobufInput("Name", "name", scene))
        {
            changed = true;
        }

        if (scene.scene_components_size() > 0)
        {
            if (ImGui::CollapsingHeader("Components"))
            {
                for (auto& [typeId, comp] : *scene.mutable_scene_components())
                {
                    if (renderChild(comp))
                    {
                        changed = true;
                    }
                }
            }
        }

        return true;
    }
}