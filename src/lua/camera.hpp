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
	class LuaProgram;
	class LuaTexture;
	class LuaEntity;
	class LuaScene;
	class LuaTransform;

	enum class LuaNativeCameraComponentType
	{
		PhongLighting,
		SkeletalAnimation
	};

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
		LuaCamera& setOrtho1(const VarViewport& vp, float near, float far) noexcept;
		LuaCamera& setOrtho2(const VarViewport& vp) noexcept;
		LuaCamera& setOrtho3(const VarLuaTable<glm::uvec2>& size, float near, float fare) noexcept;
		LuaCamera& setOrtho4(const VarLuaTable<glm::uvec2>& size) noexcept;
		LuaCamera& addNativeComponent(LuaNativeCameraComponentType type) noexcept;
		LuaCamera& setForwardRenderer() noexcept;

		const glm::mat4& getProjectionMatrix() const noexcept;
		void setProjectionMatrix(const VarLuaTable<glm::mat4>& matrix) noexcept;

		LuaCamera& setTargetTextures(const sol::table& textures) noexcept;
		std::vector<LuaTexture> getTargetTextures() noexcept;

		std::optional<Viewport> getViewport() const noexcept;
		LuaCamera& setViewport(std::optional<VarViewport> viewport) noexcept;
		Viewport getCurrentViewport() const noexcept;

		std::optional<LuaTransform> getTransform() const noexcept;
		glm::mat4 getModelMatrix() const noexcept;

		Ray screenPointToRay(const VarLuaTable<glm::vec3>& point) const noexcept;
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