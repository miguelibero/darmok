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

	bool MaterialTextureEditor::render(Material::Definition& mat, ITextureLoader& loader) noexcept
	{
		static const glm::vec2 maxPreviewSize{ 100.F };

		bool changed = false;
		MaterialDefinitionWrapper wrapper{ mat };
		std::string assetPath = wrapper.getTexturePath(_type).value_or("");
		auto action = ImguiUtils::drawTextureReferenceInput(_label.c_str(), assetPath, _tex, loader, maxPreviewSize);
		if (action == ReferenceInputAction::Changed)
		{
			wrapper.setTexturePath(_type, assetPath);
			changed = true;
		}

		return changed;
	}

	void MaterialInspectorEditor::init(EditorApp& app, ObjectEditorContainer& container)
	{
		AssetObjectEditor<Material::Definition>::init(app, container);	
		for(auto [val, name] : StringUtils::getEnumValues<Material::TextureDefinition::Type>())
		{
			StringUtils::camelCaseToHumanReadable(name);
			name = name + " Texture";
			_textureEditors.emplace(val, MaterialTextureEditor{ name, val });
		}
	}

	void MaterialInspectorEditor::shutdown()
	{
		AssetObjectEditor<Material::Definition>::shutdown();
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
		return itr->second.render(mat, getProject().getAssets().getTextureLoader());
	}

	MaterialInspectorEditor::RenderResult MaterialInspectorEditor::renderType(Material::Definition& mat) noexcept
	{
		auto changed = false;

		auto progResult = renderChild(*mat.mutable_program());
		if(!progResult)
		{
			return unexpected{ std::move(progResult).error() };
		}
		changed |= *progResult;

		if (ImguiUtils::drawProtobufInput("Base color", "base_color", mat))
		{
			changed = true;
		}
		if(renderTextureInput(mat, Material::TextureDefinition::BaseColor))
		{
			changed = true;
		}

		if (ImguiUtils::drawProtobufSliderInput("Metallic factor", "metallic_factor", mat, 0.f, 1.f))
		{
			changed = true;
		}
		if (ImguiUtils::drawProtobufSliderInput("Roughness factor", "roughness_factor", mat, 0.f, 1.f))
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

		if (ImguiUtils::drawProtobufSliderInput("Normal scale", "normal_scale", mat, 0.f, 1.f))
		{
			changed = true;
		}
		if (renderTextureInput(mat, Material::TextureDefinition::Normal))
		{
			changed = true;
		}

		if (ImguiUtils::drawProtobufSliderInput("Occlusion Strength", "occlusion_strength", mat, 0.f, 1.f))
		{
			changed = true;
		}
		if (ImguiUtils::drawProtobufSliderInput("Multiple scattering", "multiple_scattering", mat, 0.f, 1.f))
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

		if (ImguiUtils::drawProtobufSliderInput("Shininess", "shininess", mat, 2, 500))
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
			
		return changed;
	}
}