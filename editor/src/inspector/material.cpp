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
		auto changed = true;
		auto drawTexture = [this, &mat, &changed](const char* label, MaterialTextureType type)
		{
			auto tex = mat.getTexture(type);
			if (_app->drawTextureReference(label, tex))
			{
				mat.setTexture(type, tex);
				changed = true;
			}
		};

		if (ImGui::CollapsingHeader("Material"))
		{
			{
				std::string name = mat.getName();
				if (ImGui::InputText("Name", &name))
				{
					mat.setName(name);
					changed = true;
				}
			}
			if (_app)
			{
				auto prog = mat.getProgram();
				if (_app->drawProgramReference("Program", prog))
				{
					mat.setProgram(prog);
					changed = true;
				}
			}
			{
				auto color = Colors::normalize(mat.getBaseColor());
				if (ImGui::ColorEdit4("Base Color", glm::value_ptr(color)))
				{
					mat.setBaseColor(Colors::denormalize(color));
					changed = true;
				}
			}
			drawTexture("Base Texture", MaterialTextureType::BaseColor);
			{
				auto f = mat.getMetallicFactor();
				if (ImGui::SliderFloat("Metallic Factor", &f, 0.F, 1.F))
				{
					mat.setMetallicFactor(f);
					changed = true;
				}
			}
			{
				auto f = mat.getRoughnessFactor();
				if (ImGui::SliderFloat("Roughness Factor", &f, 0.F, 1.F))
				{
					mat.setRoughnessFactor(f);
					changed = true;
				}
			}
			drawTexture("Metallic Roughness Texture", MaterialTextureType::MetallicRoughness);
			{
				auto f = mat.getNormalScale();
				if (ImGui::SliderFloat("Normal Scale", &f, 0.F, 1.F))
				{
					mat.setNormalScale(f);
					changed = true;
				}
			}
			drawTexture("Normal Texture", MaterialTextureType::Normal);
			{
				auto f = mat.getOcclusionStrength();
				if (ImGui::SliderFloat("Occlusion Strength", &f, 0.F, 1.F))
				{
					mat.setOcclusionStrength(f);
					changed = true;
				}
			}
			{
				auto color = Colors::normalize(mat.getEmissiveColor());
				if (ImGui::ColorEdit3("Emissive Color", glm::value_ptr(color)))
				{
					mat.setEmissiveColor(Colors::denormalize(color));
					changed = true;
				}
			}
			drawTexture("Emissive Texture", MaterialTextureType::Emissive);
			{
				auto prim = mat.getPrimitiveType();
				if (ImguiUtils::drawEnumCombo("Primitive Type", prim, Material::getPrimitiveTypeName))
				{
					mat.setPrimitiveType(prim);
					changed = true;
				}
			}
			{
				auto opacity = mat.getOpacityType();
				if (ImguiUtils::drawEnumCombo("Opacity Type", opacity, Material::getOpacityTypeName))
				{
					mat.setOpacityType(opacity);
					changed = true;
				}
			}
			{
				auto twoSided = mat.getTwoSided();
				if (ImGui::Checkbox("Two Sided", &twoSided))
				{
					mat.setTwoSided(twoSided);
					changed = true;
				}
			}
			// phong lighting: specular + shininess
			ImGui::Separator();
			ImGui::Text("Basic");
			{
				auto color = Colors::normalize(mat.getSpecularColor());
				if (ImGui::ColorEdit3("Specular Color", glm::value_ptr(color)))
				{
					mat.setSpecularColor(Colors::denormalize(color));
					changed = true;
				}
			}
			drawTexture("Specular Texture", MaterialTextureType::Specular);
			{
				int v = mat.getShininess();
				if (ImGui::SliderInt("Shininess", &v, 0, 256))
				{
					mat.setShininess(v);
					changed = true;
				}
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