#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <sol/sol.hpp>
#include <darmok/character.hpp>
#include <darmok/optional_ref.hpp>
#include "glm.hpp"

namespace darmok
{
	class LuaEntity;
	class LuaScene;
	class Scene;
}

namespace darmok::physics3d
{
	class CharacterController;

	class LuaCharacterController final : ICharacterControllerDelegate
	{
	public:
		using Shape = PhysicsShape;
		using Config = CharacterControllerConfig;

		LuaCharacterController(CharacterController& ctrl) noexcept;
		~LuaCharacterController() noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		CharacterController& _ctrl;
		sol::table _delegate;

		void onAdjustBodyVelocity(CharacterController& character, PhysicsBody& body, glm::vec3& linearVelocity, glm::vec3& angularVelocity) override;
		bool onContactValidate(CharacterController& character, PhysicsBody& body) override;
		void onContactAdded(CharacterController& character, PhysicsBody& body, const Contact& contact, ContactSettings& settings) override;
		void onContactSolve(CharacterController& character, PhysicsBody& body, const Contact& contact, glm::vec3& characterVelocity) override;

		bool isGrounded() const noexcept;
		GroundState getGroundState() const noexcept;
		void setLinearVelocity(const VarLuaTable<glm::vec3>& velocity);
		glm::vec3 getLinearVelocity() const noexcept;
		void setPosition(const VarLuaTable<glm::vec3>& pos) noexcept;
		glm::vec3 getPosition() const noexcept;
		void setRotation(const VarLuaTable<glm::quat>& rot) noexcept;
		glm::quat getRotation() const noexcept;
		glm::vec3 getGroundNormal() const noexcept;
		glm::vec3 getGroundPosition() const noexcept;
		glm::vec3 getGroundVelocity() const noexcept;

		LuaCharacterController& setDelegate(const sol::table& delegate) noexcept;

		static LuaCharacterController& addEntityComponent1(LuaEntity& entity, const Config& config) noexcept;
		static LuaCharacterController& addEntityComponent2(LuaEntity& entity, const Shape& shape) noexcept;
		static OptionalRef<LuaCharacterController>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};
}