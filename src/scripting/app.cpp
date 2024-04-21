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

	void LuaApp::configure(sol::state_view& lua) noexcept
	{
		auto usertype = lua.new_usertype<LuaApp>("App");
		usertype["scene"] = sol::property(&LuaApp::getScene);
		usertype["assets"] = sol::property(&LuaApp::getAssets);
		usertype["window"] = sol::property(&LuaApp::getWindow);
		usertype["inpuy"] = sol::property(&LuaApp::getWindow);
		usertype["input"] = sol::property(&LuaApp::getInput);

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

		lua["app"] = LuaApp(app);
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
			_luaUpdate = lua["update"];
		}
	}

	void ScriptingAppImpl::updateLogic(float deltaTime)
	{
		if (_luaUpdate)
		{
			auto result = _luaUpdate(deltaTime);
			if (!result.valid())
			{
				sol::error err = result;
				std::cerr << "error running update:" << std::endl;
				std::cerr << err.what() << std::endl;
			}
		}
	}

	void ScriptingAppImpl::shutdown() noexcept
	{
		_lua.reset();
		_luaUpdate.reset();
	}
}