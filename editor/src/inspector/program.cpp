#include <darmok-editor/inspector/program.hpp>
#include <darmok-editor/project.hpp>
#include <darmok/program.hpp>
#include <darmok/asset.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void ProgramInspectorEditor::init(AssetContext& assets, EditorProject& proj, ObjectEditorContainer& container)
    {
        _assets = assets;
        _proj = proj;
    }

    void ProgramInspectorEditor::shutdown()
    {
        _assets.reset();
        _proj.reset();
    }

    bool ProgramInspectorEditor::render(ProgramSource& src) noexcept
    {
        if (ImGui::CollapsingHeader("Program"))
        {
            ImGui::InputText("Name", &src.name);

            if (ImGui::Button("Delete"))
            {
                auto& progs = _proj->getPrograms();
                auto ptr = &src;
                auto itr = std::find_if(progs.begin(), progs.end(), [ptr](auto& elm) { return elm.get() == ptr; });
                if (itr != progs.end())
                {
                    progs.erase(itr);
                }
            }
        }
        return true;
    }
}