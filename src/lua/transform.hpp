#pragma once

#include "lua.hpp"
#include "glm.hpp"

#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>

#include <optional>

namespace darmok
{
    class Transform;
	class LuaEntity;
	class Scene;

    class LuaTransform final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;

	private:
		static Transform& addEntityComponent1(LuaEntity& entity) noexcept;
		static Transform& addEntityComponent2(LuaEntity& entity, Transform& parent) noexcept;
		static Transform& addEntityComponent3(LuaEntity& entity, Transform& parent, const VarLuaTable<glm::vec3>& pos) noexcept;
		static Transform& addEntityComponent4(LuaEntity& entity, const VarLuaTable<glm::vec3>& pos) noexcept;
		static OptionalRef<Transform>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const Transform& trans, std::shared_ptr<Scene>& scene) noexcept;

		static OptionalRef<Transform>::std_t getParent(Transform& trans) noexcept;
		static void setParent(Transform& trans, OptionalRef<Transform>::std_t parent) noexcept;

		static glm::vec3 worldToLocalPoint(const Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept;
		static glm::vec3 localToWorldPoint(const Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept;
		static void setPosition(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept;
		static void setRotation(Transform& trans, const VarLuaTable<glm::quat>& v) noexcept;
		static void setEulerAngles(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept;
		static void setForward(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept;
		static void setScale(Transform& trans, const VarLuaVecTable<glm::vec3>& v) noexcept;
		static void setLocalMatrix(Transform& trans, const VarLuaTable<glm::mat4>& v) noexcept;
		static Transform& rotate1(Transform& trans, float x, float y, float z) noexcept;
		static Transform& rotate2(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept;
		static Transform& rotateAround(Transform& trans, const VarLuaTable<glm::vec3>& point, const VarLuaTable<glm::vec3>& axis, float angle) noexcept;
		static Transform& lookDir1(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept;
		static Transform& lookDir2(Transform& trans, const VarLuaTable<glm::vec3>& v, const VarLuaTable<glm::vec3>& up) noexcept;
		static Transform& lookAt1(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept;
		static Transform& lookAt2(Transform& trans, const VarLuaTable<glm::vec3>& v, const VarLuaTable<glm::vec3>& up) noexcept;
	};
}