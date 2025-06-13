#include <darmok-editor/inspector/material.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>

#include <imgui.h>

namespace darmok::editor
{
	void MaterialInspectorEditor::init(EditorApp& app, ObjectEditorContainer& container)
	{
		_app = app;
	}

	void MaterialInspectorEditor::shutdown()
	{
		_app.reset();
	}

	bool MaterialInspectorEditor::renderType(Material::Definition& mat) noexcept
	{
		auto changed = false;

		if (ImGui::CollapsingHeader("Material"))
		{
			if (ImguiUtils::drawProtobufInput("Specular", "specular_color", mat))
			{
				changed = true;
			}
			if (ImguiUtils::drawProtobufInput("Shininess", "shininess", mat))
			{
				changed = true;
			}
			if (ImguiUtils::drawProtobufInput("Primitive Type", "primitive_type", mat))
			{
				changed = true;
			}
			if (ImguiUtils::drawProtobufInput("Opacity Type", "opacity_type", mat))
			{
				changed = true;
			}
			if (ImguiUtils::drawProtobufInput("Two Sided", "twosided", mat))
			{
				changed = true;
			}
			
			auto proj = _app->getProject();

			if (ImGui::Button("Delete"))
			{
				// proj.removeMaterial(mat);
				changed = true;
			}
		}
		return changed;
	}
}