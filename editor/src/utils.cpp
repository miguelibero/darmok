#include <darmok-editor/utils.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/material.hpp>

namespace darmok::editor
{
    const ImVec2& ImguiUtils::getAssetSize()
    {
        static const ImVec2 size = ImVec2(100, 100);
        return size;
    }

    bool ImguiUtils::drawAsset(const char* label, bool selected)
    {
        static const ImGuiSelectableFlags flags = 0;

        auto size = getAssetSize();
        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        selected = ImGui::Selectable(label, selected, flags, size);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        auto min = ImGui::GetItemRectMin();
        auto max = ImGui::GetItemRectMax();
        auto borderColor = selected ? IM_COL32(255, 0, 0, 255) : IM_COL32(200, 200, 200, 255);
        auto drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(min, max, borderColor, 0.0f, 0, 2.0f);
        return selected;
    }

    bool ImguiUtils::drawMaterialReference(const char* label, std::shared_ptr<Material>& mat)
    {
        return ImguiUtils::drawAssetReference(label, mat, mat->getName(), "MATERIAL");
    }

    bool ImguiUtils::drawProgramReference(const char* label, std::shared_ptr<Program>& prog, IProgramLoader& loader)
    {
        auto def = loader.getDefinition(prog);
        std::string name;
        if (!def)
        {
            if (auto standard = StandardProgramLoader::getType(prog))
            {
                name = StandardProgramLoader::getTypeName(standard.value());
            }
        }
        else
        {
            name = def->name;
        }
        if (drawAssetReference(label, def, name, "PROGRAM"))
        {
            prog = loader.loadResource(def);
            return true;
        }
        return false;
    }

    bool ImguiUtils::drawMeshReference(const char* label, std::shared_ptr<IMesh>& mesh, IMeshLoader& loader)
    {
        auto def = loader.getDefinition(mesh);
        std::string name = def ? def->name : "";
        if (drawAssetReference(label, def, name, "MESH"))
        {
            mesh = loader.loadResource(def);
            return true;
        }
        return false;
    }
}