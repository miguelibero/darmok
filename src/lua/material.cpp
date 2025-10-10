#include "lua/material.hpp"
#include "lua/glm.hpp"
#include "lua/utils.hpp"
#include "lua/protobuf.hpp"
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>

namespace darmok
{
	std::shared_ptr<Material> LuaMaterial::create1(const std::shared_ptr<Program>& prog) noexcept
	{
		auto mat = std::make_shared<Material>();
		mat->program = prog;
		return mat;
	}

	std::shared_ptr<Material> LuaMaterial::create2(const std::shared_ptr<Texture>& tex) noexcept
	{
		auto mat = std::make_shared<Material>();
		mat->textures[Material::TextureDefinition::BaseColor] = tex;
		return mat;
	}

	std::shared_ptr<Material> LuaMaterial::create3(const std::shared_ptr<Program>& prog, const std::shared_ptr<Texture>& tex) noexcept
	{
		auto mat = std::make_shared<Material>();
		mat->program = prog;
		mat->textures[Material::TextureDefinition::BaseColor] = tex;
		return mat;
	}

	std::shared_ptr<Material> LuaMaterial::create4(const std::shared_ptr<Program>& prog, const VarLuaTable<Color>& color) noexcept
	{
		auto mat = std::make_shared<Material>();
		mat->program = prog;
		mat->baseColor = LuaGlm::tableGet(color);
		return mat;
	}

	void LuaMaterial::setTexture1(Material& mat, const std::shared_ptr<Texture>& tex) noexcept
	{
		setTexture2(mat, Material::TextureDefinition::BaseColor, tex);
	}

	void LuaMaterial::setTexture2(Material& mat, MaterialTextureType type, const std::shared_ptr<Texture>& tex) noexcept
	{
		mat.textures[type] = tex;
	}

	void LuaMaterial::setTexture3(Material& mat, const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& tex) noexcept
	{
		mat.uniformTextures[Texture::createUniformKey(name, stage)] = tex;
	}

	std::shared_ptr<Texture> LuaMaterial::getTexture1(const Material& mat) noexcept
	{
		return getTexture2(mat, Material::TextureDefinition::BaseColor);
	}

	std::shared_ptr<Texture> LuaMaterial::getTexture2(const Material& mat, MaterialTextureType type) noexcept
	{
		auto itr = mat.textures.find(type);
		if (itr == mat.textures.end())
		{
			return nullptr;
		}
		return itr->second;
	}

	std::shared_ptr<Texture> LuaMaterial::getTexture3(const Material& mat, const std::string& name, uint8_t stage) noexcept
	{
		auto itr = mat.uniformTextures.find(Texture::createUniformKey(name, stage));
		if (itr == mat.uniformTextures.end())
		{
			return nullptr;
		}
		return itr->second;
	}

	void LuaMaterial::setUniform1(Material& mat, const std::string& name, const VarLuaTable<glm::vec4>& val)
	{
		mat.uniformValues[name] = LuaGlm::tableGet(val);
	}

	void LuaMaterial::setUniform2(Material& mat, const std::string& name, const VarLuaTable<glm::mat4>& val)
	{
		mat.uniformValues[name] = LuaGlm::tableGet(val);
	}

	void LuaMaterial::setUniform3(Material& mat, const std::string& name, const VarLuaTable<glm::mat3>& val)
	{
		mat.uniformValues[name] = LuaGlm::tableGet(val);
	}

	void LuaMaterial::setDefine(Material& mat, const std::string& define)
	{
		mat.programDefines.insert(define);
	}


	void LuaMaterial::bind(sol::state_view& lua) noexcept
	{
		LuaUtils::newEnum<MaterialPrimitiveType>(lua, "MaterialPrimitiveType");
		LuaUtils::newEnum<MaterialTextureType>(lua, "MaterialTextureType");
		LuaUtils::newEnum<MaterialOpacityType>(lua, "MaterialOpacityType");
		LuaUtils::newProtobuf<Material::Definition>(lua, "MaterialDefinition")
			.protobufProperty<protobuf::ProgramRef>("program")
			.protobufProperty<protobuf::MaterialTexture>("textures")
			.protobufProperty<protobuf::UniformValue>("uniform_values")
			.protobufProperty<protobuf::Color>("base_color")
			.protobufProperty<protobuf::Color3>("emissive_color")
			.protobufProperty<protobuf::Color3>("specular_color");

		lua.new_usertype<Material>("Material",
			sol::factories(
				[]() { return std::make_shared<Material>();  },
				&LuaMaterial::create1,
				&LuaMaterial::create2,
				&LuaMaterial::create3,
				&LuaMaterial::create4
			),
			"program", &Material::program,
			"primitive_type", &Material::primitiveType,
			"set_texture", sol::overload(
				&LuaMaterial::setTexture1,
				&LuaMaterial::setTexture2,
				&LuaMaterial::setTexture3
			),
			"get_texture", sol::overload(
				&LuaMaterial::getTexture1,
				&LuaMaterial::getTexture2,
				&LuaMaterial::getTexture3
			),
			"set_uniform", sol::overload(
				&LuaMaterial::setUniform1,
				&LuaMaterial::setUniform2,
				&LuaMaterial::setUniform3
			),
			"set_define", sol::overload(
				&LuaMaterial::setDefine
			),
			"defines", & Material::programDefines,
			"base_color", &Material::baseColor,
			"specular_color", &Material::specularColor,
			"metallic_factor", &Material::metallicFactor,
			"roughness_factor", &Material::roughnessFactor,
			"normal_scale", &Material::normalScale,
			"occlusion_strength", &Material::occlusionStrength,
			"emissive_color", &Material::emissiveColor,
			"two_sided", &Material::twoSided,
			"multiple_scattering", &Material::multipleScattering,
			"white_furnance_factor", &Material::whiteFurnanceFactor
		);
	}
}