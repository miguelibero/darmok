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

    std::string ProgramRefInspectorEditor::getTitle() const noexcept
    {
		return "Program Ref";
    }

	enum class EditorStandardProgramType
	{
		Custom,
		Unlit,
		ForwardBasic,
		Forward,
		Gui,
		Tonemap,
	};

    ProgramRefInspectorEditor::RenderResult ProgramRefInspectorEditor::renderType(protobuf::ProgramRef& ref) noexcept
    {
		auto changed = false;
		auto standardProgram = EditorStandardProgramType::Custom;
		if (ref.has_standard())
		{
			standardProgram = static_cast<EditorStandardProgramType>(1 + ref.standard());
		}
		if (ImguiUtils::drawEnumCombo("Program", standardProgram))
		{
			changed = true;
			if (standardProgram == EditorStandardProgramType::Custom)
			{
				ref.clear_standard();
			}
			else
			{
				ref.set_standard(StandardProgramLoader::Type{ toUnderlying(standardProgram) - 1 });
			}
		}
		if (standardProgram == EditorStandardProgramType::Custom)
		{
			auto action = ImguiUtils::drawProtobufAssetReferenceInput("Program Path", "path", ref, "Program");
			if (action == ReferenceInputAction::Changed)
			{
				changed = true;
			}
		}

		return changed;
    }
}