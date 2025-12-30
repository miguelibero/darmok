#include <darmok/lua_script.hpp>
#include "lua/utils.hpp"
#include "lua/scene.hpp"
#include "lua/app.hpp"

namespace darmok
{
	LuaScript::Definition LuaScript::createDefinition() noexcept
	{
		Definition def;
		def.set_content(R"(
function init(entity)
end

function update(dt)
end

function shutdown()
end
)");
		return def;
	}

	LuaScript::LuaScript(std::string_view content) noexcept
		: _content{ content }
	{
	}

	LuaScript::~LuaScript() noexcept = default;

	expected<void, std::string> LuaScript::load(const Definition& def) noexcept
	{
		_content = def.content();
		if (_entity)
		{
			return doInit();
		}
		return {};
	}

	expected<void, std::string> LuaScript::init(sol::state_view& lua, const LuaEntity& entity) noexcept
	{
		auto luaResult = lua.safe_script(_content, sol::script_pass_on_error);
		if (!luaResult.valid())
		{
			sol::error err = luaResult;
			return unexpected<std::string>{ err.what() };
		}
		_obj = luaResult;
		_entity = std::make_unique<LuaEntity>(entity);
		return doInit();
	}

	expected<void, std::string> LuaScript::doInit() noexcept
	{
		if (!_entity)
		{
			return unexpected<std::string>{ "missing entity" };
		}
		auto initResult = LuaDelegate{ _obj, "init", "lua script init" }.tryGet<void>(*_entity);
		if (!initResult)
		{
			return initResult;
		}
		_update = std::make_unique<LuaDelegate>(_obj, "update", "lua script update");
		return {};
	}

	expected<void, std::string> LuaScript::shutdown() noexcept
	{
		_update.reset();
		_entity.reset();
		return LuaDelegate{ _obj, "shutdown", "lua script shutdown" }.tryGet<void>();
	}

	expected<void, std::string> LuaScript::update(float deltaTime) noexcept
	{
		if (!_update)
		{
			return {};
		}
		return _update->tryGet<void>(deltaTime);
	}

	LuaScriptRunner::Definition LuaScriptRunner::createDefinition() noexcept
	{
		Definition def;
		return def;
	}

	LuaScriptRunner::LuaScriptRunner() noexcept
	{
	}

	LuaScriptRunner::LuaScriptRunner(const sol::state_view& lua) noexcept
		: _luaView{ lua }
	{
	}

	expected<void, std::string> LuaScriptRunner::init(Scene& scene, App& app) noexcept
	{
		if (!_luaView)
		{
			_lua = createLuaState(app);
			_luaView = *_lua;
		}
		_scene = scene;
		auto appScenesResult = app.getOrAddComponent<SceneAppComponent>();
		if (!appScenesResult)
		{
			return unexpected{ std::move(appScenesResult).error() };
		}
		for (auto scenePtr : appScenesResult.value().get().getScenes())
		{
			if (&*scenePtr == &scene)
			{
				_scenePtr = scenePtr;
			}
		}

		scene.onConstructComponent<LuaScript>().connect<&LuaScriptRunner::onScriptConstructed>(*this);
		scene.onDestroyComponent<LuaScript>().connect<&LuaScriptRunner::onScriptDestroyed>(*this);
		_entities.clear();
		for (auto entity : _scene->getComponents<LuaScript>())
		{
			auto comp = _scene->getComponent<LuaScript>(entity);
			auto result = comp->init(*_luaView, LuaEntity{ entity, _scenePtr });
			if (!result)
			{
				return result;
			}
			_entities.push_back(entity);
		}
		return {};
	}

	expected<void, std::string> LuaScriptRunner::load(const Definition& def) noexcept
	{
		return {};
	}

	expected<void, std::string> LuaScriptRunner::shutdown() noexcept
	{
		if (_scene)
		{
			_scene->onConstructComponent<LuaScript>().disconnect<&LuaScriptRunner::onScriptConstructed>(*this);
			_scene->onDestroyComponent<LuaScript>().disconnect<&LuaScriptRunner::onScriptDestroyed>(*this);
			_scene.reset();
		}
		for (auto entity : _entities)
		{
			auto comp = _scene->getComponent<LuaScript>(entity);
			if(!comp)
			{
				continue;
			}
			auto result = comp->shutdown();
			if (!result)
			{
				return result;
			}
		}
		_entities.clear();
		return {};
	}

	expected<void, std::string> LuaScriptRunner::onScriptConstructed(EntityRegistry& registry, Entity entity) noexcept
	{
		return {};
	}

	expected<void, std::string> LuaScriptRunner::onScriptDestroyed(EntityRegistry& registry, Entity entity) noexcept
	{
		if (!updatingEntity(entity))
		{
			return {};
		}
		if (!_scene)
		{
			return unexpected<std::string>{ "runner not initialized" };
		}
		return _scene->getComponent<LuaScript>(entity)->shutdown();
	}

	bool LuaScriptRunner::updatingEntity(Entity entity) const noexcept
	{
		return std::find(_entities.begin(), _entities.end(), entity) != _entities.end();
	}

	expected<void, std::string> LuaScriptRunner::update(float deltaTime) noexcept
	{
		if (!_scene || !_luaView)
		{
			return unexpected<std::string>{ "runner not initialized" };
		}
		for (auto entity : _scene->getComponents<LuaScript>())
		{
			auto comp = _scene->getComponent<LuaScript>(entity);
			if (!updatingEntity(entity))
			{
				auto result = comp->init(*_luaView, LuaEntity{ entity, _scenePtr });
				if (!result)
				{
					return result;
				}
			}
			auto result = comp->update(deltaTime);
			if (!result)
			{
				return result;
			}
		}
		return {};
	}
}