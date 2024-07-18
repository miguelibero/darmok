#include "light.hpp"
#include "scene.hpp"
#include "render.hpp"
#include <darmok/light.hpp>
#include <darmok/render_forward.hpp>

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

	void LuaPointLight::setAttenuation(const VarLuaTable<glm::vec3>& attn) noexcept
	{
		_light->setAttenuation(LuaGlm::tableGet(attn));
	}

	void LuaPointLight::setColor(const VarLuaTable<Color3>& color) noexcept
	{
		_light->setColor(LuaGlm::tableGet(color));
	}

	void LuaPointLight::setDiffuseColor(const VarLuaTable<Color3>& color) noexcept
	{
		_light->setDiffuseColor(LuaGlm::tableGet(color));
	}

	void LuaPointLight::setSpecularColor(const VarLuaTable<Color3>& color) noexcept
	{
		_light->setSpecularColor(LuaGlm::tableGet(color));
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

	LuaPointLight LuaPointLight::addEntityComponent1(LuaEntity& entity) noexcept
	{
		return entity.addComponent<PointLight>();
	}

	LuaPointLight LuaPointLight::addEntityComponent2(LuaEntity& entity, float intensity) noexcept
	{
		return entity.addComponent<PointLight>(intensity);
	}

	std::optional<LuaPointLight> LuaPointLight::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<PointLight, LuaPointLight>();
	}

	std::optional<LuaEntity> LuaPointLight::getEntity(LuaScene& scene) noexcept
	{
		return scene.getEntity(_light.value());
	}

	void LuaPointLight::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaPointLight>("PointLight", sol::no_constructor,
			"type_id", &entt::type_hash<PointLight>::value,
			"add_entity_component", sol::overload(
				&LuaPointLight::addEntityComponent1,
				&LuaPointLight::addEntityComponent2
			),
			"get_entity_component", &LuaPointLight::getEntityComponent,
			"get_entity", &LuaPointLight::getEntity,
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

	void LuaAmbientLight::setColor(const VarLuaTable<Color3>& color) noexcept
	{
		_light->setColor(LuaGlm::tableGet(color));
	}

	const Color3& LuaAmbientLight::getColor() const noexcept
	{
		return _light->getColor();
	}

	float LuaAmbientLight::getIntensity() const noexcept
	{
		return _light->getIntensity();
	}

	LuaAmbientLight LuaAmbientLight::addEntityComponent1(LuaEntity& entity) noexcept
	{
		return entity.addComponent<AmbientLight>();
	}

	LuaAmbientLight LuaAmbientLight::addEntityComponent2(LuaEntity& entity, float intensity) noexcept
	{
		return entity.addComponent<AmbientLight>(intensity);
	}

	std::optional<LuaAmbientLight> LuaAmbientLight::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<AmbientLight, LuaAmbientLight>();
	}

	std::optional<LuaEntity> LuaAmbientLight::getEntity(LuaScene& scene) noexcept
	{
		return scene.getEntity(_light.value());
	}

	void LuaAmbientLight::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaAmbientLight>("AmbientLight", sol::no_constructor,
			"type_id", &entt::type_hash<AmbientLight>::value,
			"add_entity_component", sol::overload(
				&LuaAmbientLight::addEntityComponent1,
				&LuaAmbientLight::addEntityComponent2
			),
			"get_entity_component", &LuaAmbientLight::getEntityComponent,
			"get_entity", &LuaAmbientLight::getEntity,

			"intensity", sol::property(&LuaAmbientLight::getIntensity, &LuaAmbientLight::setIntensity),
			"color", sol::property(&LuaAmbientLight::getColor, &LuaAmbientLight::setColor)
		);
	}

	LuaPhongLightingComponent::LuaPhongLightingComponent(PhongLightingComponent& comp) noexcept
		: _comp(comp)
	{
	}

	LuaPhongLightingComponent LuaPhongLightingComponent::addRenderComponent(ForwardRenderer& renderer) noexcept
	{
		return renderer.addComponent<PhongLightingComponent>();
	}

	void LuaPhongLightingComponent::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaPhongLightingComponent>("PhongLightingComponent",
			sol::no_constructor,
			"add_render_component", &LuaPhongLightingComponent::addRenderComponent
		);
	}
}