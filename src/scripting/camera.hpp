#pragma once

#include <optional>
#include <vector>
#include <darmok/optional_ref.hpp>
#include <darmok/shape.hpp>
#include <sol/sol.hpp>
#include <glm/glm.hpp>

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

		LuaCamera& setProjection1(float fovy, float aspect, const glm::vec2& range) noexcept;
		LuaCamera& setProjection2(float fovy, float aspect, float near) noexcept;
		LuaCamera& setProjection3(float fovy, const glm::uvec2& size, const glm::vec2& range) noexcept;
		LuaCamera& setProjection4(float fovy, const glm::uvec2& size, float near) noexcept;
		LuaCamera& setOrtho1(const glm::vec4& edges, const glm::vec2& range, float offset) noexcept;
		LuaCamera& setOrtho2(const glm::vec4& edges, const glm::vec2& range) noexcept;
		LuaCamera& setOrtho3(const glm::vec4& edges) noexcept;
		LuaCamera& setOrtho4(const glm::uvec2& size, const glm::vec2& range, float offset) noexcept;
		LuaCamera& setOrtho5(const glm::uvec2& size, const glm::vec2& range) noexcept;
		LuaCamera& setOrtho6(const glm::uvec2& size) noexcept;
		LuaCamera& setTargetTextures(const sol::table& textures) noexcept;
		LuaCamera& addNativeComponent(LuaNativeCameraComponentType type) noexcept;
		LuaCamera& setForwardRenderer(const LuaProgram& program) noexcept;

		std::vector<LuaTexture> getTargetTextures() noexcept;
		const glm::mat4& getMatrix() const noexcept;
		void setMatrix(const glm::mat4& matrix) noexcept;
		std::optional<Ray> screenPointToRay(const glm::vec2& point) const noexcept;

		static void bind(sol::state_view& lua) noexcept;

	private:
		OptionalRef<Camera> _camera;

		static LuaCamera addEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaCamera> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};

}