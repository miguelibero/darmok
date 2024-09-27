#include "character.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include "physics3d.hpp"
#include <darmok/character.hpp>

namespace darmok::physics3d
{
	LuaCharacterController::LuaCharacterController(CharacterController& ctrl) noexcept
		: _ctrl(ctrl)
	{
		ctrl.setDelegate(*this);
	}

	LuaCharacterController::~LuaCharacterController() noexcept
	{
		_ctrl.setDelegate(nullptr);
	}

	bool LuaCharacterController::isGrounded() const noexcept
	{
		return _ctrl.isGrounded();
	}

	GroundState LuaCharacterController::getGroundState() const noexcept
	{
		return _ctrl.getGroundState();
	}

	void LuaCharacterController::setLinearVelocity(const VarLuaTable<glm::vec3>& velocity)
	{
		_ctrl.setLinearVelocity(LuaGlm::tableGet(velocity));
	}

	glm::vec3 LuaCharacterController::getLinearVelocity() const noexcept
	{
		return _ctrl.getLinearVelocity();
	}

	void LuaCharacterController::setPosition(const VarLuaTable<glm::vec3>& pos) noexcept
	{
		_ctrl.setPosition(LuaGlm::tableGet(pos));
	}

	glm::vec3 LuaCharacterController::getPosition() const noexcept
	{
		return _ctrl.getPosition();
	}

	void LuaCharacterController::setRotation(const VarLuaTable<glm::quat>& rot) noexcept
	{
		_ctrl.setRotation(LuaGlm::tableGet(rot));
	}

	glm::quat LuaCharacterController::getRotation() const noexcept
	{
		return _ctrl.getRotation();
	}

	glm::vec3 LuaCharacterController::getGroundNormal() const noexcept
	{
		return _ctrl.getGroundNormal();
	}

	glm::vec3 LuaCharacterController::getGroundPosition() const noexcept
	{
		return _ctrl.getGroundPosition();
	}

	glm::vec3 LuaCharacterController::getGroundVelocity() const noexcept
	{
		return _ctrl.getGroundVelocity();
	}

	LuaCharacterController& LuaCharacterController::addEntityComponent1(LuaEntity& entity, const Config& config) noexcept
	{
		return entity.addWrapperComponent<LuaCharacterController, CharacterController>(config);
	}

	LuaCharacterController& LuaCharacterController::addEntityComponent2(LuaEntity& entity, const Shape& shape) noexcept
	{
		return entity.addWrapperComponent<LuaCharacterController, CharacterController>(shape);
	}

	OptionalRef<LuaCharacterController>::std_t LuaCharacterController::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<LuaCharacterController>();
	}

	std::optional<LuaEntity> LuaCharacterController::getEntity(LuaScene& scene) noexcept
	{
		return scene.getEntity(_ctrl);
	}

	LuaCharacterController& LuaCharacterController::setDelegate(const sol::table& delegate) noexcept
	{
		_delegate = delegate;
		return *this;
	}

	void LuaCharacterController::onAdjustBodyVelocity(CharacterController& character, PhysicsBody& body, glm::vec3& linearVelocity, glm::vec3& angularVelocity)
	{
		if (!_delegate)
		{
			return;
		}
		LuaCharacterController luaChar(character);
		LuaPhysicsBody luaBody(body);
		LuaUtils::callTableDelegate(_delegate, "on_adjust_body_velocity", "running character adjust body velocity",
			[&](auto& func, auto& self)
			{
				return func(self, luaChar, luaBody, linearVelocity, angularVelocity);
			}
		);
	}

	bool LuaCharacterController::onContactValidate(CharacterController& character, PhysicsBody& body)
	{
		if (!_delegate)
		{
			return true;
		}
		LuaCharacterController luaChar(character);
		LuaPhysicsBody luaBody(body);
		return LuaUtils::callTableDelegate(_delegate, "on_contact_validate", "running character contact validate",
			[&](auto& func, auto& self)
			{
				return func(self, luaChar, luaBody);
			}
		);
	}

	void LuaCharacterController::onContactAdded(CharacterController& character, PhysicsBody& body, const Contact& contact, ContactSettings& settings)
	{
		if (!_delegate)
		{
			return;
		}
		LuaCharacterController luaChar(character);
		LuaPhysicsBody luaBody(body);
		LuaUtils::callTableDelegate(_delegate, "on_contact_added", "running character contact added",
			[&](auto& func, auto& self)
			{
				return func(self, luaChar, luaBody, contact, settings);
			}
		);
	}

	void LuaCharacterController::onContactSolve(CharacterController& character, PhysicsBody& body, const Contact& contact, glm::vec3& characterVelocity)
	{
		if (!_delegate)
		{
			return;
		}
		LuaCharacterController luaChar(character);
		LuaPhysicsBody luaBody(body);
		LuaUtils::callTableDelegate(_delegate, "on_contact_solve", "running character contact solve",
			[&](auto& func, auto& self)
			{
				return func(self, luaChar, luaBody, contact, characterVelocity);
			}
		);
	}

	void LuaCharacterController::bind(sol::state_view& lua) noexcept
	{
		Scene::registerComponentDependency<CharacterController, LuaCharacterController>();

		lua.new_usertype<Config>("CharacterControllerConfig", sol::default_constructor,
			"shape", &CharacterControllerConfig::shape,
			"up", &CharacterControllerConfig::up,
			"supportingPlane", &CharacterControllerConfig::supportingPlane,
			"maxSlopeAngle", &CharacterControllerConfig::maxSlopeAngle,
			"layer", &CharacterControllerConfig::layer,
			"maxStrength", &CharacterControllerConfig::maxStrength,
			"backFaceMode", &CharacterControllerConfig::backFaceMode,
			"padding", &CharacterControllerConfig::padding,
			"penetrationRecoverySpeed", &CharacterControllerConfig::penetrationRecoverySpeed,
			"predictiveContactDistance", &CharacterControllerConfig::predictiveContactDistance
		);
		lua.new_usertype<LuaCharacterController>("CharacterController", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<LuaCharacterController>::value),
			"add_entity_component", sol::overload(
				&LuaCharacterController::addEntityComponent1,
				&LuaCharacterController::addEntityComponent2
			),
			"get_entity_component", &LuaCharacterController::getEntityComponent,
			"get_entity", &LuaCharacterController::getEntity,
			"delegate", sol::property(&LuaCharacterController::setDelegate),
			"grounded", sol::property(&LuaCharacterController::isGrounded),
			"ground_state", sol::property(&LuaCharacterController::getGroundState),
			"position", sol::property(&LuaCharacterController::getPosition, &LuaCharacterController::setPosition),
			"rotation", sol::property(&LuaCharacterController::getRotation, &LuaCharacterController::setRotation),
			"linear_velocity", sol::property(&LuaCharacterController::getLinearVelocity, &LuaCharacterController::setLinearVelocity),
			"ground_position", sol::property(&LuaCharacterController::getGroundPosition),
			"ground_normal", sol::property(&LuaCharacterController::getGroundNormal),
			"ground_velocity", sol::property(&LuaCharacterController::getGroundVelocity)
		);
	}
}