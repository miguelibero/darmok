#include "app.hpp"
#include <darmok/app.hpp>
#include <darmok/scripting.hpp>
#include "asset.hpp"
#include "input.hpp"
#include "math.hpp"
#include "scene.hpp"
#include "window.hpp"

namespace darmok
{
    LuaApp::LuaApp(App& app) noexcept
		: _app(app)
	{
	}

	LuaScene LuaApp::getScene() noexcept
	{
		if (!_scene)
		{
			_scene = _app->addComponent<SceneAppComponent>().getScene();
		}
		return LuaScene(_scene.value());
	}

	LuaAssets LuaApp::getAssets() noexcept
	{
		return LuaAssets(_app->getAssets());
	}

	LuaWindow LuaApp::getWindow() noexcept
	{
		return LuaWindow(_app->getWindow());
	}

	LuaInput LuaApp::getInput() noexcept
	{
		return LuaInput(_app->getInput());
	}

	void LuaApp::registerUpdate(const sol::protected_function& func) noexcept
	{
		if (func)
		{
			_updates.push_back(func);
		}
	}

	bool LuaApp::unregisterUpdate(const sol::protected_function& func) noexcept
	{
		auto itr = std::find(_updates.begin(), _updates.end(), func);
		if (itr == _updates.end())
		{
			return false;
		}
		_updates.erase(itr);
		return true;
	}

	void LuaApp::update(float deltaTime) noexcept
	{
		for(auto& update : _updates)
		{
			if (!update)
			{
				continue;
			}
			auto result = update(deltaTime);
			if (result.valid())
			{
				continue;
			}
			sol::error err = result;
			std::cerr << "error running update:" << std::endl;
			std::cerr << err.what() << std::endl;
		}
	}

	void LuaApp::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaApp>("App",
			"scene", sol::property(&LuaApp::getScene),
			"assets", sol::property(&LuaApp::getAssets),
			"window", sol::property(&LuaApp::getWindow),
			"input", sol::property(&LuaApp::getInput),
			"register_update", &LuaApp::registerUpdate,
			"unregister_update", &LuaApp::unregisterUpdate
		);
	}

	ScriptingApp::ScriptingApp()
		: _impl(std::make_unique<ScriptingAppImpl>())
	{
	}

	ScriptingApp::~ScriptingApp()
	{
		// intentionally left blank for the unique_ptr<AppImpl> forward declaration
	}

	void ScriptingApp::init(const std::vector<std::string>& args)
	{
		App::init(args);
		_impl->init(*this, args);
	}

	void ScriptingApp::updateLogic(float deltaTime)
	{
		App::updateLogic(deltaTime);
		_impl->updateLogic(deltaTime);
	}		

	void ScriptingAppImpl::init(App& app, const std::vector<std::string>& args)
	{
		_lua = std::make_unique<sol::state>();
		auto& lua = *_lua;
		lua.open_libraries(sol::lib::base, sol::lib::package);
		
		LuaMath::configure1(lua);
		LuaMath::configure2(lua);
		LuaAssets::configure(lua);
		LuaScene::configure(lua);
		LuaWindow::configure(lua);
		LuaInput::configure(lua);
		LuaApp::configure(lua);

		_luaApp = LuaApp(app);

		lua["app"] = std::ref(_luaApp);
		lua["args"] = args;

		auto result = lua.script_file("main.lua");
		if (!result.valid())
		{
			sol::error err = result;
			std::cerr << "error running main:" << std::endl;
			std::cerr << err.what() << std::endl;
		}
		else
		{
			_luaApp->registerUpdate(lua["update"]);
		}
	}

	void ScriptingAppImpl::updateLogic(float deltaTime)
	{
		if (_luaApp)
		{
			_luaApp->update(deltaTime);
		}
	}

	void ScriptingAppImpl::shutdown() noexcept
	{
		_lua.reset();
		_luaApp.reset();
	}
}