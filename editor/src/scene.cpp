#include <darmok-editor/scene.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/scene_reflect.hpp>

namespace darmok::editor
{
    void SceneInspectorEditor::init(EditorAppDelegate& app, ObjectEditorContainer& editors) noexcept
    {
        _editors = editors;
    }

    bool SceneInspectorEditor::render(Scene& scene) noexcept
    {
        if (!_editors)
        {
            return false;
        }
        if (ImGui::CollapsingHeader("Scene"))
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
        return true;
    }
}