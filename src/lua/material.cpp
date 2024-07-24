#include "material.hpp"
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
			{ "Diffuse", MaterialTextureType::Diffuse },
			{ "Specular", MaterialTextureType::Specular },
			{ "Normal", MaterialTextureType::Normal },
		});
		lua.new_enum<MaterialColorType>("MaterialColorType", {
			{ "Diffuse", MaterialColorType::Diffuse },
			{ "Specular", MaterialColorType::Specular },
			{ "Ambient", MaterialColorType::Ambient },
			{ "Emissive", MaterialColorType::Emissive },
			{ "Transparent", MaterialColorType::Transparent },
			{ "Reflective", MaterialColorType::Reflective },
		});

		lua.new_usertype<Material>("Material",
			sol::factories(
				[]() { return std::make_shared<Material>();  },
				[](const std::shared_ptr<Texture>& tex) { return std::make_shared<Material>(tex);  },
				[](const std::shared_ptr<Program>& prog, const std::shared_ptr<Texture>& tex)
					{ return std::make_shared<Material>(prog, tex); }
			),
			"shininess", sol::property(&Material::getShininess, &Material::setShininess),
			"program", sol::property(&Material::getProgram, &Material::setProgram),
			"specular_strength", sol::property(&Material::getSpecularStrength, &Material::setSpecularStrength),
			"primitive_type", sol::property(&Material::getPrimitiveType, &Material::setPrimitiveType),
			"set_texture", &Material::setTexture,
			"get_texture", &Material::getTexture,
			"set_color", &Material::setColor,
			"get_color", &Material::getColor,
			"set_define", sol::overload(
				&Material::setProgramDefine,
				[](Material& mat, const std::string& define) { return mat.setProgramDefine(define); }
			),
			"set_defines", &Material::setProgramDefines
		);
	}
}