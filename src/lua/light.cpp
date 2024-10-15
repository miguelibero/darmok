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
			"radius", sol::property(&PointLight::getRadius, &PointLight::setRadius),
			"color", sol::property(&PointLight::getColor, &LuaPointLight::setColor)
		);
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
			"color", sol::property(&DirectionalLight::getColor, &LuaDirectionalLight::setColor)
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

		lua.new_usertype<LightingRenderComponent>("LightingRenderComponent", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<LightingRenderComponent>::value),
			"add_camera_component", &LuaLightingRenderComponent::addCameraComponent,
			"get_camera_component", &LuaLightingRenderComponent::getCameraComponent
		);
	}
}