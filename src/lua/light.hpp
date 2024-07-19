#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/color_fwd.hpp>
#include <sol/sol.hpp>
#include "glm.hpp"

namespace darmok
{
    class PointLight;
	class LuaEntity;
	class LuaScene;

	class LuaPointLight final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<PointLight> _light;

		static void setAttenuation(PointLight& light, const VarLuaTable<glm::vec3>& attn) noexcept;
		static void setColor(PointLight& light, const VarLuaTable<Color3>& color) noexcept;
		static void setDiffuseColor(PointLight& light, const VarLuaTable<Color3>& color) noexcept;
		static void setSpecularColor(PointLight& light, const VarLuaTable<Color3>& color) noexcept;

		static PointLight& addEntityComponent1(LuaEntity& entity) noexcept;
		static PointLight& addEntityComponent2(LuaEntity& entity, float intensity) noexcept;
		static OptionalRef<PointLight>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const PointLight& light, LuaScene& scene) noexcept;
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
		static std::optional<LuaEntity> getEntity(const AmbientLight& light, LuaScene& scene) noexcept;
	};

	class PhongLightingComponent;
	class ForwardRenderer;

	class LuaPhongLightingComponent final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static PhongLightingComponent& addRenderComponent(ForwardRenderer& renderer) noexcept;
	};
}