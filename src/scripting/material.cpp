#include "material.hpp"
#include "texture.hpp"
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

	LuaMaterial::LuaMaterial(const LuaTexture& texture) noexcept
		: _material(std::make_shared<Material>(texture.getReal()))
	{
	}

	LuaMaterial::LuaMaterial(const LuaRenderTexture& texture) noexcept
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

	void LuaMaterial::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaMaterial>("Material",
			sol::constructors<LuaMaterial(), LuaMaterial(LuaTexture), LuaMaterial(LuaRenderTexture)>(),
			"shininess", sol::property(&LuaMaterial::getShininess, &LuaMaterial::setShininess),
			"specular_strength", sol::property(&LuaMaterial::getSpecularStrength, &LuaMaterial::setSpecularStrength)
		);
	}
}