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
	}

	LuaCharacterController::~LuaCharacterController() noexcept
	{
		if (_ctrl && _scene->hasComponent(_ctrl.value()))
		{
			_ctrl->removeListener(*this);
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

	LuaCharacterController& LuaCharacterController::addListener(const sol::table& listener) noexcept
	{
		if (_listeners.empty())
		{
			_ctrl->addListener(*this);
		}
		_listeners.emplace_back(listener);
		return *this;
	}

	bool LuaCharacterController::removeListener(const sol::table& listener) noexcept
	{
		auto itr = std::find(_listeners.begin(), _listeners.end(), listener);
		if (itr == _listeners.end())
		{
			return false;
		}
		_listeners.erase(itr);
		if (_listeners.empty())
		{
			_ctrl->removeListener(*this);
		}
		return true;
	}

	void LuaCharacterController::onCollisionEnter(CharacterController& character, PhysicsBody& body, const Collision& collision)
	{
		LuaCharacterController luaChar(character, _scene);
		LuaPhysicsBody luaBody(body, _scene);
		callLuaListeners(_listeners, "on_collision_enter", "running character collision enter",
			[&collision, &luaChar, &luaBody](auto& func)
			{
				return func(luaChar, luaBody, collision);
			});
	}

	void LuaCharacterController::onCollisionStay(CharacterController& character, PhysicsBody& body, const Collision& collision)
	{
		LuaCharacterController luaChar(character, _scene);
		LuaPhysicsBody luaBody(body, _scene);
		callLuaListeners(_listeners, "on_collision_stay", "running character collision stay",
			[&collision, &luaChar, &luaBody](auto& func)
			{
				return func(luaChar, luaBody, collision);
			});
	}

	void LuaCharacterController::onCollisionExit(CharacterController& character, PhysicsBody& body)
	{
		LuaCharacterController luaChar(character, _scene);
		LuaPhysicsBody luaBody(body, _scene);
		callLuaListeners(_listeners, "on_collision_exit", "running character collision exit",
			[&luaChar, &luaBody](auto& func)
			{
				return func(luaChar, luaBody);
			});
	}

	void LuaCharacterController::bind(sol::state_view& lua) noexcept
	{
		lua.new_enum<GroundState>("Physics3dGroundState", {
			{ "Grounded", GroundState::Grounded },
			{ "GroundedSteep", GroundState::GroundedSteep },
			{ "NotSupported", GroundState::NotSupported },
			{ "Air", GroundState::Air },
		});
		lua.new_usertype<LuaCharacterController>("CharacterController", sol::no_constructor,
			"type_id", &entt::type_hash<CharacterController>::value,
			"add_entity_component", sol::overload(
				&LuaCharacterController::addEntityComponent1,
				&LuaCharacterController::addEntityComponent2
			),
			"get_entity_component", &LuaCharacterController::getEntityComponent,
			"get_entity", &LuaCharacterController::getEntity,
			"add_listener", &LuaCharacterController::addListener,
			"remove_listener", &LuaCharacterController::removeListener,
			"grounded", sol::property(&LuaCharacterController::isGrounded),
			"ground_state", sol::property(&LuaCharacterController::getGroundState),
			"position", sol::property(&LuaCharacterController::getPosition, &LuaCharacterController::setPosition),
			"linear_velocity", sol::property(&LuaCharacterController::getLinearVelocity, &LuaCharacterController::setLinearVelocity)
		);
	}
}