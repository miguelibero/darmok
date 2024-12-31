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

    bool SceneInspectorEditor::render(Scene& scene) noexcept
    {
        if (ImGui::CollapsingHeader("Scene"))
        {
            {
                auto name = scene.getName();
                if (ImGui::InputText("Name", &name))
                {
                    scene.setName(name);
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
                            _editors->render(comp);
                        }
                    }
                }
            }
        }
        return true;
    }
}