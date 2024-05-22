#pragma once

#include <optional>
#include <vector>
#include <darmok/optional_ref.hpp>
#include <darmok/shape.hpp>
#include <sol/sol.hpp>
#include "glm.hpp"

namespace darmok
{
    class Camera;
	class LuaProgram;
	class LuaTexture;
	class LuaEntity;
	class LuaScene;

	enum class LuaNativeCameraComponentType
	{
		PhongLighting
	};

	class LuaCamera final
	{
	public:
		LuaCamera(Camera& camera) noexcept;

		const Camera& getReal() const;
		Camera& getReal();

		glm::uvec2 getScreenPoint(const VarLuaTable<glm::vec2>& normPoint) const noexcept;

		LuaCamera& setProjection1(float fovy, float aspect, const VarLuaTable<glm::vec2>& range) noexcept;
		LuaCamera& setProjection2(float fovy, float aspect, float near) noexcept;
		LuaCamera& setProjection3(float fovy, const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& range) noexcept;
		LuaCamera& setProjection4(float fovy, const VarLuaTable<glm::uvec2>& size, float near) noexcept;
		LuaCamera& setOrtho1(const VarLuaTable<glm::vec4>& edges, const VarLuaTable<glm::vec2>& range) noexcept;
		LuaCamera& setOrtho2(const VarLuaTable<glm::vec4>& edges) noexcept;
		LuaCamera& setOrtho3(const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& range) noexcept;
		LuaCamera& setOrtho4(const VarLuaTable<glm::uvec2>& size) noexcept;
		LuaCamera& setTargetTextures(const sol::table& textures) noexcept;
		LuaCamera& addNativeComponent(LuaNativeCameraComponentType type) noexcept;
		LuaCamera& setForwardRenderer(const LuaProgram& program) noexcept;

		std::vector<LuaTexture> getTargetTextures() noexcept;
		const glm::mat4& getMatrix() const noexcept;
		void setMatrix(const VarLuaTable<glm::mat4>& matrix) noexcept;
		Ray screenPointToRay(const VarLuaTable<glm::vec2>& point) const noexcept;
		glm::vec3 worldToScreenPoint(const VarLuaTable<glm::vec3>& position) const noexcept;

		static void bind(sol::state_view& lua) noexcept;

	private:
		OptionalRef<Camera> _camera;

		static LuaCamera addEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaCamera> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};

}