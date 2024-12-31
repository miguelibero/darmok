#include <darmok-editor/inspector/program.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/program.hpp>
#include <darmok/asset.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void ProgramInspectorEditor::init(EditorApp& editor, ObjectEditorContainer& container)
    {
        _editor = editor;
    }

    void ProgramInspectorEditor::shutdown()
    {
        _editor.reset();
    }

    bool ProgramInspectorEditor::render(ProgramSource& src) noexcept
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