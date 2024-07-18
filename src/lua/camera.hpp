#pragma once

#include <optional>
#include <vector>
#include <darmok/optional_ref.hpp>
#include <darmok/shape.hpp>
#include <sol/sol.hpp>
#include "glm.hpp"
#include "viewport.hpp"

namespace darmok
{
    class Camera;
	class Program;
	class Texture;
	class LuaEntity;
	class LuaScene;
	class LuaTransform;

	class LuaCamera final
	{
	public:
		LuaCamera(Camera& camera) noexcept;

		const Camera& getReal() const;
		Camera& getReal();

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

		const glm::mat4& getProjectionMatrix() const noexcept;
		void setProjectionMatrix(const VarLuaTable<glm::mat4>& matrix) noexcept;

		void setTargetTextures(const std::vector<std::shared_ptr<Texture>>& textures) noexcept;
		std::vector<std::shared_ptr<Texture>> getTargetTextures() noexcept;

		std::optional<Viewport> getViewport() const noexcept;
		void setViewport(std::optional<VarViewport> viewport) noexcept;
		Viewport getCurrentViewport() const noexcept;

		std::optional<LuaTransform> getTransform() const noexcept;
		glm::mat4 getModelMatrix() const noexcept;

		bool getEnabled() const noexcept;
		void setEnabled(bool enabled) noexcept;

		Ray screenPointToRay1(const glm::vec2& point) const noexcept;
		Ray screenPointToRay2(const VarLuaTable<glm::vec3>& point) const noexcept;
		Ray viewportPointToRay(const VarLuaTable<glm::vec3>& point) const noexcept;
		glm::vec3 worldToScreenPoint(const VarLuaTable<glm::vec3>& point) const noexcept;
		glm::vec3 worldToViewportPoint(const VarLuaTable<glm::vec3>& point) const noexcept;
		glm::vec3 screenToWorldPoint(const VarLuaTable<glm::vec3>& point) const noexcept;
		glm::vec3 viewportToWorldPoint(const VarLuaTable<glm::vec3>& point) const noexcept;
		glm::vec3 viewportToScreenPoint(const VarLuaTable<glm::vec3>& point) const noexcept;
		glm::vec3 screenToViewportPoint(const VarLuaTable<glm::vec3>& point) const noexcept;

		static void bind(sol::state_view& lua) noexcept;

	private:
		OptionalRef<Camera> _camera;

		static LuaCamera addEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaCamera> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};
}