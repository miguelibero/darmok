#include "character.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include "physics3d.hpp"
#include <darmok/character.hpp>

namespace darmok::physics3d
{
	LuaCharacterControllerDelegate::LuaCharacterControllerDelegate(const sol::table& table) noexcept
		: _table(table)
	{
	}

	const LuaTableDelegateDefinition LuaCharacterControllerDelegate::_adjustBodyDef("on_adjust_body_velocity", "running character adjust body velocity");
	const LuaTableDelegateDefinition LuaCharacterControllerDelegate::_contactValidateDef("on_contact_validate", "running character contact validate");
	const LuaTableDelegateDefinition LuaCharacterControllerDelegate::_contactAddedDef("on_contact_added", "running character contact added");
	const LuaTableDelegateDefinition LuaCharacterControllerDelegate::_contactSolveDef("on_contact_solve", "running character contact solve");

	void LuaCharacterControllerDelegate::onAdjustBodyVelocity(CharacterController& character, PhysicsBody& body, glm::vec3& linearVelocity, glm::vec3& angularVelocity)
	{
		_adjustBodyDef(_table, character, body, linearVelocity, angularVelocity);
	}

	bool LuaCharacterControllerDelegate::onContactValidate(CharacterController& character, PhysicsBody& body)
	{
		return _contactValidateDef(_table, character, body);
	}

	void LuaCharacterControllerDelegate::onContactAdded(CharacterController& character, PhysicsBody& body, const Contact& contact, ContactSettings& settings)
	{
		_contactAddedDef(_table, character, body, contact, settings);
	}

	void LuaCharacterControllerDelegate::onContactSolve(CharacterController& character, PhysicsBody& body, const Contact& contact, glm::vec3& characterVelocity)
	{
		_contactSolveDef(_table, character, body, contact, characterVelocity);
	}

	void LuaCharacterController::setLinearVelocity(CharacterController& ctrl, const VarLuaTable<glm::vec3>& velocity)
	{
		ctrl.setLinearVelocity(LuaGlm::tableGet(velocity));
	}

	void LuaCharacterController::setPosition(CharacterController& ctrl, const VarLuaTable<glm::vec3>& pos) noexcept
	{
		ctrl.setPosition(LuaGlm::tableGet(pos));
	}

	void LuaCharacterController::setRotation(CharacterController& ctrl, const VarLuaTable<glm::quat>& rot) noexcept
	{
		ctrl.setRotation(LuaGlm::tableGet(rot));
	}

	CharacterController& LuaCharacterController::addEntityComponent1(LuaEntity& entity, const Config& config) noexcept
	{
		return entity.addComponent<CharacterController>(config);
	}

	CharacterController& LuaCharacterController::addEntityComponent2(LuaEntity& entity, const Shape& shape) noexcept
	{
		return entity.addComponent<CharacterController>(shape);
	}

	OptionalRef<CharacterController>::std_t LuaCharacterController::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<CharacterController>();
	}

	std::optional<LuaEntity> LuaCharacterController::getEntity(const CharacterController& ctrl, LuaScene& scene) noexcept
	{
		return scene.getEntity(ctrl);
	}

	void LuaCharacterController::setDelegate(CharacterController& ctrl, const sol::table& table) noexcept
	{
		ctrl.setDelegate(std::make_unique<LuaCharacterControllerDelegate>(table));
	}

	void LuaCharacterController::bind(sol::state_view& lua) noexcept
	{
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
		lua.new_usertype<CharacterController>("CharacterController", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<CharacterController>::value),
			"add_entity_component", sol::overload(
				&LuaCharacterController::addEntityComponent1,
				&LuaCharacterController::addEntityComponent2
			),
			"get_entity_component", &LuaCharacterController::getEntityComponent,
			"get_entity", &LuaCharacterController::getEntity,
			"delegate", sol::property(&LuaCharacterController::setDelegate),
			"grounded", sol::property(&CharacterController::isGrounded),
			"ground_state", sol::property(&CharacterController::getGroundState),
			"position", sol::property(&CharacterController::getPosition, &LuaCharacterController::setPosition),
			"rotation", sol::property(&CharacterController::getRotation, &LuaCharacterController::setRotation),
			"linear_velocity", sol::property(&CharacterController::getLinearVelocity, &LuaCharacterController::setLinearVelocity),
			"ground_position", sol::property(&CharacterController::getGroundPosition),
			"ground_normal", sol::property(&CharacterController::getGroundNormal),
			"ground_velocity", sol::property(&CharacterController::getGroundVelocity)
		);
	}
}