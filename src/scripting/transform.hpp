#pragma once

#include "scene_fwd.hpp"
#include <variant>
#include <optional>
#include "sol.hpp"
#include "math.hpp"
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Transform;

    class LuaTransform final
	{
	public:
		using native_t = Transform;
		const static LuaNativeComponentType native_type = LuaNativeComponentType::Transform;

		LuaTransform(Transform& transform) noexcept;
		std::optional<LuaTransform> getParent() noexcept;
		void setParent(std::optional<LuaTransform> parent) noexcept;

		const glm::vec3& getPosition() const noexcept;
		const glm::quat& getRotation() const noexcept;
		glm::vec3 getEulerAngles() const noexcept;
		glm::vec3 getForward() const noexcept;
		glm::vec3 getRight() const noexcept;
		glm::vec3 getUp() const noexcept;
		const glm::vec3& getScale() const noexcept;
		const glm::vec3& getPivot() const noexcept;
		const glm::mat4& getMatrix() const noexcept;
		const glm::mat4& getInverse() const noexcept;

		void setPosition(const VarVec3& v) noexcept;
		void setRotation(const VarQuat& v) noexcept;
		void setEulerAngles(const VarVec3& v) noexcept;
		void setForward(const VarVec3& v) noexcept;
		void setScale(const VarVec3& v) noexcept;
		void setPivot(const VarVec3& v) noexcept;
		void setMatrix(const glm::mat4& v) noexcept;

		void lookDir1(const VarVec3& v) noexcept;
		void lookDir2(const VarVec3& v, const VarVec3& up) noexcept;
		void lookAt1(const VarVec3& v) noexcept;
		void lookAt2(const VarVec3& v, const VarVec3& up) noexcept;

		static void configure(sol::state_view& lua) noexcept;

	private:
		OptionalRef<Transform> _transform;
	};
}