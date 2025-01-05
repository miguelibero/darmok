#include <darmok-editor/inspector/texture.hpp>
#include <darmok-editor/app.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void TextureDefinitionInspectorEditor::init(EditorApp& app, ObjectEditorContainer& container)
    {
        _app = app;
    }

    void TextureDefinitionInspectorEditor::shutdown()
    {
        _app.reset();
    }

    bool TextureDefinitionInspectorEditor::renderType(TextureDefinition& def) noexcept
    {
        auto changed = false;

        if (ImGui::CollapsingHeader("Texture"))
        {
            {
                if (ImGui::InputText("Name", &def.name))
                {
                    changed = true;
                }
                ImGui::Spacing();
            }

            auto proj = _app->getProject();

            if (ImGui::Button("Apply Changes"))
            {
                proj.reloadTexture(def);
            }

            ImGui::SameLine();

            if (ImGui::Button("Delete"))
            {
                proj.removeTexture(def);
                changed = true;
            }
        }
        return false;
    }
}