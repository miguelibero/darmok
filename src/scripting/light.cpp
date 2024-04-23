#include "light.hpp"
#include <darmok/light.hpp>

namespace darmok
{
    LuaPointLight::LuaPointLight(PointLight& light) noexcept
		: _light(light)
	{
	}

	const PointLight& LuaPointLight::getReal() const
	{
		return _light.value();
	}

	PointLight& LuaPointLight::getReal()
	{
		return _light.value();
	}

	void LuaPointLight::setIntensity(float intensity) noexcept
	{
		_light->setIntensity(intensity);
	}

	void LuaPointLight::setRadius(float radius) noexcept
	{
		_light->setRadius(radius);
	}

	void LuaPointLight::setAttenuation(const VarVec3& attn) noexcept
	{
		_light->setAttenuation(LuaMath::tableToGlm(attn));
	}

	void LuaPointLight::setColor(const VarColor3& color) noexcept
	{
		_light->setColor(LuaMath::tableToGlm(color));
	}

	void LuaPointLight::setDiffuseColor(const VarColor3& color) noexcept
	{
		_light->setDiffuseColor(LuaMath::tableToGlm(color));
	}

	void LuaPointLight::setSpecularColor(const VarColor3& color) noexcept
	{
		_light->setSpecularColor(LuaMath::tableToGlm(color));
	}

	float LuaPointLight::getIntensity() const noexcept
	{
		return _light->getIntensity();
	}

	float LuaPointLight::getRadius() const noexcept
	{
		return _light->getRadius();
	}

	const glm::vec3& LuaPointLight::getAttenuation() const noexcept
	{
		return _light->getAttenuation();
	}

	const Color3& LuaPointLight::getDiffuseColor() const noexcept
	{
		return _light->getDiffuseColor();
	}

	const Color3& LuaPointLight::getSpecularColor() const noexcept
	{
		return _light->getSpecularColor();
	}

	void LuaPointLight::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaPointLight>("PointLight",
			sol::constructors<>(),
			"intensity", sol::property(&LuaPointLight::getIntensity, &LuaPointLight::setIntensity),
			"radius", sol::property(&LuaPointLight::getRadius, &LuaPointLight::setRadius),
			"attenuation", sol::property(&LuaPointLight::getAttenuation, &LuaPointLight::setAttenuation),
			"diffuse_color", sol::property(&LuaPointLight::getDiffuseColor, &LuaPointLight::setDiffuseColor),
			"specular_color", sol::property(&LuaPointLight::getSpecularColor, &LuaPointLight::setSpecularColor),
			"color", sol::property(&LuaPointLight::getDiffuseColor, &LuaPointLight::setColor)
		);
	}

	LuaAmbientLight::LuaAmbientLight(AmbientLight& light) noexcept
		: _light(light)
	{
	}

	const AmbientLight& LuaAmbientLight::getReal() const
	{
		return _light.value();
	}

	AmbientLight& LuaAmbientLight::getReal()
	{
		return _light.value();
	}

	void LuaAmbientLight::setIntensity(float intensity) noexcept
	{
		_light->setIntensity(intensity);
	}

	void LuaAmbientLight::setColor(const VarColor3& color) noexcept
	{
		_light->setColor(LuaMath::tableToGlm(color));
	}

	const Color3& LuaAmbientLight::getColor() const noexcept
	{
		return _light->getColor();
	}

	float LuaAmbientLight::getIntensity() const noexcept
	{
		return _light->getIntensity();
	}

	void LuaAmbientLight::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaAmbientLight>("AmbientLight",
			sol::constructors<>(),
			"intensity", sol::property(&LuaAmbientLight::getIntensity, &LuaAmbientLight::setIntensity),
			"color", sol::property(&LuaAmbientLight::getColor, &LuaAmbientLight::setColor)
		);
	}
}