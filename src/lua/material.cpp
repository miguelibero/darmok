#include "material.hpp"
#include "glm.hpp"
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>

namespace darmok
{
	void LuaMaterial::bind(sol::state_view& lua) noexcept
	{
		lua.new_enum<MaterialPrimitiveType>("MaterialPrimitiveType", {
			{ "Triangle", MaterialPrimitiveType::Triangle },
			{ "Line", MaterialPrimitiveType::Line }
		});

		lua.new_enum<MaterialTextureType>("MaterialTextureType", {
			{ "BaseColor", MaterialTextureType::BaseColor },
			{ "Specular", MaterialTextureType::Specular },
			{ "MetallicRoughness", MaterialTextureType::MetallicRoughness },
			{ "Normal", MaterialTextureType::Normal },
			{ "Occlusion", MaterialTextureType::Occlusion },
			{ "Emissive", MaterialTextureType::Emissive },
		});

		lua.new_usertype<Material>("Material",
			sol::factories(
				[]() { return std::make_shared<Material>();  },
				[](const std::shared_ptr<Program>& prog){ return std::make_shared<Material>(prog); },
				[](const std::shared_ptr<Texture>& tex) { return std::make_shared<Material>(tex);  },
				[](const std::shared_ptr<Program>& prog, const std::shared_ptr<Texture>& tex)
					{ return std::make_shared<Material>(prog, tex); },
				[](const std::shared_ptr<Program>& prog, const VarLuaTable<Color>& color)
					{ return std::make_shared<Material>(prog, LuaGlm::tableGet(color)); }
			),
			"program", sol::property(&Material::getProgram, &Material::setProgram),
			"primitive_type", sol::property(&Material::getPrimitiveType, &Material::setPrimitiveType),
			"set_texture", sol::overload(
				sol::resolve<Material&(const std::shared_ptr<Texture>&)>(&Material::setTexture),
				sol::resolve<Material&(MaterialTextureType, const std::shared_ptr<Texture>&)>(&Material::setTexture),
				sol::resolve<Material&(const std::string&, uint8_t, const std::shared_ptr<Texture>&)>(&Material::setTexture)
			),
			"get_texture", &Material::getTexture,
			"set_uniform", &Material::setUniform,
			"set_define", sol::overload(
				&Material::setProgramDefine,
				[](Material& mat, const std::string& define) { return mat.setProgramDefine(define); }
			),
			"set_defines", &Material::setProgramDefines,
			"base_color", sol::property(&Material::getBaseColor, &Material::setBaseColor),
			"specular_color", sol::property(&Material::getSpecularColor, &Material::setSpecularColor),
			"metallic_factor", sol::property(&Material::getMetallicFactor, &Material::setMetallicFactor),
			"roughness_factor", sol::property(&Material::getRoughnessFactor, &Material::setRoughnessFactor),
			"normal_scale", sol::property(&Material::getNormalScale, &Material::setNormalScale),
			"occlusion_strength", sol::property(&Material::getOcclusionStrength, &Material::setOcclusionStrength),
			"emissive_color", sol::property(&Material::getEmissiveColor, &Material::setEmissiveColor),
			"two_sided", sol::property(&Material::getTwoSided, &Material::setTwoSided),
			"multiple_scattering", sol::property(&Material::getMultipleScattering, &Material::setMultipleScattering),
			"white_furnance_factor", sol::property(&Material::getWhiteFurnanceFactor, &Material::setWhiteFurnanceFactor)
		);
	}
}