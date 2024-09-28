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

	class LuaCamera final
	{
	public:
		LuaCamera(Camera& cam, const std::weak_ptr<Scene>& scene) noexcept;
		Camera& getReal() noexcept;
		const Camera& getReal() const noexcept;

		static void bind(sol::state_view& lua) noexcept;

	private:
		Camera& _cam;
		std::weak_ptr<Scene> _scene;

		static LuaCamera& addEntityComponent(LuaEntity& entity) noexcept;
		static OptionalRef<LuaCamera>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) const noexcept;

		LuaCamera& setPerspective1(float fovy, float aspect, float near, float far) noexcept;
		LuaCamera& setPerspective2(float fovy, float aspect, float near) noexcept;
		LuaCamera& setPerspective3(float fovy, const VarLuaTable<glm::uvec2>& size, float near, float far) noexcept;
		LuaCamera& setPerspective4(float fovy, const VarLuaTable<glm::uvec2>& size, float near) noexcept;
		LuaCamera& setOrtho1(const VarViewport& vp, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept;
		LuaCamera& setOrtho2(const VarViewport& vp, const VarLuaTable<glm::vec2>& center) noexcept;
		LuaCamera& setOrtho3(const VarViewport& vp) noexcept;
		LuaCamera& setOrtho4(const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept;
		LuaCamera& setOrtho5(const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& center) noexcept;
		LuaCamera& setOrtho6(const VarLuaTable<glm::uvec2>& size) noexcept;

		LuaCamera& setViewportPerspective1(float fovy) noexcept;
		LuaCamera& setViewportPerspective2(float fovy, float near, float far) noexcept;
		LuaCamera& setViewportOrtho1(const VarLuaTable<glm::vec2>& center) noexcept;
		LuaCamera& setViewportOrtho2(const VarLuaTable<glm::vec2>& center, float near, float far) noexcept;

		const std::string& getName() const noexcept;
		void setName(const std::string& name) noexcept;
		bool getEnabled() const noexcept;
		void setEnabled(bool enabled) noexcept;
		const glm::mat4& getProjectionMatrix() const noexcept;
		void setProjectionMatrix(const VarLuaTable<glm::mat4>& matrix) noexcept;
		
		void setViewport(std::optional<VarViewport> viewport) noexcept;
		std::optional<Viewport> getViewport() noexcept;
		Viewport getCurrentViewport() noexcept;

		OptionalRef<Transform>::std_t getTransform() noexcept;
		glm::mat4 getModelMatrix() const noexcept;
		glm::mat4 getModelInverse() const noexcept;
		RenderGraphDefinition& getRenderGraph() const noexcept;
		RenderChain& getRenderChain() const noexcept;

		Ray screenPointToRay1(const glm::vec2& point) noexcept;
		Ray screenPointToRay2(const VarLuaTable<glm::vec3>& point) noexcept;
		Ray viewportPointToRay(const VarLuaTable<glm::vec3>& point) noexcept;
		glm::vec3 worldToScreenPoint(const VarLuaTable<glm::vec3>& point) noexcept;
		glm::vec3 worldToViewportPoint(const VarLuaTable<glm::vec3>& point) noexcept;
		glm::vec3 screenToWorldPoint(const VarLuaTable<glm::vec3>& point) noexcept;
		glm::vec3 viewportToWorldPoint(const VarLuaTable<glm::vec3>& point) noexcept;
		glm::vec3 viewportToScreenPoint(const VarLuaTable<glm::vec3>& point) noexcept;
		glm::vec3 screenToViewportPoint(const VarLuaTable<glm::vec3>& point) noexcept;

		void setCullingMask(uint32_t mask) noexcept;
		uint32_t getCullingMask() const noexcept;
	};

	struct CullingMask;

	class LuaCullingMask final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static CullingMask& addEntityComponent(LuaEntity& entity, uint32_t value) noexcept;
		static OptionalRef<CullingMask>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const CullingMask& cam, LuaScene& scene) noexcept;
	};
}