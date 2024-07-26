#include "transform.hpp"
#include "scene.hpp"
#include <darmok/transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace darmok
{
	OptionalRef<Transform>::std_t LuaTransform::getParent(Transform& trans) noexcept
	{
		return trans.getParent();
	}

	void LuaTransform::setParent(Transform& trans, OptionalRef<Transform>::std_t parent) noexcept
	{
		trans.setParent(parent);
	}

	glm::vec3 LuaTransform::worldToLocalPoint(const Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		return trans.worldToLocalPoint(LuaGlm::tableGet(v));
	}

	glm::vec3 LuaTransform::localToWorldPoint(const Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		return trans.worldToLocalPoint(LuaGlm::tableGet(v));
	}

	void LuaTransform::setPosition(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		trans.setPosition(LuaGlm::tableGet(v));
	}

	void LuaTransform::setEulerAngles(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		trans.setEulerAngles(LuaGlm::tableGet(v));
	}

	void LuaTransform::setRotation(Transform& trans, const VarLuaTable<glm::quat>& v) noexcept
	{
		trans.setRotation(LuaGlm::tableGet(v));
	}

	void LuaTransform::setForward(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		trans.setForward(LuaGlm::tableGet(v));
	}

	void LuaTransform::setScale(Transform& trans, const VarLuaVecTable<glm::vec3>& v) noexcept
	{
		trans.setScale(LuaGlm::tableGet(v));
	}

	void LuaTransform::setPivot(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		trans.setPivot(LuaGlm::tableGet(v));
	}

	void LuaTransform::lookDir1(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		trans.lookDir(LuaGlm::tableGet(v));
	}

	void LuaTransform::lookDir2(Transform& trans, const VarLuaTable<glm::vec3>& v, const VarLuaTable<glm::vec3>& up) noexcept
	{
		trans.lookDir(LuaGlm::tableGet(v), LuaGlm::tableGet(up));
	}

	void LuaTransform::lookAt1(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		trans.lookAt(LuaGlm::tableGet(v));
	}

	void LuaTransform::lookAt2(Transform& trans, const VarLuaTable<glm::vec3>& v, const VarLuaTable<glm::vec3>& up) noexcept
	{
		trans.lookAt(LuaGlm::tableGet(v), LuaGlm::tableGet(up));
	}

	void LuaTransform::setMatrix(Transform& trans, const VarLuaTable<glm::mat4>& v) noexcept
	{
		trans.setLocalMatrix(LuaGlm::tableGet(v));
	}

	void LuaTransform::rotate1(Transform& trans, float x, float y, float z) noexcept
	{
		trans.rotate(glm::vec3(x, y, z));
	}

	void LuaTransform::rotate2(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		trans.rotate(LuaGlm::tableGet(v));
	}

	Transform& LuaTransform::addEntityComponent1(LuaEntity& entity) noexcept
	{
		return entity.addComponent<Transform>();
	}

	Transform& LuaTransform::addEntityComponent2(LuaEntity& entity, Transform& parent) noexcept
	{
		auto& trans = entity.addComponent<Transform>();
		trans.setParent(parent);
		return trans;
	}

	Transform& LuaTransform::addEntityComponent3(LuaEntity& entity, Transform& parent, const VarLuaTable<glm::vec3>& pos) noexcept
	{
		return entity.addComponent<Transform>(parent, LuaGlm::tableGet(pos));
	}

	Transform& LuaTransform::addEntityComponent4(LuaEntity& entity, const VarLuaTable<glm::vec3>& pos) noexcept
	{
		return entity.addComponent<Transform>(LuaGlm::tableGet(pos));
	}

	OptionalRef<Transform>::std_t LuaTransform::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<Transform>();
	}

	std::optional<LuaEntity> LuaTransform::getEntity(const Transform& trans, LuaScene& scene) noexcept
	{
		return scene.getEntity(trans);
	}

	void LuaTransform::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Transform>("Transform", sol::no_constructor,
			"type_id", &entt::type_hash<Transform>::value,
			"add_entity_component", sol::overload(
				&LuaTransform::addEntityComponent1,
				&LuaTransform::addEntityComponent2,
				&LuaTransform::addEntityComponent3,
				&LuaTransform::addEntityComponent4
			),
			"get_entity_component", &LuaTransform::getEntityComponent,
			"get_entity", &LuaTransform::getEntity,

			"name", sol::property(&Transform::getName, &Transform::setName),
			"position", sol::property(&Transform::getPosition, &LuaTransform::setPosition),
			"world_position", sol::property(&Transform::getWorldPosition),
			"rotation", sol::property(&Transform::getRotation, &LuaTransform::setRotation),
			"euler_angles", sol::property(&Transform::getEulerAngles, &LuaTransform::setEulerAngles),
			"forward", sol::property(&Transform::getForward),
			"right", sol::property(&Transform::getRight),
			"up", sol::property(&Transform::getUp),
			"scale", sol::property(&Transform::getScale, &LuaTransform::setScale),
			"pivot", sol::property(&Transform::getPivot, &LuaTransform::setPivot),
			"matrix", sol::property(&Transform::getWorldMatrix, &LuaTransform::setMatrix),
			"local_to_world_matrix", sol::property(&Transform::getWorldMatrix),
			"world_to_local_matrix", sol::property(&Transform::getWorldInverse),
			"parent", sol::property(&LuaTransform::getParent, &LuaTransform::setParent),
			"rotate", sol::overload(&LuaTransform::rotate1, &LuaTransform::rotate2),
			"look_dir", sol::overload(&LuaTransform::lookDir1, &LuaTransform::lookDir2),
			"look_at", sol::overload(&LuaTransform::lookAt1, &LuaTransform::lookAt2),
			"world_to_local_point", &LuaTransform::worldToLocalPoint,
			"local_to_world_point", &LuaTransform::localToWorldPoint,
			sol::meta_function::to_string, &Transform::toString
		);
	}
}