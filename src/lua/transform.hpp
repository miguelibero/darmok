#pragma once

#include "glm.hpp"
#include <optional>
#include <sol/sol.hpp>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Transform;
	class LuaEntity;
	class LuaScene;

    class LuaTransform final
	{
	public:
		LuaTransform(Transform& transform) noexcept;

		const Transform& getReal() const;
		Transform& getReal();
		std::string to_string() const noexcept;

		std::optional<LuaTransform> getParent() noexcept;
		void setParent(std::optional<LuaTransform> parent) noexcept;

		const std::string& getName() const noexcept;
		void setName(const std::string& name) noexcept;

		const glm::vec3& getPosition() const noexcept;
		const glm::quat& getRotation() const noexcept;
		glm::vec3 getEulerAngles() const noexcept;
		glm::vec3 getForward() const noexcept;
		glm::vec3 getRight() const noexcept;
		glm::vec3 getUp() const noexcept;
		const glm::vec3& getScale() const noexcept;
		const glm::vec3& getPivot() const noexcept;
		const glm::mat4& getMatrix() const noexcept;
		const glm::mat4& getLocalToWorldMatrix() const noexcept;
		const glm::mat4& getWorldToLocalMatrix() const noexcept;
		glm::vec3 getWorldPosition() const noexcept;
		glm::vec3 worldToLocalPoint(const VarLuaTable<glm::vec3>& v) const noexcept;
		glm::vec3 localToWorldPoint(const VarLuaTable<glm::vec3>& v) const noexcept;
		void setPosition(const VarLuaTable<glm::vec3>& v) noexcept;
		void setRotation(const VarLuaTable<glm::quat>& v) noexcept;
		void setEulerAngles(const VarLuaTable<glm::vec3>& v) noexcept;
		void setForward(const VarLuaTable<glm::vec3>& v) noexcept;
		void setScale(const VarLuaVecTable<glm::vec3>& v) noexcept;
		void setPivot(const VarLuaTable<glm::vec3>& v) noexcept;
		void setMatrix(const VarLuaTable<glm::mat4>& v) noexcept;

		void rotate1(float x, float y, float z) noexcept;
		void rotate2(const VarLuaTable<glm::vec3>& v) noexcept;

		void lookDir1(const VarLuaTable<glm::vec3>& v) noexcept;
		void lookDir2(const VarLuaTable<glm::vec3>& v, const VarLuaTable<glm::vec3>& up) noexcept;
		void lookAt1(const VarLuaTable<glm::vec3>& v) noexcept;
		void lookAt2(const VarLuaTable<glm::vec3>& v, const VarLuaTable<glm::vec3>& up) noexcept;

		static void bind(sol::state_view& lua) noexcept;

	private:
		OptionalRef<Transform> _transform;

		static LuaTransform addEntityComponent1(LuaEntity& entity) noexcept;
		static LuaTransform addEntityComponent2(LuaEntity& entity, LuaTransform& parent) noexcept;
		static LuaTransform addEntityComponent3(LuaEntity& entity, LuaTransform& parent, const VarLuaTable<glm::vec3>& pos) noexcept;
		static LuaTransform addEntityComponent4(LuaEntity& entity, const VarLuaTable<glm::vec3>& pos) noexcept;
		static std::optional<LuaTransform> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};
}