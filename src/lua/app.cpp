#include "app.hpp"
#include <darmok/app.hpp>
#include <darmok/lua.hpp>
#include <darmok/scene.hpp>
#include <darmok/rmlui.hpp>
#include <darmok/string.hpp>
#include <filesystem>
#include "asset.hpp"
#include "input.hpp"
#include "math.hpp"
#include "shape.hpp"
#include "scene.hpp"
#include "window.hpp"
#include "texture.hpp"
#include "rmlui.hpp"
#include "skeleton.hpp"
#include "utils.hpp"

namespace darmok
{
    LuaApp::LuaApp(App& app) noexcept
		: _app(app)
		, _assets(app.getAssets())
		, _window(app.getWindow())
		, _input(app.getInput())
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

	LuaScene& LuaApp::getScene() noexcept
	{
		if (_sceneComponents.empty())
		{
			return doAddScene();
		}
		return _scenes.front();
	}

	size_t LuaApp::findScene(const LuaScene& scene) const noexcept
	{
		size_t i = 0;
		auto& real = scene.getReal();
		for (auto& comp : _sceneComponents)
		{
			if (comp->getScene() == real)
			{
				return i;
			}
			i++;
		}
		return -1;
	}

	LuaScene& LuaApp::setScene(const LuaScene& scene) noexcept
	{
		if (_sceneComponents.empty())
		{
			auto& comp = _app->addComponent<SceneAppComponent>(scene.getReal());
			_sceneComponents.push_back(comp);
			return _scenes.emplace_back(scene);
		}
		_sceneComponents[0]->setScene(scene.getReal());
		_scenes[0] = scene;
		return _scenes[0];
	}

	const std::vector<LuaScene>& LuaApp::getScenes() noexcept
	{
		return _scenes;
	}

	LuaScene& LuaApp::addScene1(const LuaScene& scene) noexcept
	{
		auto i = findScene(scene);
		if (i >= 0)
		{
			return _scenes[i];
		}
		getScene();
		auto& comp = _app->addComponent<SceneAppComponent>(scene.getReal());
		_sceneComponents.push_back(comp);
		return _scenes.emplace_back(scene);
	}

	LuaScene& LuaApp::addScene2() noexcept
	{
		getScene();
		return doAddScene();
	}

	LuaScene& LuaApp::doAddScene() noexcept
	{
		auto& comp = _app->addComponent<SceneAppComponent>();
		_sceneComponents.push_back(comp);
		return _scenes.emplace_back(comp.getScene());
	}

	bool LuaApp::removeScene(const LuaScene& scene) noexcept
	{
		auto i = findScene(scene);
		if (i < 0)
		{
			return false;
		}
		_sceneComponents.erase(_sceneComponents.begin() + i);
		_scenes.erase(_scenes.begin() + i);
		return true;
	}

	LuaRmluiAppComponent& LuaApp::getMainGui() noexcept
	{
		static const std::string name = "";
		return getOrAddGui(name);
	}

	OptionalRef<LuaRmluiAppComponent>::std_t LuaApp::getGui(const std::string& name) noexcept
	{
		auto itr = _guiComponents.find(name);
		if (itr == _guiComponents.end())
		{
			return std::nullopt;
		}
		return itr->second;
	}

	LuaRmluiAppComponent& LuaApp::getOrAddGui(const std::string& name) noexcept
	{
		auto itr = _guiComponents.find(name);
		if (itr == _guiComponents.end())
		{
			auto& comp = _app->addComponent<RmluiAppComponent>(name);
			itr = _guiComponents.emplace(name, comp).first;
		}
		return itr->second;
	}

	LuaRmluiAppComponent& LuaApp::addGui(const std::string& name)
	{
		auto& comp = _app->addComponent<RmluiAppComponent>(name);
		auto r = _guiComponents.emplace(name, comp);
		return r.first->second;
	}

	bool LuaApp::removeGui(const std::string& name) noexcept
	{
		auto itr = _guiComponents.find(name);
		if (itr == _guiComponents.end())
		{
			return false;
		}
		_guiComponents.erase(itr);
		return true;
	}

