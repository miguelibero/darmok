#include "transform.hpp"
#include "scene.hpp"
#include <darmok/transform.hpp>

namespace darmok
{
    LuaTransform::LuaTransform(Transform& transform) noexcept
		: _transform(transform)
	{
	}

	const Transform& LuaTransform::getReal() const
	{
		return _transform.value();
	}

	Transform& LuaTransform::getReal()
	{
		return _transform.value();
	}

	std::string LuaTransform::to_string() const noexcept
	{
		return std::string("Transform(") + _transform->to_string() + ")";
	}

	std::optional<LuaTransform> LuaTransform::getParent() noexcept
	{
		auto parent = _transform->getParent();
		if (parent)
		{
			return LuaTransform(parent.value());
		}
		return std::nullopt;
	}

	void LuaTransform::setParent(std::optional<LuaTransform> parent) noexcept
	{
		OptionalRef<Transform> p = nullptr;
		if (parent.has_value())
		{
			p = parent.value()._transform;
		}
		_transform->setParent(p);
	}

	const glm::vec3& LuaTransform::getPosition() const noexcept
	{
		return _transform->getPosition();
	}

	const glm::quat& LuaTransform::getRotation() const noexcept
	{
		return _transform->getRotation();
	}

	glm::vec3 LuaTransform::getEulerAngles() const noexcept
	{
		return glm::degrees(glm::eulerAngles(getRotation()));
	}

	glm::vec3 LuaTransform::getForward() const noexcept
	{
		static const glm::vec3 dir(0, 0, 1);
		return _transform->getRotation() * dir;
	}

	glm::vec3 LuaTransform::getRight() const noexcept
	{
		static const glm::vec3 dir(1, 0, 0);
		return _transform->getRotation() * dir;
	}

	glm::vec3 LuaTransform::getUp() const noexcept
	{
		static const glm::vec3 dir(0, 1, 0);
		return _transform->getRotation() * dir;
	}

	const glm::vec3& LuaTransform::getScale() const noexcept
	{
		return _transform->getScale();
	}

	const glm::vec3& LuaTransform::getPivot() const noexcept
	{
		return _transform->getPivot();
	}
	
	const glm::mat4& LuaTransform::getMatrix() const noexcept
	{
		return _transform->getLocalMatrix();
	}

	const glm::mat4& LuaTransform::getLocalToWorldMatrix() const noexcept
	{
		return _transform->getWorldMatrix();
	}

	const glm::mat4& LuaTransform::getWorldToLocalMatrix() const noexcept
	{
		return _transform->getWorldInverse();
	}

	void LuaTransform::setPosition(const VarVec3& v) noexcept
	{
		_transform->setPosition(LuaMath::tableToGlm(v));
	}

	void LuaTransform::setEulerAngles(const VarVec3& v) noexcept
	{
		_transform->setEulerAngles(LuaMath::tableToGlm(v));
	}

	void LuaTransform::setRotation(const VarQuat& v) noexcept
	{
		_transform->setRotation(LuaMath::tableToGlm(v));
	}

	void LuaTransform::setForward(const VarVec3& v) noexcept
	{
		_transform->setForward(LuaMath::tableToGlm(v));
	}

	void LuaTransform::setScale(const VarVec3& v) noexcept
	{
		_transform->setScale(LuaMath::tableToGlm(v));
	}

	void LuaTransform::setPivot(const VarVec3& v) noexcept
	{
		_transform->setPivot(LuaMath::tableToGlm(v));
	}

	void LuaTransform::lookDir1(const VarVec3& v) noexcept
	{
		_transform->lookDir(LuaMath::tableToGlm(v));
	}

	void LuaTransform::lookDir2(const VarVec3& v, const VarVec3& up) noexcept
	{
		_transform->lookDir(LuaMath::tableToGlm(v), LuaMath::tableToGlm(up));
	}

	void LuaTransform::lookAt1(const VarVec3& v) noexcept
	{
		_transform->lookAt(LuaMath::tableToGlm(v));
	}

	void LuaTransform::lookAt2(const VarVec3& v, const VarVec3& up) noexcept
	{
		_transform->lookAt(LuaMath::tableToGlm(v), LuaMath::tableToGlm(up));
	}

	void LuaTransform::setMatrix(const glm::mat4& v) noexcept
	{
		_transform->setLocalMatrix(v);
	}

	void LuaTransform::rotate1(float x, float y, float z) noexcept
	{
		rotate2(glm::vec3(x, y, z));
	}

	void LuaTransform::rotate2(const VarVec3& v) noexcept
	{
		_transform->rotate(LuaMath::tableToGlm(v));
	}

	LuaTransform LuaTransform::addEntityComponent1(LuaEntity& entity) noexcept
	{
		return entity.addComponent<Transform>();
	}

	LuaTransform LuaTransform::addEntityComponent2(LuaEntity& entity, const VarVec3& pos) noexcept
	{
		return entity.addComponent<Transform>(LuaMath::tableToGlm(pos));
	}

	LuaTransform LuaTransform::addEntityComponent3(LuaEntity& entity, LuaTransform& parent) noexcept
	{
		auto& trans = entity.addComponent<Transform>();
		trans.setParent(parent.getReal());
		return trans;
	}

	LuaTransform LuaTransform::addEntityComponent4(LuaEntity& entity, LuaTransform& parent, const VarVec3& pos) noexcept
	{
		return entity.addComponent<Transform>(parent.getReal(), LuaMath::tableToGlm(pos));
	}

