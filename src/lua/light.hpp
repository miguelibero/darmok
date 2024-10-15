#pragma once

#include "lua.hpp"
#include "glm.hpp"

#include <darmok/optional_ref.hpp>
#include <darmok/color_fwd.hpp>

namespace darmok
{
    class PointLight;
	class LuaEntity;
	class Scene;

	class LuaPointLight final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<PointLight> _light;

		static void setColor(PointLight& light, const VarLuaTable<Color3>& color) noexcept;

		static PointLight& addEntityComponent1(LuaEntity& entity) noexcept;
		static PointLight& addEntityComponent2(LuaEntity& entity, float intensity) noexcept;
		static OptionalRef<PointLight>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const PointLight& light, const std::shared_ptr<Scene>& scene) noexcept;
	};

	class DirectionalLight;

	class LuaDirectionalLight final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static void setColor(DirectionalLight& light, const VarLuaTable<Color3>& color) noexcept;

		static DirectionalLight& addEntityComponent1(LuaEntity& entity) noexcept;
		static DirectionalLight& addEntityComponent2(LuaEntity& entity, float intensity) noexcept;
		static OptionalRef<DirectionalLight>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const DirectionalLight& light, const std::shared_ptr<Scene>& scene) noexcept;
	};

	class AmbientLight;

	class LuaAmbientLight final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static void setColor(AmbientLight& light, const VarLuaTable<Color3>& color) noexcept;

		static AmbientLight& addEntityComponent1(LuaEntity& entity) noexcept;
		static AmbientLight& addEntityComponent2(LuaEntity& entity, float intensity) noexcept;
		static OptionalRef<AmbientLight>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const AmbientLight& light, const std::shared_ptr<Scene>& scene) noexcept;
	};

	class LightingRenderComponent;
	class Camera;

	class LuaLightingRenderComponent final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static LightingRenderComponent& addCameraComponent(Camera& cam) noexcept;
		static OptionalRef<LightingRenderComponent>::std_t getCameraComponent(Camera& cam) noexcept;
	};
}