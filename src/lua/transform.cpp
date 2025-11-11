#include "lua/transform.hpp"
#include "lua/scene.hpp"
#include "lua/protobuf.hpp"
#include "lua/scene_serialize.hpp"

#include <darmok/protobuf/scene.pb.h>
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

	void LuaTransform::setLocalMatrix(Transform& trans, const VarLuaTable<glm::mat4>& v) noexcept
	{
		trans.setLocalMatrix(LuaGlm::tableGet(v));
	}

	Transform& LuaTransform::lookDir1(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		return trans.lookDir(LuaGlm::tableGet(v));
	}

	Transform& LuaTransform::lookDir2(Transform& trans, const VarLuaTable<glm::vec3>& v, const VarLuaTable<glm::vec3>& up) noexcept
	{
		return trans.lookDir(LuaGlm::tableGet(v), LuaGlm::tableGet(up));
	}

	Transform& LuaTransform::lookAt1(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		return trans.lookAt(LuaGlm::tableGet(v));
	}

	Transform& LuaTransform::lookAt2(Transform& trans, const VarLuaTable<glm::vec3>& v, const VarLuaTable<glm::vec3>& up) noexcept
	{
		return trans.lookAt(LuaGlm::tableGet(v), LuaGlm::tableGet(up));
	}

	Transform& LuaTransform::rotate1(Transform& trans, float x, float y, float z) noexcept
	{
		return trans.rotate(glm::vec3(x, y, z));
	}

	Transform& LuaTransform::rotate2(Transform& trans, const VarLuaTable<glm::vec3>& v) noexcept
	{
		return trans.rotate(LuaGlm::tableGet(v));
	}

	Transform& LuaTransform::rotateAround(Transform& trans, const VarLuaTable<glm::vec3>& point, const VarLuaTable<glm::vec3>& axis, float angle) noexcept
	{
		return trans.rotateAround(LuaGlm::tableGet(point), LuaGlm::tableGet(axis), angle);
	}

	Transform& LuaTransform::addEntityComponent1(LuaEntity& entity)
	{
		return entity.addComponent<Transform>();
	}

	Transform& LuaTransform::addEntityComponent2(LuaEntity& entity, const VarLuaTable<glm::vec3>& pos)
	{
		return entity.addComponent<Transform>(LuaGlm::tableGet(pos));
	}

	OptionalRef<Transform>::std_t LuaTransform::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<Transform>();
	}

	std::optional<LuaEntity> LuaTransform::getEntity(const Transform& trans, std::shared_ptr<Scene>& scene) noexcept
	{
		return LuaScene::getEntity(scene, trans);
	}

	void LuaTransform::bind(sol::state_view& lua) noexcept
	{
		auto def = LuaUtils::newProtobuf<Transform::Definition>(lua, "TransformDefinition")
			.protobufProperty<protobuf::Vec3>("position")
			.protobufProperty<protobuf::Quat>("rotation")
			.protobufProperty<protobuf::Vec3>("scale");
		def.userType["get_entity_component"] = [](LuaEntityDefinition& entity)
		{
			return entity.getComponent<Transform::Definition>();
		};
		def.userType["get_entity"] = [](const Transform::Definition& transform, const LuaSceneDefinition& scene)
		{
			return scene.getEntity(transform);
		};

		lua.new_usertype<Transform>("Transform", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<Transform>::value),
			"add_entity_component", sol::overload(
				&LuaTransform::addEntityComponent1,
				&LuaTransform::addEntityComponent2
			),
			"get_entity_component", &LuaTransform::getEntityComponent,
			"get_entity", &LuaTransform::getEntity,
			"reset", &Transform::reset,

			"name", sol::property(&Transform::getName, &Transform::setName),
			"position", sol::property(&Transform::getPosition, &LuaTransform::setPosition),
			"world_position", sol::property(&Transform::getWorldPosition),
			"world_rotation", sol::property(&Transform::getWorldRotation),
			"rotation", sol::property(&Transform::getRotation, &LuaTransform::setRotation),
			"euler_angles", sol::property(&Transform::getEulerAngles, &LuaTransform::setEulerAngles),
			"forward", sol::property(&Transform::getForward),
			"right", sol::property(&Transform::getRight),
			"up", sol::property(&Transform::getUp),
			"scale", sol::property(&Transform::getScale, &LuaTransform::setScale),
			"matrix", sol::property(&Transform::getLocalMatrix, &LuaTransform::setLocalMatrix),
			"world_matrix", sol::property(&Transform::getWorldMatrix),
			"world_inverse", sol::property(&Transform::getWorldInverse),
			"local_to_world_matrix", sol::property(&Transform::getWorldMatrix),
			"world_to_local_matrix", sol::property(&Transform::getWorldInverse),
			"parent", sol::property(&LuaTransform::getParent, &LuaTransform::setParent),
			"rotate", sol::overload(&LuaTransform::rotate1, &LuaTransform::rotate2),
			"rotate_around", &LuaTransform::rotateAround,
			"look_dir", sol::overload(&LuaTransform::lookDir1, &LuaTransform::lookDir2),
			"look_at", sol::overload(&LuaTransform::lookAt1, &LuaTransform::lookAt2),
			"world_to_local_point", &LuaTransform::worldToLocalPoint,
			"local_to_world_point", &LuaTransform::localToWorldPoint,
			sol::meta_function::to_string, &Transform::toString
		);
	}
}