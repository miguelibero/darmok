#include "app.hpp"
#include <darmok/app.hpp>
#include <darmok/lua.hpp>
#include <darmok/scene.hpp>
#include <darmok/rmlui.hpp>
#include <darmok/string.hpp>
#include <darmok/asset.hpp>
#include <filesystem>
#include <bx/commandline.h>
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
#include "physics3d.hpp"

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
		lua.script(R"(
function App:add_component(type, ...)
	return type.add_app_component(self, ...)
end
function App:get_shared_component(type)
	return type.get_shared_component(self)
end
)");
	}

	LuaRunnerApp::LuaRunnerApp() noexcept
		: _impl(std::make_unique<LuaRunnerAppImpl>())
	{
	}

	LuaRunnerApp::~LuaRunnerApp() noexcept
	{
		// intentionally left blank for the unique_ptr<LuaRunnerAppImpl> forward declaration
	}

	std::optional<int> LuaRunnerApp::setup(const std::vector<std::string>& args)
	{
		return _impl->setup(args);
	}

	void LuaRunnerApp::init()
	{
		App::init();
		return _impl->init(*this);
	}

	void LuaRunnerApp::shutdown()
	{
		_impl->beforeShutdown();
		App::shutdown();
		_impl->afterShutdown();
	}

	void LuaRunnerApp::updateLogic(float deltaTime)
	{
		App::updateLogic(deltaTime);
		_impl->updateLogic(deltaTime);
	}

	LuaRunnerAppImpl::~LuaRunnerAppImpl() noexcept
	{
		_luaApp.reset();
		_lua.reset();
	}

	std::optional<int> LuaRunnerAppImpl::setup(const std::vector<std::string>& args)
	{
		std::vector<const char*> argv;
		argv.reserve(args.size());
		for (auto& arg : args)
		{
			argv.push_back(arg.c_str());
		}
		bx::CommandLine cmdLine(args.size(), &argv.front());
		auto cmdPath = std::string(cmdLine.get(0));
		auto cmdName = std::filesystem::path(cmdPath).filename().string();
		if (cmdLine.hasArg('h', "help"))
		{
			help(cmdName);
			return 0;
		}

		if (cmdLine.hasArg('v', "version"))
		{
			version(cmdName);
			return 0;
		}

		auto importedAssets = importAssets(cmdName, cmdLine);
		auto mainLuaFound = findMainLua(cmdName, cmdLine);

		if (!importedAssets && !mainLuaFound)
		{
			help(cmdName, "could not find main lua script");
			return -1;
		}

		return std::nullopt;
	}

	bool LuaRunnerAppImpl::findMainLua(const std::string& cmdName, const bx::CommandLine& cmdLine) noexcept
	{
		static const std::vector<std::filesystem::path> possiblePaths = {
			"main.lua",
			"lua/main.lua",
			"assets/main.lua"
		};

		std::string mainPath;
		const char* mainPathArg;
		if (cmdLine.hasArg(mainPathArg, 'm', "main-lua"))
		{
			_mainLua = mainPathArg;
			if (!std::filesystem::exists(_mainLua))
			{
				help(cmdName, "specified main lua script does not exist");
				return false;
			}
			return true;
		}
		else
		{
			for (auto& path : possiblePaths)
			{
				if (std::filesystem::exists(path))
				{
					_mainLua = path;
					return true;
				}
			}
		}
		return false;
	}

	void LuaRunnerAppImpl::addPackagePath(const std::string& path) noexcept
	{
		auto& lua = *_lua;
		std::string current = lua["package"]["path"];
		auto pathPattern = std::filesystem::path(path) / std::filesystem::path("?.lua");
		current += (!current.empty() ? ";" : "") + std::filesystem::absolute(pathPattern).string();
		lua["package"]["path"] = current;
	}	

	void LuaRunnerAppImpl::init(App& app)
	{
		auto mainDir = _mainLua.parent_path().string();

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

		if (!mainDir.empty())
		{
			addPackagePath(mainDir);
		}

		auto result = lua.script_file(_mainLua.string());
		if (!result.valid())
		{
			recoveredLuaError("running main", result);
		}
		else
		{
			_luaApp->registerUpdate(lua["update"]);
		}

	}

	std::string LuaRunnerAppImpl::_defaultAssetInputPath = "asset_sources";
	std::string LuaRunnerAppImpl::_defaultAssetOutputPath = "assets";

	bool LuaRunnerAppImpl::importAssets(const std::string& cmdName, const bx::CommandLine& cmdLine)
	{
		const char* inputPath = nullptr;
		cmdLine.hasArg(inputPath, 'i', "asset-input");
		if (inputPath == nullptr && std::filesystem::exists(_defaultAssetInputPath))
		{
			inputPath = _defaultAssetInputPath.c_str();
		}
		if (inputPath == nullptr)
		{
			return false;
		}
		const char* outputPath = nullptr;
		cmdLine.hasArg(outputPath, 'o', "asset-output");
		if (outputPath == nullptr)
		{
			outputPath = _defaultAssetOutputPath.c_str();
		}
		DarmokAssetImporter importer(inputPath);
		importer.setOutputPath(outputPath);
		const char* cachePath = nullptr;
		cmdLine.hasArg(cachePath, 'c', "asset-cache");
		if (cachePath != nullptr)
		{
			importer.setCachePath(cachePath);
		}
		importer(std::cout);
		return true;
	}

	void LuaRunnerAppImpl::version(const std::string& name) noexcept
	{
		std::cout << name << ": darmok lua runner." << std::endl;
	}

	void LuaRunnerAppImpl::help(const std::string& name, const char* error) noexcept
	{
		if (error)
		{
			std::cerr << "Error:" << std::endl << error << std::endl << std::endl;
		}
		version(name);
		std::cout << "Usage: " << name << " -m <main lua> -i <in> -o <out>" << std::endl;
		std::cout << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "  -h, --help                  this help and exit." << std::endl;
		std::cout << "  -v, --version               Output version information and exit." << std::endl;
		std::cout << "  -m, --main-lua <path>       Path to the main lua file (dir will be taken as package path)." << std::endl;
		std::cout << "  -i, --asset-input <path>    Asset input file path." << std::endl;
		std::cout << "  -o, --asset-output <path>   Asset output file path." << std::endl;
		std::cout << "  -c, --asset-cache <path>    Asset cache file path (directory that keeps the timestamps of the inputs)." << std::endl;
		std::cout << "  --bgfx-shaderc              Path of the bgfx shaderc executable (used to process bgfx shaders)." << std::endl;
		std::cout << "  --bgfx-shader-include       Path of the bgfx shader include dir (used to process bgfx shaders)." << std::endl;
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