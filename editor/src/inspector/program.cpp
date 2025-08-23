#include <darmok-editor/inspector/program.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/program.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    std::string ProgramSourceInspectorEditor::getTitle() const noexcept
    {
        return "Program";
    }

    const std::string ProgramSourceInspectorEditor::_shaderFilter = "*.txt *.sc";

    ProgramSourceInspectorEditor::RenderResult ProgramSourceInspectorEditor::renderType(protobuf::ProgramSource& src) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Name", "name", src))
        {
            changed = true;
        }
        return changed;
    }
}