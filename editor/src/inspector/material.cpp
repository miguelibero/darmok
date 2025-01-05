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
			{
				auto tex = mat.getTexture(MaterialTextureType::BaseColor);
				if (_app->drawTextureReference("Base Texture", tex))
				{
					mat.setTexture(MaterialTextureType::BaseColor, tex);
				}
			}
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
			{
				auto f = mat.getNormalScale();
				if (ImGui::SliderFloat("Normal Scale", &f, 0.F, 1.F))
				{
					mat.setNormalScale(f);
					changed = true;
				}
			}
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

			ImGui::BeginChild("Basic");
			{
				auto color = Colors::normalize(mat.getSpecularColor());
				if (ImGui::ColorEdit3("Specular Color", glm::value_ptr(color)))
				{
					mat.setSpecularColor(Colors::denormalize(color));
					changed = true;
				}
			}
			{
				int v = mat.getShininess();
				if (ImGui::SliderInt("Shininess", &v, 0, 256))
				{
					mat.setShininess(v);
					changed = true;
				}
			}
			ImGui::EndChild();

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