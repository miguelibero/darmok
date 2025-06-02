#include "light.hpp"
#include "scene.hpp"
#include "render_scene.hpp"
#include <darmok/light.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
	void LuaPointLight::setColor(PointLight& light, const VarLuaTable<Color3>& color) noexcept
	{
		light.setColor(LuaGlm::tableGet(color));
	}

	PointLight& LuaPointLight::addEntityComponent1(LuaEntity& entity) noexcept
	{
		return entity.addComponent<PointLight>();
	}

	PointLight& LuaPointLight::addEntityComponent2(LuaEntity& entity, float intensity) noexcept
	{
		return entity.addComponent<PointLight>(intensity);
	}

	OptionalRef<PointLight>::std_t LuaPointLight::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<PointLight>();
	}

	std::optional<LuaEntity> LuaPointLight::getEntity(const PointLight& light, const std::shared_ptr<Scene>& scene) noexcept
	{
		return LuaScene::getEntity(scene, light);
	}

	void LuaPointLight::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<PointLight>("PointLight", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<PointLight>::value),
			"add_entity_component", sol::overload(
				&LuaPointLight::addEntityComponent1,
				&LuaPointLight::addEntityComponent2
			),
			"get_entity_component", &LuaPointLight::getEntityComponent,
			"get_entity", &LuaPointLight::getEntity,
			"intensity", sol::property(&PointLight::getIntensity, &PointLight::setIntensity),
			"range", sol::property(&PointLight::getRange, &PointLight::setRange),
			"color", sol::property(&PointLight::getColor, &LuaPointLight::setColor),
			"shadow_type", sol::property(&PointLight::getShadowType, &PointLight::setShadowType)
		);
	}

	void LuaSpotLight::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<SpotLight>("SpotLight", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<SpotLight>::value),
			"add_entity_component", sol::overload(
				&LuaSpotLight::addEntityComponent1,
				&LuaSpotLight::addEntityComponent2
			),
			"get_entity_component", &LuaSpotLight::getEntityComponent,
			"get_entity", &LuaSpotLight::getEntity,
			"intensity", sol::property(&SpotLight::getIntensity, &SpotLight::setIntensity),
			"range", sol::property(&SpotLight::getRange, &SpotLight::setRange),
			"color", sol::property(&SpotLight::getColor, &SpotLight::setColor),
			"cone_angle", sol::property(&SpotLight::getConeAngle, &SpotLight::setConeAngle),
			"inner_cone_angle", sol::property(&SpotLight::getInnerConeAngle, &SpotLight::setInnerConeAngle),
			"shadow_type", sol::property(&SpotLight::getShadowType, &SpotLight::setShadowType)
		);
	}

	void LuaSpotLight::setColor(SpotLight& light, const VarLuaTable<Color3>& color) noexcept
	{
		light.setColor(LuaGlm::tableGet(color));
	}

	SpotLight& LuaSpotLight::addEntityComponent1(LuaEntity& entity) noexcept
	{
		return entity.addComponent<SpotLight>();
	}

	SpotLight& LuaSpotLight::addEntityComponent2(LuaEntity& entity, float intensity) noexcept
	{
		return entity.addComponent<SpotLight>(intensity);
	}

	OptionalRef<SpotLight>::std_t LuaSpotLight::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<SpotLight>();
	}

	std::optional<LuaEntity> LuaSpotLight::getEntity(const SpotLight& light, const std::shared_ptr<Scene>& scene) noexcept
	{
		return LuaScene::getEntity(scene, light);
	}

	void LuaDirectionalLight::setColor(DirectionalLight& light, const VarLuaTable<Color3>& color) noexcept
	{
		light.setColor(LuaGlm::tableGet(color));
	}

	DirectionalLight& LuaDirectionalLight::addEntityComponent1(LuaEntity& entity) noexcept
	{
		return entity.addComponent<DirectionalLight>();
	}

	DirectionalLight& LuaDirectionalLight::addEntityComponent2(LuaEntity& entity, float intensity) noexcept
	{
		return entity.addComponent<DirectionalLight>(intensity);
	}

	OptionalRef<DirectionalLight>::std_t LuaDirectionalLight::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<DirectionalLight>();
	}

	std::optional<LuaEntity> LuaDirectionalLight::getEntity(const DirectionalLight& light, const std::shared_ptr<Scene>& scene) noexcept
	{
		return LuaScene::getEntity(scene, light);
	}

	void LuaDirectionalLight::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<DirectionalLight>("DirectionalLight", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<DirectionalLight>::value),
			"add_entity_component", sol::overload(
				&LuaDirectionalLight::addEntityComponent1,
				&LuaDirectionalLight::addEntityComponent2
			),
			"get_entity_component", &LuaDirectionalLight::getEntityComponent,
			"get_entity", &LuaDirectionalLight::getEntity,

			"intensity", sol::property(&DirectionalLight::getIntensity, &DirectionalLight::setIntensity),
			"color", sol::property(&DirectionalLight::getColor, &LuaDirectionalLight::setColor),
			"shadow_type", sol::property(&DirectionalLight::getShadowType, &DirectionalLight::setShadowType)
		);
	}

	void LuaAmbientLight::setColor(AmbientLight& light, const VarLuaTable<Color3>& color) noexcept
	{
		light.setColor(LuaGlm::tableGet(color));
	}

	AmbientLight& LuaAmbientLight::addEntityComponent1(LuaEntity& entity) noexcept
	{
		return entity.addComponent<AmbientLight>();
	}

	AmbientLight& LuaAmbientLight::addEntityComponent2(LuaEntity& entity, float intensity) noexcept
	{
		return entity.addComponent<AmbientLight>(intensity);
	}

	OptionalRef<AmbientLight>::std_t LuaAmbientLight::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<AmbientLight>();
	}

	std::optional<LuaEntity> LuaAmbientLight::getEntity(const AmbientLight& light, const std::shared_ptr<Scene>& scene) noexcept
	{
		return LuaScene::getEntity(scene, light);
	}

	void LuaAmbientLight::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<AmbientLight>("AmbientLight", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<AmbientLight>::value),
			"add_entity_component", sol::overload(
				&LuaAmbientLight::addEntityComponent1,
				&LuaAmbientLight::addEntityComponent2
			),
			"get_entity_component", &LuaAmbientLight::getEntityComponent,
			"get_entity", &LuaAmbientLight::getEntity,

			"intensity", sol::property(&AmbientLight::getIntensity, &AmbientLight::setIntensity),
			"color", sol::property(&AmbientLight::getColor, &LuaAmbientLight::setColor)
		);
	}

	LightingRenderComponent& LuaLightingRenderComponent::addCameraComponent(Camera& cam) noexcept
	{
		return cam.addComponent<LightingRenderComponent>();
	}

	OptionalRef<LightingRenderComponent>::std_t LuaLightingRenderComponent::getCameraComponent(Camera& cam) noexcept
	{
		return cam.getComponent<LightingRenderComponent>();
	}

	void LuaLightingRenderComponent::bind(sol::state_view& lua) noexcept
	{
		LuaAmbientLight::bind(lua);
		LuaDirectionalLight::bind(lua);
		LuaPointLight::bind(lua);
		LuaSpotLight::bind(lua);

		LuaUtils::newEnum<LightDefinition::ShadowType>(lua, "ShadowType");

		lua.new_usertype<LightingRenderComponent>("LightingRenderComponent", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<LightingRenderComponent>::value),
			"add_camera_component", &LuaLightingRenderComponent::addCameraComponent,
			"get_camera_component", &LuaLightingRenderComponent::getCameraComponent
		);
	}
}