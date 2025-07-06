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

	bool MaterialTextureEditor::render(Material::Definition& mat, AssetPack& assets) noexcept
	{
		static const glm::vec2 maxPreviewSize{ 100.F };

		bool changed = false;
		MaterialDefinitionWrapper wrapper{ mat };
		std::string assetPath = wrapper.getTexturePath(_type).value_or("");
		auto action = ImguiUtils::drawTextureReferenceInput(_label.c_str(), assetPath, _tex, assets, maxPreviewSize);
		if (action == ReferenceInputAction::Changed)
		{
			wrapper.setTexturePath(_type, assetPath);
			changed = true;
		}

		return changed;
	}

	void MaterialInspectorEditor::init(EditorApp& app, ObjectEditorContainer& container)
	{
		_app = app;
		for(auto [val, name] : StringUtils::getEnumValues<Material::TextureDefinition::Type>())
		{
			StringUtils::camelCaseToHumanReadable(name);
			name = name + " Texture";
			_textureEditors.emplace(val, MaterialTextureEditor{ name, val });
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
		return itr->second.render(mat, _app->getProject().getAssets());
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

	bool MaterialInspectorEditor::renderType(Material::Definition& mat) noexcept
	{
		auto changed = false;

		auto standardProgram = EditorStandardProgramType::Custom;
		if (mat.has_standard_program())
		{
			standardProgram = static_cast<EditorStandardProgramType>(1 + mat.standard_program());
		}
		if(ImguiUtils::drawEnumCombo("Program", standardProgram))
		{
			changed = true;
			if (standardProgram == EditorStandardProgramType::Custom)
			{
				mat.clear_standard_program();
			}
			else
			{
				mat.set_standard_program(StandardProgramLoader::Type{ toUnderlying(standardProgram) - 1 });
			}
		}
		if (standardProgram == EditorStandardProgramType::Custom)
		{
			auto action = ImguiUtils::drawProtobufAssetReferenceInput("Program Path", "program_path", mat, "Program");
			if (action == ReferenceInputAction::Changed)
			{
				changed = true;
			}
		}

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
			
		auto& proj = _app->getProject();

		if (ImGui::Button("Delete"))
		{
			proj.getSceneDefinition().removeAsset(mat);
			changed = true;
		}

		return changed;
	}
}