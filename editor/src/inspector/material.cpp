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
			if (ImguiUtils::drawProtobufInput("Base color", "base_color", mat))
			{
				changed = true;
			}
			if (ImguiUtils::drawProtobufInput("Emissive color", "emissive_color", mat))
			{
				changed = true;
			}
			if (ImguiUtils::drawProtobufInput("Metallic factor", "metallic_factor", mat))
			{
				changed = true;
			}
			if (ImguiUtils::drawProtobufInput("Roughness factor", "roughness_factor", mat))
			{
				changed = true;
			}
			if (ImguiUtils::drawProtobufInput("Normal scale", "normal_scale", mat))
			{
				changed = true;
			}
			if (ImguiUtils::drawProtobufInput("Occlusion Strength", "occlusion_strength", mat))
			{
				changed = true;
			}
			if (ImguiUtils::drawProtobufInput("Multiple scattering", "multiple_scattering", mat))
			{
				changed = true;
			}

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
			
			auto& proj = _app->getProject();

			if (ImGui::Button("Delete"))
			{
				proj.getSceneDefinition().removeAsset(mat);
				changed = true;
			}
		}
		return changed;
	}
}