#include <darmok-editor/inspector/material.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>

#include <imgui.h>

namespace darmok::editor
{
	MaterialTextureEditor::MaterialTextureEditor(std::string_view label, Material::TextureDefinition::Type type)
		: _label{ label }
		, _type{ type }
	{
	}

	bool MaterialTextureEditor::render(Material::Definition& mat, AssetContext& assets) noexcept
	{
		bool changed = false;
		MaterialDefinitionWrapper wrapper{ mat };
		std::string assetPath = wrapper.getTexturePath(_type).value_or("");
		auto action = ImguiUtils::drawTextureReferenceInput(_label.c_str(), assetPath, _tex, assets);
		if (action == ReferenceInputAction::Changed)
		{
			changed = true;
		}
		return changed;
	}

	void MaterialInspectorEditor::init(EditorApp& app, ObjectEditorContainer& container)
	{
		_app = app;
		for(auto& [val, name] : StringUtils::getEnumValues<Material::TextureDefinition::Type>())
		{
			_textureEditors.emplace(val, name, val);
		}
	}

	void MaterialInspectorEditor::shutdown()
	{
		_app.reset();
		_textureEditors.clear();
	}

	std::string MaterialInspectorEditor::getTitle() const noexcept
	{
		return "Material";
	}

	bool MaterialInspectorEditor::renderTextureInput(Material::Definition& mat, Material::TextureDefinition::Type type) noexcept
	{
		auto itr = _textureEditors.find(type);
		if(itr == _textureEditors.end())
		{
			return false;
		}
		return itr->second.render(mat, _app->getAssets());
	}

	bool MaterialInspectorEditor::renderType(Material::Definition& mat) noexcept
	{
		auto changed = false;

		if (ImguiUtils::drawProtobufInput("Base color", "base_color", mat))
		{
			changed = true;
		}
		if(renderTextureInput(mat, Material::TextureDefinition::BaseColor))
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
		if (renderTextureInput(mat, Material::TextureDefinition::MetallicRoughness))
		{
			changed = true;
		}

		if (ImguiUtils::drawProtobufInput("Emissive color", "emissive_color", mat))
		{
			changed = true;
		}
		if (renderTextureInput(mat, Material::TextureDefinition::Emissive))
		{
			changed = true;
		}

		if (ImguiUtils::drawProtobufInput("Normal scale", "normal_scale", mat))
		{
			changed = true;
		}
		if (renderTextureInput(mat, Material::TextureDefinition::Normal))
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
		if (renderTextureInput(mat, Material::TextureDefinition::Occlusion))
		{
			changed = true;
		}

		if (ImguiUtils::drawProtobufInput("Specular", "specular_color", mat))
		{
			changed = true;
		}
		if (renderTextureInput(mat, Material::TextureDefinition::Specular))
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

		return changed;
	}
}