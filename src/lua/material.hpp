#pragma once

#include "lua/lua.hpp"
#include "lua/glm.hpp"

#include <darmok/protobuf/material.pb.h>

namespace darmok
{
	struct Material;
	class Program;
	class Texture;

	class LuaMaterial final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		using MaterialTextureType = protobuf::MaterialTexture::Type;
		using MaterialPrimitiveType = protobuf::Material::PrimitiveType;
		using MaterialOpacityType = protobuf::Material::OpacityType;
		using MaterialShadingModel = protobuf::Material::ShadingModel;

		static std::shared_ptr<Material> create1(const std::shared_ptr<Texture>& tex) noexcept;
		static std::shared_ptr<Material> create2(const VarLuaTable<Color>& color) noexcept;

		static void setTexture1(Material& mat, const std::shared_ptr<Texture>& tex) noexcept;
		static void setTexture2(Material& mat, MaterialTextureType type, const std::shared_ptr<Texture>& tex) noexcept;
		static void setTexture3(Material& mat, const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& tex) noexcept;

		static std::shared_ptr<Texture> getTexture1(const Material& mat) noexcept;
		static std::shared_ptr<Texture> getTexture2(const Material& mat, MaterialTextureType type) noexcept;
		static std::shared_ptr<Texture> getTexture3(const Material& mat, const std::string& name, uint8_t stage) noexcept;

		static void setUniform1(Material& mat, const std::string& name, const VarLuaTable<glm::vec4>& val);
		static void setUniform2(Material& mat, const std::string& name, const VarLuaTable<glm::mat4>& val);
		static void setUniform3(Material& mat, const std::string& name, const VarLuaTable<glm::mat3>& val);
	};

}
