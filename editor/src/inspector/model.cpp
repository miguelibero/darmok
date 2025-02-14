#include <darmok-editor/inspector/model.hpp>
#include <darmok-editor/app.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void ModelInspectorEditor::init(EditorApp& app, ObjectEditorContainer& container)
    {
        _app = app;
        _container = container;
    }

    void ModelInspectorEditor::shutdown()
    {
        _app.reset();
        _container.reset();
    }

    const std::string ModelInspectorEditor::_fileFilter = "*.fbx *.glb *.gltf *.obj";

    bool ModelInspectorEditor::renderType(Model& model) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Model"))
        {
            if (ImGui::InputText("Name", &model.name))
            {
                changed = true;
            }
            auto proj = _app->getProject();

            std::filesystem::path path;
            if (ImguiUtils::drawFileInput("Load File", path, _fileFilter))
            {
                src.fragmentShader = Data::fromFile(fsPath);
                changed = true;
            }

            if (ImGui::Button("Apply Changes"))
            {
                proj.reloadModel(model);
            }

            ImGui::SameLine();

            if (ImGui::Button("Delete"))
            {
                proj.removeModel(model);
                changed = true;
            }
        }
        return changed;
    }
}
