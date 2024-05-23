#include "material.hpp"
#include "texture.hpp"
#include "program.hpp"
#include <darmok/material.hpp>
#include <darmok/texture.hpp>

namespace darmok
{
    	LuaMaterial::LuaMaterial() noexcept
		: _material(std::make_shared<Material>())
	{
	}

	LuaMaterial::LuaMaterial(const std::shared_ptr<Material>& material) noexcept
		: _material(material)
	{
	}

	LuaMaterial::LuaMaterial(const LuaProgram& program, const LuaTexture& texture) noexcept
		: _material(std::make_shared<Material>(program.getReal(), texture.getReal()))
	{
	}

	LuaMaterial::LuaMaterial(const LuaTexture& texture) noexcept
		: _material(std::make_shared<Material>(texture.getReal()))
	{
	}

	const std::shared_ptr<Material>& LuaMaterial::getReal() const noexcept
	{
		return _material;
	}

	uint8_t LuaMaterial::getShininess() const noexcept
	{
		return _material->getShininess();
	}

	LuaMaterial& LuaMaterial::setShininess(uint8_t v) noexcept
	{
		_material->setShininess(v);
		return *this;
	}

	float LuaMaterial::getSpecularStrength() const noexcept
	{
		return _material->getSpecularStrength();
	}

	LuaMaterial& LuaMaterial::setSpecularStrength(float v) noexcept
	{
		_material->setSpecularStrength(v);
		return *this;
	}

	MaterialPrimitiveType LuaMaterial::getPrimitiveType() const noexcept
	{
		return _material->getPrimitiveType();
	}

	LuaMaterial& LuaMaterial::setPrimitiveType(MaterialPrimitiveType type) noexcept
	{
		_material->setPrimitiveType(type);
		return *this;
	}

	LuaProgram LuaMaterial::getProgram() const noexcept
	{
		return LuaProgram(_material->getProgram());
	}

	LuaMaterial& LuaMaterial::setProgram(const LuaProgram& program) noexcept
	{
		_material->setProgram(program.getReal());
		return *this;
	}

	void LuaMaterial::bind(sol::state_view& lua) noexcept
	{
		lua.new_enum<MaterialPrimitiveType>("MaterialPrimitiveType", {
			{ "triangle", MaterialPrimitiveType::Triangle },
			{ "line", MaterialPrimitiveType::Line }
		});

		lua.new_usertype<LuaMaterial>("Material",
			sol::constructors<LuaMaterial(), LuaMaterial(LuaProgram, LuaTexture), LuaMaterial(LuaTexture)>(),
			"shininess", sol::property(&LuaMaterial::getShininess, &LuaMaterial::setShininess),
			"program", sol::property(&LuaMaterial::getProgram, &LuaMaterial::setProgram),
			"specular_strength", sol::property(&LuaMaterial::getSpecularStrength, &LuaMaterial::setSpecularStrength),
			"primitive_type", sol::property(&LuaMaterial::getPrimitiveType, &LuaMaterial::setPrimitiveType)
		);
	}
}