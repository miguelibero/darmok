#pragma once

#include "lua/lua.hpp"
#include "lua/glm.hpp"
#include "lua/utils.hpp"

#include <darmok/physics3d_character.hpp>
#include <darmok/optional_ref.hpp>

#include <vector>
#include <optional>
#include <memory>

namespace darmok
{
	class LuaEntity;
	class LuaScene;
	class Scene;
}

namespace darmok::physics3d
{
	class CharacterController;

	class LuaCharacterDelegate final : public ITypeCharacterDelegate<LuaCharacterDelegate>
	{
	public:
		LuaCharacterDelegate(const sol::table& table) noexcept;
		void onAdjustBodyVelocity(CharacterController& character, PhysicsBody& body, glm::vec3& linearVelocity, glm::vec3& angularVelocity) override;
		bool onContactValidate(CharacterController& character, PhysicsBody& body) override;
		void onContactAdded(CharacterController& character, PhysicsBody& body, const Contact& contact, ContactSettings& settings) override;
		void onContactSolve(CharacterController& character, PhysicsBody& body, const Contact& contact, glm::vec3& characterVelocity) override;
	private:
		sol::main_table _table;

		static const LuaTableDelegateDefinition _adjustBodyDef;
		static const LuaTableDelegateDefinition _contactValidateDef;
		static const LuaTableDelegateDefinition _contactAddedDef;
		static const LuaTableDelegateDefinition _contactSolveDef;
	};

	class LuaCharacterController final
	{
	public:
		using Definition = CharacterController::Definition;
		static void bind(sol::state_view& lua) noexcept;
	private:
		static void setLinearVelocity(CharacterController& ctrl, const VarLuaTable<glm::vec3>& velocity);
		static void setPosition(CharacterController& ctrl, const VarLuaTable<glm::vec3>& pos) noexcept;
		static void setRotation(CharacterController& ctrl, const VarLuaTable<glm::quat>& rot) noexcept;

		static void setDelegate(CharacterController& ctrl, const sol::table& table) noexcept;

		static CharacterController& addEntityComponent1(LuaEntity& entity, const Definition& def) noexcept;
		static CharacterController& addEntityComponent2(LuaEntity& entity, const PhysicsShape& shape) noexcept;
		static OptionalRef<CharacterController>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const CharacterController& ctrl, const std::shared_ptr<Scene>& scene) noexcept;
	};
}