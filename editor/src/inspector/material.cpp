#include <darmok-editor/inspector/material.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

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

	bool MaterialInspectorEditor::renderType(Material& mat) noexcept
	{
		auto changed = false;
		auto drawTexture = [this, &mat, &changed](const char* label, auto textureType)
		{
			if (_app->drawTextureReference(label, mat.textures[textureType]))
			{
				changed = true;
			}
		};

		auto drawColorTexture = [this, &mat, &changed](const char* label, auto& color, auto textureType)
		{
			if (ImguiUtils::drawColorInput(label, color))
			{
				changed = true;
			}
			if (_app->drawTextureReference(label, mat.textures[textureType]))
			{
				changed = true;
			}
		};

		if (ImGui::CollapsingHeader("Material"))
		{
			if (_app)
			{
				if (_app->drawProgramReference("Program", mat.program))
				{
					changed = true;
				}
			}
			// pbr
			drawColorTexture("Base Texture", mat.baseColor, MaterialTextureType::BaseColor);
			if (ImGui::SliderFloat("Metallic Factor", &mat.metallicFactor, 0.F, 1.F))
			{
				changed = true;
			}
			if (ImGui::SliderFloat("Roughness Factor", &mat.roughnessFactor, 0.F, 1.F))
			{
				changed = true;
			}
			drawTexture("Metallic Roughness", MaterialTextureType::MetallicRoughness);
			if (ImGui::SliderFloat("Normal Scale", &mat.normalScale, 0.F, 1.F))
			{
				changed = true;
			}
			drawTexture("Normal", MaterialTextureType::Normal);
			if (ImGui::SliderFloat("Occlusion Strength", &mat.occlusionStrength, 0.F, 1.F))
			{
				changed = true;
			}
			if (ImguiUtils::drawColorInput("Emissive Color", mat.emissiveColor))
			{

				changed = true;
			}
			drawTexture("Emissive", MaterialTextureType::Emissive);
			
			// phong
			drawColorTexture("Specular", mat.specularColor, MaterialTextureType::Specular);
			{
				int v = mat.shininess;
				if (ImGui::SliderInt("Shininess", &v, 0, 256))
				{
					mat.shininess = v;
					changed = true;
				}
			}

			if (ImguiUtils::drawEnumCombo("Primitive Type", mat.primitiveType))
			{
				changed = true;
			}
			if (ImguiUtils::drawEnumCombo("Opacity Type", mat.opacityType))
			{
				changed = true;
			}
			if (ImGui::Checkbox("Two Sided", &mat.twoSided))
			{
				changed = true;
			}
			
			auto proj = _app->getProject();

			if (ImGui::Button("Delete"))
			{
				proj.removeMaterial(mat);
				changed = true;
			}
		}
		return changed;
	}
}