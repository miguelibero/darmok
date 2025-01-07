#include <darmok-editor/inspector/program.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/program.hpp>
#include <darmok/asset.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void ProgramSourceInspectorEditor::init(EditorApp& app, ObjectEditorContainer& container)
    {
        _app = app;
    }

    void ProgramSourceInspectorEditor::shutdown()
    {
        _app.reset();
    }

    const std::string ProgramSourceInspectorEditor::_shaderFilter = "*.txt *.sc";

    bool ProgramSourceInspectorEditor::renderType(ProgramSource& src) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Program"))
        {
            if (ImGui::InputText("Name", &src.name))
            {
                changed = true;
            }

            if (_app)
            {
                std::filesystem::path fsPath;
                if (ImguiUtils::drawFileInput("Load Fragment Shader", fsPath, _shaderFilter))
                {
                    src.fragmentShader = Data::fromFile(fsPath);
                    changed = true;
                }
                std::filesystem::path vsPath;
                if (ImguiUtils::drawFileInput("Load Vertex Shader", vsPath, _shaderFilter))
                {
                    src.vertexShader = Data::fromFile(vsPath);
                    changed = true;
                }

                auto& proj = _app->getProject();
                if (ImGui::Button("Apply"))
                {
                    proj.reloadProgram(src);
                    changed = true;
                }

                if (ImGui::Button("Delete"))
                {
                    proj.removeProgram(src);
                    changed = true;
                }
            }
        }
        return changed;
    }
}