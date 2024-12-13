#include <darmok-editor/material_inspector.hpp>
#include <darmok-editor/utils.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
	bool MaterialInspectorEditor::render(Material& mat) noexcept
	{
		if (ImGui::CollapsingHeader("Material"))
		{
			{
				std::string name = mat.getName();
				if (ImGui::InputText("Name", &name))
				{
					mat.setName(name);
				}
			}
			{
				auto color = Colors::normalize(mat.getBaseColor());
				if (ImGui::ColorEdit4("Base Color", glm::value_ptr(color)))
				{
					mat.setBaseColor(Colors::denormalize(color));
				}
			}
			{
				auto f = mat.getMetallicFactor();
				if (ImGui::SliderFloat("Metallic Factor", &f, 0.F, 1.F))
				{
					mat.setMetallicFactor(f);
				}
			}
			{
				auto f = mat.getRoughnessFactor();
				if (ImGui::SliderFloat("Roughness Factor", &f, 0.F, 1.F))
				{
					mat.setRoughnessFactor(f);
				}
			}
			{
				auto f = mat.getNormalScale();
				if (ImGui::SliderFloat("Normal Scale", &f, 0.F, 1.F))
				{
					mat.setNormalScale(f);
				}
			}
			{
				auto f = mat.getOcclusionStrength();
				if (ImGui::SliderFloat("Occlusion Strength", &f, 0.F, 1.F))
				{
					mat.setOcclusionStrength(f);
				}
			}
			{
				auto color = Colors::normalize(mat.getEmissiveColor());
				if (ImGui::ColorEdit3("Emissive Color", glm::value_ptr(color)))
				{
					mat.setEmissiveColor(Colors::denormalize(color));
				}
			}
			{
				auto prim = mat.getPrimitiveType();
				if (ImguiUtils::drawEnumCombo("Primitive Type", prim, Material::getPrimitiveTypeName))
				{
					mat.setPrimitiveType(prim);
				}
			}
			{
				auto opacity = mat.getOpacityType();
				if (ImguiUtils::drawEnumCombo("Opacity Type", opacity, Material::getOpacityTypeName))
				{
					mat.setOpacityType(opacity);
				}
			}
			{
				auto twoSided = mat.getTwoSided();
				if (ImGui::Checkbox("Two Sided", &twoSided))
				{
					mat.setTwoSided(twoSided);
				}
			}
			// phong lighting: specular + shininess
			{
				auto color = Colors::normalize(mat.getSpecularColor());
				if (ImGui::ColorEdit3("Specular Color", glm::value_ptr(color)))
				{
					mat.setSpecularColor(Colors::denormalize(color));
				}
			}
			{
				int v = mat.getShininess();
				if (ImGui::SliderInt("Shininess", &v, 0, 256))
				{
					mat.setShininess(v);
				}
			}
		}
		return true;
	}
}