	LuaAssets& LuaApp::getAssets() noexcept
	{
		return _assets;
	}

	LuaWindow& LuaApp::getWindow() noexcept
	{
		return _window;
	}

	LuaInput& LuaApp::getInput() noexcept
	{
		return _input;
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
			if (!result.valid())
			{
				recoveredLuaError("running update", result);
			}
		}
	}

	void LuaApp::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaApp>("App", sol::no_constructor,
			"scene", sol::property(&LuaApp::getScene, &LuaApp::setScene),
			"scenes", sol::property(&LuaApp::getScenes),
			"add_scene", sol::overload(&LuaApp::addScene1, &LuaApp::addScene2),
			"remove_scene", &LuaApp::removeScene,
			
			"gui", sol::property(&LuaApp::getMainGui),
			"get_gui", &LuaApp::getGui,
			"get_or_add_gui", &LuaApp::getOrAddGui,
			"add_gui", &LuaApp::addGui,

			"assets", sol::property(&LuaApp::getAssets),
			"window", sol::property(&LuaApp::getWindow),
			"input", sol::property(&LuaApp::getInput),
			"register_update", &LuaApp::registerUpdate,
			"unregister_update", &LuaApp::unregisterUpdate
		);
	}

	LuaRunnerApp::LuaRunnerApp()
		: _impl(std::make_unique<LuaRunnerAppImpl>())
	{
	}

	LuaRunnerApp::~LuaRunnerApp()
	{
		// intentionally left blank for the unique_ptr<LuaRunnerAppImpl> forward declaration
	}

	void LuaRunnerApp::init(const std::vector<std::string>& args)
	{
		App::init(args);
		_impl->init(*this, args);
	}

	int LuaRunnerApp::shutdown()
	{
		_impl->beforeShutdown();
		auto r = App::shutdown();
		_impl->afterShutdown();
		return r;
	}

	void LuaRunnerApp::updateLogic(float deltaTime)
	{
		App::updateLogic(deltaTime);
		_impl->updateLogic(deltaTime);
	}

	std::string LuaRunnerAppImpl::findMainLua(const std::vector<std::string>& args) noexcept
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

	void LuaRunnerAppImpl::addPackagePath(const std::string& path) noexcept
	{
		auto& lua = *_lua;
		std::string current = lua["package"]["path"];
		auto pathPattern = std::filesystem::path(path) / std::filesystem::path("?.lua");
		current += (!current.empty() ? ";" : "") + std::filesystem::absolute(pathPattern).string();
		lua["package"]["path"] = current;
	}

	LuaRunnerAppImpl::~LuaRunnerAppImpl() noexcept
	{
		_luaApp.reset();
		_lua.reset();
	}

	void LuaRunnerAppImpl::init(App& app, const std::vector<std::string>& args)
	{
		auto mainFile = findMainLua(args);
		auto mainDir = std::filesystem::path(mainFile).parent_path().string();

		_lua = std::make_unique<sol::state>();
		auto& lua = *_lua;
		lua.open_libraries(sol::lib::base, sol::lib::package);
		
		LuaMath::bind(lua);
		LuaShape::bind(lua);
		LuaAssets::bind(lua);
		LuaScene::bind(lua);
		LuaWindow::bind(lua);
		LuaInput::bind(lua);
		LuaApp::bind(lua);
		LuaRmluiAppComponent::bind(lua);

		_luaApp.emplace(app);
		lua["app"] = std::ref(_luaApp.value());
		lua["args"] = args;

		if (!mainDir.empty())
		{
			addPackagePath(mainDir);
		}
		
		auto result = lua.script_file(mainFile);
		if (!result.valid())
		{
			recoveredLuaError("running main", result);
		}
		else
		{
			_luaApp->registerUpdate(lua["update"]);
		}
	}

	void LuaRunnerAppImpl::updateLogic(float deltaTime)
	{
		if (_luaApp)
		{
			_luaApp->update(deltaTime);
		}
	}

	void LuaRunnerAppImpl::beforeShutdown() noexcept
	{
		_luaApp.reset();
	}

	void LuaRunnerAppImpl::afterShutdown() noexcept
	{
		_lua.reset();
	}
}