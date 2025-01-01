#include <darmok-editor/inspector/scene.hpp>
#include <darmok/scene_reflect.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void SceneInspectorEditor::init(EditorApp& app, ObjectEditorContainer& editors) noexcept
    {
        _editors = editors;
    }

    bool SceneInspectorEditor::renderType(Scene& scene) noexcept
    {
        auto changed = false;

        if (ImGui::CollapsingHeader("Scene"))
        {
            {
                auto name = scene.getName();
                if (ImGui::InputText("Name", &name))
                {
                    scene.setName(name);
                    changed = true;
                }
                ImGui::Spacing();
            }

            if (_editors)
            {
                auto comps = SceneReflectionUtils::getSceneComponents(scene);
                if (!comps.empty())
                {
                    if (ImGui::CollapsingHeader("Components"))
                    {
                        for (auto& comp : comps)
                        {
                            if (_editors->render(comp))
                            {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
        return true;
    }
}