	std::optional<LuaTransform> LuaTransform::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<Transform, LuaTransform>();
	}

	std::optional<LuaEntity> LuaTransform::getEntity(LuaScene& scene) noexcept
	{
		return scene.getEntity(_transform.value());
	}

	void LuaTransform::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaTransform>("Transform",
			sol::constructors<>(),
			"type_id", &entt::type_hash<Transform>::value,
			"add_entity_component", sol::overload(
				&LuaTransform::addEntityComponent1,
				&LuaTransform::addEntityComponent2,
				&LuaTransform::addEntityComponent3,
				&LuaTransform::addEntityComponent4
			),
			"get_entity_component", &LuaTransform::getEntityComponent,
			"get_entity", &LuaTransform::getEntity,

			"position", sol::property(&LuaTransform::getPosition, &LuaTransform::setPosition),
			"rotation", sol::property(&LuaTransform::getRotation, &LuaTransform::setRotation),
			"euler_angles", sol::property(&LuaTransform::getEulerAngles, &LuaTransform::setEulerAngles),
			"forward", sol::property(&LuaTransform::getForward),
			"right", sol::property(&LuaTransform::getRight),
			"up", sol::property(&LuaTransform::getUp),
			"scale", sol::property(&LuaTransform::getScale, &LuaTransform::setScale),
			"pivot", sol::property(&LuaTransform::getPivot, &LuaTransform::setPivot),
			"matrix", sol::property(&LuaTransform::getMatrix, &LuaTransform::setMatrix),
			"local_to_world_matrix", sol::property(&LuaTransform::getLocalToWorldMatrix),
			"world_to_local_matrix", sol::property(&LuaTransform::getWorldToLocalMatrix),
			"parent", sol::property(&LuaTransform::getParent, &LuaTransform::setParent),
			"rotate", sol::overload(&LuaTransform::rotate1, &LuaTransform::rotate2),
			"look_dir", sol::overload(&LuaTransform::lookDir1, &LuaTransform::lookDir2),
			"look_at", sol::overload(&LuaTransform::lookAt1, &LuaTransform::lookAt2)
		);
	}

	namespace lua
	{
		namespace transform
		{
			std::reference_wrapper<Transform> addEntityComponent1(LuaEntity& entity) noexcept
			{
				return entity.addComponent<Transform>();
			}

			std::reference_wrapper<Transform> addEntityComponent2(LuaEntity& entity, const VarVec3& pos) noexcept
			{
				return entity.addComponent<Transform>(LuaMath::tableToGlm(pos));
			}

			std::reference_wrapper<Transform> addEntityComponent3(LuaEntity& entity, OptionalRef<Transform> parent) noexcept
			{
				auto& trans = entity.addComponent<Transform>();
				trans.setParent(parent);
				return trans;
			}

			std::reference_wrapper<Transform> addEntityComponent4(LuaEntity& entity, OptionalRef<Transform> parent, const VarVec3& pos) noexcept
			{
				return entity.addComponent<Transform>(parent, LuaMath::tableToGlm(pos));
			}

			OptionalRef<Transform>::std_t getEntityComponent(LuaEntity& entity) noexcept
			{
				return entity.getScene().getReal()->getComponent<Transform>(entity.getReal());
			}

			std::optional<LuaEntity> getEntity(Transform& self, LuaScene& scene) noexcept
			{
				return scene.getEntity(self);
			}

			void rotate2(Transform& self, const VarVec3& v) noexcept
			{
				self.rotate(LuaMath::tableToGlm(v));
			}

			void rotate1(Transform& self, float x, float y, float z) noexcept
			{
				rotate2(self, glm::vec3(x, y, z));
			}
		}

		void bindTransform(sol::state_view& lua) noexcept
		{
			lua.new_usertype<Transform>("Transform",
				sol::no_constructor,
				"type_id", &entt::type_hash<Transform>::value /*,
				"add_entity_component", sol::overload(
					&transform::addEntityComponent1,
					&transform::addEntityComponent2,
					&transform::addEntityComponent3,
					&transform::addEntityComponent4
				),
				"get_entity_component", &transform::getEntityComponent,
				"get_entity", &transform::getEntity,

				"position", sol::property(&Transform::getPosition, &Transform::setPosition),
				"rotation", sol::property(&Transform::getRotation, &Transform::setRotation),
				"euler_angles", sol::property(&Transform::getEulerAngles, &Transform::setEulerAngles),
				"forward", sol::property(&Transform::getForward),
				"right", sol::property(&Transform::getRight),
				"up", sol::property(&Transform::getUp),
				"scale", sol::property(&Transform::getScale, &Transform::setScale),
				"pivot", sol::property(&Transform::getPivot, &Transform::setPivot),
				"local_matrix", sol::property(&Transform::getLocalMatrix, &Transform::setLocalMatrix),
				"local_to_world_matrix", sol::property(&Transform::getWorldMatrix),
				"world_to_local_matrix", sol::property(&Transform::getWorldInverse),
				"parent", sol::property(sol::resolve<OptionalRef<Transform>()>(&Transform::getParent), &Transform::setParent),
				"rotate", sol::overload(&transform::rotate1, &transform::rotate2),
				"look_dir", sol::overload(&Transform::lookDir, &Transform::lookDir),
				"look_at", sol::overload(&Transform::lookAt, &Transform::lookAt)
				*/
			);
		}
	}
}