#include "app.hpp"
#include <darmok/app.hpp>
#include <darmok/scripting.hpp>
#include <filesystem>
#include "asset.hpp"
#include "input.hpp"
#include "math.hpp"
#include "shape.hpp"
#include "scene.hpp"
#include "window.hpp"

namespace darmok
{
    LuaApp::LuaApp(App& app) noexcept
		: _app(app)
	{
	}

	App& LuaApp::getReal() noexcept
	{
		return _app.value();
	}

	const App& LuaApp::getReal() const noexcept
	{
		return _app.value();
	}

	SceneAppComponent& LuaApp::getSceneComponent() noexcept
	{
		if (!_sceneComponent)
		{
			_sceneComponent = _app->addComponent<SceneAppComponent>();
		}
		return _sceneComponent.value();
	}

	LuaScene LuaApp::getScene() noexcept
	{
		return LuaScene(getSceneComponent().getScene());
	}

	void LuaApp::setScene(const LuaScene& scene) noexcept
	{
		getSceneComponent().setScene(scene.getReal());
	}

	std::vector<LuaScene> LuaApp::getScenes() noexcept
	{
		std::vector<LuaScene> scenes;
		for (auto& scene : getSceneComponent().getScenes())
		{
			scenes.push_back(LuaScene(scene));
		}
		return scenes;
	}

	bool LuaApp::addScene1(const LuaScene& scene) noexcept
	{
		return getSceneComponent().addScene(scene.getReal());
	}

	LuaScene LuaApp::addScene2() noexcept
	{
		return LuaScene(getSceneComponent().addScene());
	}

	bool LuaApp::removeScene(const LuaScene& scene) noexcept
	{
		return getSceneComponent().removeScene(scene.getReal());
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
			"scene", sol::property(&LuaApp::getScene, &LuaApp::setScene),
			"scenes", sol::property(&LuaApp::getScenes),
			"add_scene", sol::overload(&LuaApp::addScene1, &LuaApp::addScene2),
			"remove_scene", &LuaApp::removeScene,
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

	std::string ScriptingAppImpl::findMainLua(const std::vector<std::string>& args) noexcept
	{
		static const std::vector<std::string> possiblePaths = {
			"main.lua",
			"lua/main.lua",
			"assets/main.lua"
		};

		if (!args.empty() && args[0].ends_with(".lua"))
		{
			return args[0];
		}
		else
		{
			for (auto& path : possiblePaths)
			{
				if (std::filesystem::exists(path))
				{
					return path;
				}
			}
		}
		return possiblePaths[0];
	}

	void ScriptingAppImpl::addPackagePath(const std::string& path) noexcept
	{
		auto& lua = *_lua;
		const std::string current = lua["package"]["path"];
		lua["package"]["path"] = current + (!current.empty() ? ";" : "") + path + "?.lua";
	}

	void ScriptingAppImpl::init(App& app, const std::vector<std::string>& args)
	{
		auto mainFile = findMainLua(args);
		auto mainDir = std::filesystem::path(mainFile).parent_path().string();

		_lua = std::make_unique<sol::state>();
		auto& lua = *_lua;
		lua.open_libraries(sol::lib::base, sol::lib::package);
		
		LuaMath::configure1(lua);
		LuaMath::configure2(lua);
		LuaShape::configure(lua);
		LuaAssets::configure(lua);
		LuaScene::configure(lua);
		LuaWindow::configure(lua);
		LuaInput::configure(lua);
		LuaApp::configure(lua);

		_luaApp = LuaApp(app);
		lua["app"] = std::ref(_luaApp);
		lua["args"] = args;

		if (!mainDir.empty())
		{
			addPackagePath(mainDir);
		}
		
		auto result = lua.script_file(mainFile);
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