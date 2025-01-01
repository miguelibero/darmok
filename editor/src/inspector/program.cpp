#include <darmok-editor/inspector/program.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/program.hpp>
#include <darmok/asset.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void ProgramSourceInspectorEditor::init(EditorApp& editor, ObjectEditorContainer& container)
    {
        _editor = editor;
    }

    void ProgramSourceInspectorEditor::shutdown()
    {
        _editor.reset();
    }

    bool ProgramSourceInspectorEditor::render(ProgramSource& src) noexcept
    {
        if (ImGui::CollapsingHeader("Program"))
        {
            ImGui::InputText("Name", &src.name);

            if (ImGui::Button("Delete"))
            {
                _editor->getProject().removeProgram(src);
            }
        }
        return true;
    }
}