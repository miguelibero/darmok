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

	class LuaCharacterController final : public ICharacterControllerListener
	{
	public:
		using Shape = PhysicsShape;
		using Config = CharacterControllerConfig;

		LuaCharacterController(CharacterController& ctrl, const std::shared_ptr<Scene>& scene) noexcept;
		~LuaCharacterController() noexcept;

		GroundState getGroundState() const noexcept;
		void setLinearVelocity(const VarLuaTable<glm::vec3>& velocity);
		glm::vec3 getLinearVelocity() const noexcept;
		void setPosition(const VarLuaTable<glm::vec3>& pos) noexcept;
		glm::vec3 getPosition() const noexcept;

		LuaCharacterController& addListener(const sol::table& listener) noexcept;
		bool removeListener(const sol::table& listener) noexcept;

		void onCollisionEnter(CharacterController& character, PhysicsBody& body, const Collision& collision) override;
		void onCollisionStay(CharacterController& character, PhysicsBody& body, const Collision& collision) override;
		void onCollisionExit(CharacterController& character, PhysicsBody& body) override;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<CharacterController> _ctrl;
		std::shared_ptr<Scene> _scene;
		std::vector<sol::table> _listeners;

		static LuaCharacterController addEntityComponent1(LuaEntity& entity, const Config& config) noexcept;
		static LuaCharacterController addEntityComponent2(LuaEntity& entity, const Shape& shape) noexcept;
		static std::optional<LuaCharacterController> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};
}