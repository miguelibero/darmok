#pragma once

#include <darmok/optional_ref.hpp>
#include "sol.hpp"
#include "math.hpp"
#include "scene_fwd.hpp"

namespace darmok
{
    class PointLight;

	class LuaPointLight final
	{
	public:
		using native_t = PointLight;
		const static LuaNativeComponentType native_type = LuaNativeComponentType::PointLight;

		LuaPointLight(PointLight& light) noexcept;

		const PointLight& getReal() const;
		PointLight& getReal();

		void setIntensity(float intensity) noexcept;
		void setRadius(float radius) noexcept;
		void setAttenuation(const VarVec3& attn) noexcept;
		void setColor(const VarColor3& color) noexcept;
		void setDiffuseColor(const VarColor3& color) noexcept;
		void setSpecularColor(const VarColor3& color) noexcept;

		float getIntensity() const noexcept;
		float getRadius() const noexcept;
		const glm::vec3& getAttenuation() const noexcept;
		const Color3& getDiffuseColor() const noexcept;
		const Color3& getSpecularColor() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<PointLight> _light;
	};

	class AmbientLight;

	class LuaAmbientLight final
	{
	public:
		using native_t = AmbientLight;
		const static LuaNativeComponentType native_type = LuaNativeComponentType::AmbientLight;

		LuaAmbientLight(AmbientLight& light) noexcept;

		const AmbientLight& getReal() const;
		AmbientLight& getReal();

		void setIntensity(float intensity) noexcept;
		void setColor(const VarColor3& color) noexcept;

		const Color3& getColor() const noexcept;
		float getIntensity() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<AmbientLight> _light;
	};
}