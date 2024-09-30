#pragma once

#include <optional>
#include <vector>
#include <darmok/optional_ref.hpp>
#include <darmok/shape.hpp>
#include <sol/sol.hpp>
#include "glm.hpp"
#include "viewport.hpp"
#include "utils.hpp"

namespace darmok
{
    class Camera;
	class Program;
	class Texture;
	class LuaEntity;
	class Scene;
	class LuaScene;
	class Transform;
	class RenderChain;
	class RenderGraphDefinition;
	struct EntityFilter;

	class LuaCamera final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;

	private:
		static Camera& addEntityComponent(LuaEntity& entity) noexcept;
		static OptionalRef<Camera>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const Camera& cam, LuaScene& scene) noexcept;

		static Camera& setPerspective1(Camera& cam, float fovy, float aspect, float near, float far) noexcept;
		static Camera& setPerspective2(Camera& cam, float fovy, float aspect, float near) noexcept;
		static Camera& setPerspective3(Camera& cam, float fovy, const VarLuaTable<glm::uvec2>& size, float near, float far) noexcept;
		static Camera& setPerspective4(Camera& cam, float fovy, const VarLuaTable<glm::uvec2>& size, float near) noexcept;
		static Camera& setOrtho1(Camera& cam, const VarViewport& vp, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept;
		static Camera& setOrtho2(Camera& cam, const VarViewport& vp, const VarLuaTable<glm::vec2>& center) noexcept;
		static Camera& setOrtho3(Camera& cam, const VarViewport& vp) noexcept;
		static Camera& setOrtho4(Camera& cam, const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept;
		static Camera& setOrtho5(Camera& cam, const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& center) noexcept;
		static Camera& setOrtho6(Camera& cam, const VarLuaTable<glm::uvec2>& size) noexcept;

		static Camera& setViewportPerspective1(Camera& cam, float fovy) noexcept;
		static Camera& setViewportPerspective2(Camera& cam, float fovy, float near, float far) noexcept;
		static Camera& setViewportOrtho1(Camera& cam, const VarLuaTable<glm::vec2>& center) noexcept;
		static Camera& setViewportOrtho2(Camera& cam, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept;

		static void setProjectionMatrix(Camera& cam, const VarLuaTable<glm::mat4>& matrix) noexcept;
		
		static std::optional<Viewport> getViewport(const Camera& cam) noexcept;
		static void setViewport(Camera& cam, std::optional<VarViewport> viewport) noexcept;

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

		static void setCullingFilter(Camera& cam, const sol::object& filter) noexcept;
	};

	class LuaEntityFilter final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static EntityFilter& include(EntityFilter& filter, const sol::object& type) noexcept;
		static EntityFilter& exclude(EntityFilter& filter, const sol::object& type) noexcept;
	};
}