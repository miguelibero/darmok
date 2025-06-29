#include <darmok-editor/inspector/scene.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/scene_serialize.hpp>
#include <imgui.h>

namespace darmok::editor
{
    void SceneInspectorEditor::init(EditorApp& app, ObjectEditorContainer& editors) noexcept
    {
        _editors = editors;
    }

    std::string SceneInspectorEditor::getTitle() const noexcept
    {
        return "Scene";
    }

    bool SceneInspectorEditor::renderType(Scene::Definition& scene) noexcept
    {
        auto changed = false;
 
        if (ImguiUtils::drawProtobufInput("Name", "name", scene))
        {
            changed = true;
        }

        return true;
    }
}