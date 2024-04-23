#pragma once

#include <optional>
#include <darmok/optional_ref.hpp>
#include "sol.hpp"
#include "math.hpp"
#include "scene_fwd.hpp"

namespace darmok
{
    class Camera;
	class LuaProgram;
	class LuaTexture;
	class Ray;

	class LuaCamera final
	{
	public:
		using native_t = Camera;
		const static LuaNativeComponentType native_type = LuaNativeComponentType::Camera;

		LuaCamera(Camera& camera) noexcept;

		const Camera& getReal() const;
		Camera& getReal();

		LuaCamera& setProjection1(float fovy, float aspect, const VarVec2& range) noexcept;
		LuaCamera& setProjection2(float fovy, float aspect, float near) noexcept;
		LuaCamera& setWindowProjection1(float fovy, const VarVec2& range) noexcept;
		LuaCamera& setWindowProjection2(float fovy, float near) noexcept;
		LuaCamera& setWindowProjection3(float fovy) noexcept;
		LuaCamera& setOrtho1(const VarVec4& edges, const VarVec2& range, float offset) noexcept;
		LuaCamera& setOrtho2(const VarVec4& edges, const VarVec2& range) noexcept;
		LuaCamera& setOrtho3(const VarVec4& edges) noexcept;
		LuaCamera& setWindowOrtho1(const VarVec2& range, float offset) noexcept;
		LuaCamera& setWindowOrtho2(const VarVec2& range) noexcept;
		LuaCamera& setWindowOrtho3() noexcept;
		LuaCamera& setTargetTexture(const LuaTexture& texture) noexcept;
		LuaCamera& setForwardPhongRenderer(const LuaProgram& program) noexcept;

		std::optional<LuaTexture> getTargetTexture() noexcept;
		const glm::mat4& getMatrix() const noexcept;
		void setMatrix(const glm::mat4& matrix) noexcept;
		std::optional<Ray> screenPointToRay(const VarVec2& point) const noexcept;

		static void configure(sol::state_view& lua) noexcept;

	private:
		OptionalRef<Camera> _camera;
	};

}