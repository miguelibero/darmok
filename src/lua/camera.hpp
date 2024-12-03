#pragma once

#include "lua.hpp"
#include "glm.hpp"
#include "viewport.hpp"
#include "utils.hpp"

#include <darmok/optional_ref.hpp>
#include <darmok/shape.hpp>

#include <optional>
#include <vector>

namespace darmok
{
    class Camera;
	class Program;
	class Texture;
	class LuaEntity;
	class Scene;
	class Transform;
	class RenderChain;
	struct EntityFilter;

	class LuaCamera final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;

	private:
		static Camera& addEntityComponent(LuaEntity& entity) noexcept;
		static OptionalRef<Camera>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const Camera& cam, const std::shared_ptr<Scene>& scene) noexcept;

		static Camera& setPerspective1(Camera& cam, float fovy) noexcept;
		static Camera& setPerspective2(Camera& cam, float fovy, float near, float far) noexcept;
		static Camera& setOrtho1(Camera& cam, const VarLuaTable<glm::vec2>& center) noexcept;
		static Camera& setOrtho2(Camera& cam, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept;

		static void setBaseViewport(Camera& cam, std::optional<VarViewport> viewport) noexcept;
		std::optional<Viewport> getBaseViewport(const Camera& cam) noexcept;
		static void setViewport(Camera& cam, const VarLuaTable<glm::vec4>& viewport) noexcept;
		glm::vec4 getViewport(const Camera& cam) noexcept;

		static OptionalRef<Transform>::std_t getTransform(const Camera& cam) noexcept;

		static Ray screenPointToRay1(const Camera& cam, const glm::vec2& point) noexcept;
		static Ray screenPointToRay2(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept;
		static Ray viewportPointToRay(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept;
		static glm::vec3 worldToScreenPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept;
		static glm::vec3 worldToViewportPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept;
		static glm::vec3 screenToWorldPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept;
		static glm::vec3 viewportToWorldPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept;
		static glm::vec3 viewportToScreenPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept;
		static glm::vec3 screenToViewportPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept;
		static bool isWorldPointVisible(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept;

		static void setCullingFilter(Camera& cam, const sol::object& filter) noexcept;
	};
}