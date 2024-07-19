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

	class LuaCharacterController final : public ICharacterControllerDelegate
	{
	public:
		using Shape = PhysicsShape;
		using Config = CharacterControllerConfig;

		LuaCharacterController(CharacterController& ctrl, const std::shared_ptr<Scene>& scene) noexcept;
		~LuaCharacterController() noexcept;

		void onAdjustBodyVelocity(CharacterController& character, PhysicsBody& body, glm::vec3& linearVelocity, glm::vec3& angularVelocity) override;
		bool onContactValidate(CharacterController& character, PhysicsBody& body) override;
		void onContactAdded(CharacterController& character, PhysicsBody& body, const Contact& contact, ContactSettings& settings) override;
		void onContactSolve(CharacterController& character, PhysicsBody& body, const Contact& contact, glm::vec3& characterVelocity) override;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::reference_wrapper<CharacterController> _ctrl;
		std::shared_ptr<Scene> _scene;
		sol::table _delegate;

		bool isGrounded() const noexcept;
		GroundState getGroundState() const noexcept;
		void setLinearVelocity(const VarLuaTable<glm::vec3>& velocity);
		glm::vec3 getLinearVelocity() const noexcept;
		void setPosition(const VarLuaTable<glm::vec3>& pos) noexcept;
		glm::vec3 getPosition() const noexcept;

		LuaCharacterController& setDelegate(const sol::table& delegate) noexcept;

		static LuaCharacterController addEntityComponent1(LuaEntity& entity, const Config& config) noexcept;
		static LuaCharacterController addEntityComponent2(LuaEntity& entity, const Shape& shape) noexcept;
		static std::optional<LuaCharacterController> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};
}