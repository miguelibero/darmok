#include "character.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include "physics3d.hpp"
#include <darmok/character.hpp>

namespace darmok::physics3d
{
	LuaCharacterController::LuaCharacterController(CharacterController& ctrl, const std::shared_ptr<Scene>& scene) noexcept
		: _ctrl(ctrl)
		, _scene(scene)
	{
		ctrl.setDelegate(this);
	}

	LuaCharacterController::~LuaCharacterController() noexcept
	{
		if (_ctrl && _scene->hasComponent(_ctrl.value()))
		{
			_ctrl->setDelegate(nullptr);
		}
	}

	bool LuaCharacterController::isGrounded() const noexcept
	{
		return _ctrl->isGrounded();
	}

	GroundState LuaCharacterController::getGroundState() const noexcept
	{
		return _ctrl->getGroundState();
	}

	void LuaCharacterController::setLinearVelocity(const VarLuaTable<glm::vec3>& velocity)
	{
		_ctrl->setLinearVelocity(LuaGlm::tableGet(velocity));
	}

	glm::vec3 LuaCharacterController::getLinearVelocity() const noexcept
	{
		return _ctrl->getLinearVelocity();
	}

	void LuaCharacterController::setPosition(const VarLuaTable<glm::vec3>& pos) noexcept
	{
		_ctrl->setPosition(LuaGlm::tableGet(pos));
	}

	glm::vec3 LuaCharacterController::getPosition() const noexcept
	{
		return _ctrl->getPosition();
	}

	LuaCharacterController LuaCharacterController::addEntityComponent1(LuaEntity& entity, const Config& config) noexcept
	{
		return LuaCharacterController(entity.addComponent<CharacterController>(config), entity.getScene().getReal());
	}

	LuaCharacterController LuaCharacterController::addEntityComponent2(LuaEntity& entity, const Shape& shape) noexcept
	{
		return LuaCharacterController(entity.addComponent<CharacterController>(shape), entity.getScene().getReal());
	}

	std::optional<LuaCharacterController> LuaCharacterController::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<CharacterController, LuaCharacterController>(entity.getScene().getReal());
	}

	std::optional<LuaEntity> LuaCharacterController::getEntity(LuaScene& scene) noexcept
	{
		return scene.getEntity(_ctrl.value());
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
		LuaCharacterController luaChar(character, _scene);
		LuaPhysicsBody luaBody(body, _scene);
		callLuaDelegate(_delegate, "on_adjust_body_velocity", "running character adjust body velocity",
			[&](auto& func)
			{
				return func(luaChar, luaBody, linearVelocity, angularVelocity);
			}
		);
	}

	bool LuaCharacterController::onContactValidate(CharacterController& character, PhysicsBody& body)
	{
		if (!_delegate)
		{
			return true;
		}
		LuaCharacterController luaChar(character, _scene);
		LuaPhysicsBody luaBody(body, _scene);
		auto result = callLuaDelegate(_delegate, "on_contact_validate", "running character contact validate",
			[&](auto& func)
			{
				return func(luaChar, luaBody);
			}
		);
		if (!result.valid())
		{
			return true;
		}
		return result;
	}

	void LuaCharacterController::onContactAdded(CharacterController& character, PhysicsBody& body, const Contact& contact, ContactSettings& settings)
	{
		if (!_delegate)
		{
			return;
		}
		LuaCharacterController luaChar(character, _scene);
		LuaPhysicsBody luaBody(body, _scene);
		callLuaDelegate(_delegate, "on_contact_added", "running character contact added",
			[&](auto& func)
			{
				return func(luaChar, luaBody, contact, settings);
			}
		);
	}

	void LuaCharacterController::onContactSolve(CharacterController& character, PhysicsBody& body, const Contact& contact, glm::vec3& characterVelocity)
	{
		if (!_delegate)
		{
			return;
		}
		LuaCharacterController luaChar(character, _scene);
		LuaPhysicsBody luaBody(body, _scene);
		callLuaDelegate(_delegate, "on_contact_solve", "running character contact solve",
			[&](auto& func)
			{
				return func(luaChar, luaBody, contact, characterVelocity);
			}
		);
	}

	void LuaCharacterController::bind(sol::state_view& lua) noexcept
	{
		lua.new_enum<GroundState>("Physics3dGroundState", {
			{ "Grounded", GroundState::Grounded },
			{ "GroundedSteep", GroundState::GroundedSteep },
			{ "NotSupported", GroundState::NotSupported },
			{ "Air", GroundState::Air },
		});
		lua.new_usertype<CharacterConfig>("Physics3dCharacterConfig", sol::default_constructor,
			"shape", &CharacterConfig::shape,
			"up", &CharacterConfig::up,
			"supportingPlane", &CharacterConfig::supportingPlane,
			"maxSlopeAngle", &CharacterConfig::maxSlopeAngle,
			"layer", &CharacterConfig::layer,
			"mass", &CharacterConfig::mass,
			"friction", &CharacterConfig::friction,
			"gravityFactor", &CharacterConfig::gravityFactor
		);

		lua.new_usertype<LuaCharacterController>("CharacterController", sol::no_constructor,
			"type_id", &entt::type_hash<CharacterController>::value,
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
			"linear_velocity", sol::property(&LuaCharacterController::getLinearVelocity, &LuaCharacterController::setLinearVelocity)
		);
	}
}