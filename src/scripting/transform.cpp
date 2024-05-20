#include "transform.hpp"
#include "scene.hpp"
#include <darmok/transform.hpp>
#include <glm/gtc/quaternion.hpp>

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

	void LuaTransform::setPosition(const glm::vec3& v) noexcept
	{
		_transform->setPosition(v);
	}

	void LuaTransform::setEulerAngles(const glm::vec3& v) noexcept
	{
		_transform->setEulerAngles(v);
	}

	void LuaTransform::setRotation(const glm::quat& v) noexcept
	{
		_transform->setRotation(v);
	}

	void LuaTransform::setForward(const glm::vec3& v) noexcept
	{
		_transform->setForward(v);
	}

	void LuaTransform::setScale(const glm::vec3& v) noexcept
	{
		_transform->setScale(v);
	}

	void LuaTransform::setPivot(const glm::vec3& v) noexcept
	{
		_transform->setPivot(v);
	}

	void LuaTransform::lookDir1(const glm::vec3& v) noexcept
	{
		_transform->lookDir(v);
	}

	void LuaTransform::lookDir2(const glm::vec3& v, const glm::vec3& up) noexcept
	{
		_transform->lookDir(v, up);
	}

	void LuaTransform::lookAt1(const glm::vec3& v) noexcept
	{
		_transform->lookAt(v);
	}

	void LuaTransform::lookAt2(const glm::vec3& v, const glm::vec3& up) noexcept
	{
		_transform->lookAt(v, up);
	}

	void LuaTransform::setMatrix(const glm::mat4& v) noexcept
	{
		_transform->setLocalMatrix(v);
	}

	void LuaTransform::rotate1(float x, float y, float z) noexcept
	{
		rotate2(glm::vec3(x, y, z));
	}

	void LuaTransform::rotate2(const glm::vec3& v) noexcept
	{
		_transform->rotate(v);
	}

	LuaTransform LuaTransform::addEntityComponent1(LuaEntity& entity) noexcept
	{
		return entity.addComponent<Transform>();
	}

	LuaTransform LuaTransform::addEntityComponent2(LuaEntity& entity, const glm::vec3& pos) noexcept
	{
		return entity.addComponent<Transform>(pos);
	}

	LuaTransform LuaTransform::addEntityComponent3(LuaEntity& entity, LuaTransform& parent) noexcept
	{
		auto& trans = entity.addComponent<Transform>();
		trans.setParent(parent.getReal());
		return trans;
	}

	LuaTransform LuaTransform::addEntityComponent4(LuaEntity& entity, LuaTransform& parent, const glm::vec3& pos) noexcept
	{
		return entity.addComponent<Transform>(parent.getReal(), pos);
	}

	std::optional<LuaTransform> LuaTransform::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<Transform, LuaTransform>();
	}

	std::optional<LuaEntity> LuaTransform::getEntity(LuaScene& scene) noexcept
	{
		return scene.getEntity(_transform.value());
	}

	void LuaTransform::bind(sol::state_view& lua) noexcept
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
}