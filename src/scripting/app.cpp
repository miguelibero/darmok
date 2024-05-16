#include "app.hpp"
#include <darmok/app.hpp>
#include <darmok/scripting.hpp>
#include <darmok/scene.hpp>
#include <darmok/utils.hpp>
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

	LuaApp::SceneComponents::iterator LuaApp::findSceneComponent(const LuaScene& scene) noexcept
	{
		auto real = scene.getReal();
		return std::find_if(_sceneComponents.begin(), _sceneComponents.end(),
			[real](auto& comp) { return comp->getScene() == real; });
	}

	LuaScene LuaApp::getScene() noexcept
	{
		if (_sceneComponents.empty())
		{
			_sceneComponents.push_back(_app->addComponent<SceneAppComponent>());
		}
		return LuaScene(_sceneComponents.front()->getScene());
	}

	void LuaApp::setScene(const LuaScene& scene) noexcept
	{
		if (_sceneComponents.empty())
		{
			_sceneComponents.push_back(_app->addComponent<SceneAppComponent>(scene.getReal()));
		}
		else
		{
			_sceneComponents.front()->setScene(scene.getReal());
		}
	}

	std::vector<LuaScene> LuaApp::getScenes() noexcept
	{
		std::vector<LuaScene> scenes;
		for (auto& comp : _sceneComponents)
		{
			scenes.push_back(LuaScene(comp->getScene()));
		}
		return scenes;
	}

	bool LuaApp::addScene1(const LuaScene& scene) noexcept
	{
		auto itr = findSceneComponent(scene);
		if (itr != _sceneComponents.end())
		{
			return false;
		}
		getScene();
		auto& comp = _app->addComponent<SceneAppComponent>(scene.getReal());
		_sceneComponents.push_back(comp);
		return true;
	}

	LuaScene LuaApp::addScene2() noexcept
	{
		getScene();
		auto& comp = _app->addComponent<SceneAppComponent>();
		_sceneComponents.push_back(comp);
		return LuaScene(comp.getScene());
	}

	bool LuaApp::removeScene(const LuaScene& scene) noexcept
	{
		auto itr = findSceneComponent(scene);
		if (itr == _sceneComponents.end())
		{
			return false;
		}
		_sceneComponents.erase(itr);
		return true;
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

		if (args.size() > 1 && StringUtils::endsWith(args[1], ".lua"))
		{
			return args[1];
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
		std::string current = lua["package"]["path"];
		auto pathPattern = std::filesystem::path(path) / std::filesystem::path("?.lua");
		current += (!current.empty() ? ";" : "") + std::filesystem::absolute(pathPattern).string();
		lua["package"]["path"] = current;
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
		LuaMath::configure3(lua);
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