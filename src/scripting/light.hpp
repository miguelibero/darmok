#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/color_fwd.hpp>
#include <sol/sol.hpp>
#include <glm/glm.hpp>

namespace darmok
{
    class PointLight;
	class LuaEntity;
	class LuaScene;

	class LuaPointLight final
	{
	public:
		LuaPointLight(PointLight& light) noexcept;

		const PointLight& getReal() const;
		PointLight& getReal();

		void setIntensity(float intensity) noexcept;
		void setRadius(float radius) noexcept;
		void setAttenuation(const glm::vec3& attn) noexcept;
		void setColor(const Color3& color) noexcept;
		void setDiffuseColor(const Color3& color) noexcept;
		void setSpecularColor(const Color3& color) noexcept;

		float getIntensity() const noexcept;
		float getRadius() const noexcept;
		const glm::vec3& getAttenuation() const noexcept;
		const Color3& getDiffuseColor() const noexcept;
		const Color3& getSpecularColor() const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<PointLight> _light;

		static LuaPointLight addEntityComponent1(LuaEntity& entity) noexcept;
		static LuaPointLight addEntityComponent2(LuaEntity& entity, float intensity) noexcept;
		static std::optional<LuaPointLight> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};

	class AmbientLight;

	class LuaAmbientLight final
	{
	public:
		LuaAmbientLight(AmbientLight& light) noexcept;

		const AmbientLight& getReal() const;
		AmbientLight& getReal();

		void setIntensity(float intensity) noexcept;
		void setColor(const Color3& color) noexcept;

		const Color3& getColor() const noexcept;
		float getIntensity() const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<AmbientLight> _light;

		static LuaAmbientLight addEntityComponent1(LuaEntity& entity) noexcept;
		static LuaAmbientLight addEntityComponent2(LuaEntity& entity, float intensity) noexcept;
		static std::optional<LuaAmbientLight> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};
}