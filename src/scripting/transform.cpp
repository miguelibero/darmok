#include "transform.hpp"
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
		return _transform->getMatrix();
	}

	const glm::mat4& LuaTransform::getInverse() const noexcept
	{
		return _transform->getInverse();
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
		_transform->setMatrix(v);
	}

	void LuaTransform::rotate1(float x, float y, float z) noexcept
	{
		rotate2(glm::vec3(x, y, z));
	}

	void LuaTransform::rotate2(const VarVec3& v) noexcept
	{
		_transform->rotate(LuaMath::tableToGlm(v));
	}

	void LuaTransform::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaTransform>("Transform", sol::constructors<>(),
			"position", sol::property(&LuaTransform::getPosition, &LuaTransform::setPosition),
			"rotation", sol::property(&LuaTransform::getRotation, &LuaTransform::setRotation),
			"euler_angles", sol::property(&LuaTransform::getEulerAngles, &LuaTransform::setEulerAngles),
			"forward", sol::property(&LuaTransform::getForward),
			"right", sol::property(&LuaTransform::getRight),
			"up", sol::property(&LuaTransform::getUp),
			"scale", sol::property(&LuaTransform::getScale, &LuaTransform::setScale),
			"pivot", sol::property(&LuaTransform::getPivot, &LuaTransform::setPivot),
			"matrix", sol::property(&LuaTransform::getMatrix, &LuaTransform::setMatrix),
			"inverse", sol::property(&LuaTransform::getInverse),
			"parent", sol::property(&LuaTransform::getParent, &LuaTransform::setParent),
			"rotate", sol::overload(&LuaTransform::rotate1, &LuaTransform::rotate2),
			"look_dir", sol::overload(&LuaTransform::lookDir1, &LuaTransform::lookDir2),
			"look_at", sol::overload(&LuaTransform::lookAt1, &LuaTransform::lookAt2)
		);
	}

}