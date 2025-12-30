#include <darmok-editor/inspector/script.hpp>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    std::string LuaScriptInspectorEditor::getTitle() const noexcept
    {
        return "Lua Script";
    }

    LuaScriptInspectorEditor::RenderResult LuaScriptInspectorEditor::renderType(LuaScript::Definition& text) noexcept
    {
        auto changed = false;
        if (ImGui::InputTextMultiline(
            "##textarea",
            text.mutable_content(),
            ImVec2{ -FLT_MIN, 200 },
            ImGuiInputTextFlags_AllowTabInput
        ))
        {
            changed = true;
        }

        return changed;
    }

    std::string LuaScriptRunnerInspectorEditor::getTitle() const noexcept
    {
        return "Lua Script Runner";
    }

    LuaScriptRunnerInspectorEditor::RenderResult LuaScriptRunnerInspectorEditor::renderType(LuaScriptRunner::Definition& def) noexcept
    {
        return false;
    }
}