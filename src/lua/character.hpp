#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <sol/sol.hpp>
#include <darmok/character.hpp>
#include <darmok/optional_ref.hpp>
#include "glm.hpp"
#include "utils.hpp"

namespace darmok
{
	class LuaEntity;
	class LuaScene;
	class Scene;
}

namespace darmok::physics3d
{
	class CharacterController;

	class LuaCharacterControllerDelegate final : public ICharacterControllerDelegate
	{
	public:
		LuaCharacterControllerDelegate(const sol::table& table) noexcept;
		void onAdjustBodyVelocity(CharacterController& character, PhysicsBody& body, glm::vec3& linearVelocity, glm::vec3& angularVelocity) override;
		bool onContactValidate(CharacterController& character, PhysicsBody& body) override;
		void onContactAdded(CharacterController& character, PhysicsBody& body, const Contact& contact, ContactSettings& settings) override;
		void onContactSolve(CharacterController& character, PhysicsBody& body, const Contact& contact, glm::vec3& characterVelocity) override;
	private:
		sol::table _table;

		static const LuaTableDelegateDefinition _adjustBodyDef;
		static const LuaTableDelegateDefinition _contactValidateDef;
		static const LuaTableDelegateDefinition _contactAddedDef;
		static const LuaTableDelegateDefinition _contactSolveDef;
	};

	class LuaCharacterController final
	{
	public:
		using Shape = PhysicsShape;
		using Config = CharacterControllerConfig;
		static void bind(sol::state_view& lua) noexcept;
	private:
		static void setLinearVelocity(CharacterController& ctrl, const VarLuaTable<glm::vec3>& velocity);
		static void setPosition(CharacterController& ctrl, const VarLuaTable<glm::vec3>& pos) noexcept;
		static void setRotation(CharacterController& ctrl, const VarLuaTable<glm::quat>& rot) noexcept;

		static void setDelegate(CharacterController& ctrl, const sol::table& table) noexcept;

		static CharacterController& addEntityComponent1(LuaEntity& entity, const Config& config) noexcept;
		static CharacterController& addEntityComponent2(LuaEntity& entity, const Shape& shape) noexcept;
		static OptionalRef<CharacterController>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const CharacterController& ctrl, LuaScene& scene) noexcept;
	};
}