#pragma once

#include <darmok/protobuf/scene.pb.h>
#include <darmok/expected.hpp>
#include <darmok/scene.hpp>
#include <sol/sol.hpp>

namespace darmok
{
	class LuaEntity;
	class LuaDelegate;

	class LuaScript final
	{
	public:
		using Definition = protobuf::LuaScript;

		LuaScript(std::string_view content = {}) noexcept;
		~LuaScript() noexcept;

		expected<void, std::string> init(sol::state_view& lua, const LuaEntity& entity) noexcept;
		expected<void, std::string> load(const Definition& def) noexcept;
		expected<void, std::string> shutdown() noexcept;
		expected<void, std::string> update(float deltaTime) noexcept;

		static Definition createDefinition() noexcept;
	private:
		std::string _content;
		sol::object _obj;
		std::unique_ptr<LuaDelegate> _update;
		std::unique_ptr<LuaEntity> _entity;

		expected<void, std::string> doInit() noexcept;
	};

	class LuaScriptRunner final : public ITypeSceneComponent<LuaScriptRunner>
	{
	public:
		using Definition = protobuf::LuaScriptRunner;

		LuaScriptRunner() noexcept;
		LuaScriptRunner(const sol::state_view& lua) noexcept;
		expected<void, std::string> init(Scene& scene, App& app) noexcept override;
		expected<void, std::string> load(const Definition& def) noexcept;
		expected<void, std::string> shutdown() noexcept override;
		expected<void, std::string> update(float deltaTime) noexcept override;

		static Definition createDefinition() noexcept;
	private:
		std::optional<sol::state_view> _luaView;
		std::optional<sol::state> _lua;
		std::weak_ptr<Scene> _scenePtr;
		OptionalRef<Scene> _scene;
		std::vector<Entity> _entities;

		bool updatingEntity(Entity entity) const noexcept;
		expected<void, std::string> onScriptConstructed(EntityRegistry& registry, Entity entity) noexcept;
		expected<void, std::string> onScriptDestroyed(EntityRegistry& registry, Entity entity) noexcept;
	};
}