#include "app.hpp"
#include <darmok/app.hpp>
#include <darmok/lua.hpp>
#include <darmok/scene.hpp>
#include <darmok/string.hpp>
#include <darmok/asset.hpp>
#include <darmok/stream.hpp>
#include <darmok/data.hpp>
#include <filesystem>
#include <bx/commandline.h>
#include "asset.hpp"
#include "input.hpp"
#include "math.hpp"
#include "shape.hpp"
#include "scene.hpp"
#include "window.hpp"
#include "texture.hpp"
#include "skeleton.hpp"
#include "utils.hpp"
#include "component.hpp"


#include "generated/lua/string.h"
#include "generated/lua/table.h"
#include "generated/lua/class.h"
#include "generated/lua/base.h"

#ifdef DARMOK_RMLUI
#include <darmok/rmlui.hpp>
#include "rmlui.hpp"
#ifdef _DEBUG
#define RMLUI_DEBUGGER
#include "rmlui_debug.hpp"
#endif
#endif

namespace darmok
{
    LuaApp::LuaApp(App& app) noexcept
		: _app(app)
		, _input(app.getInput())
		, _win(app.getWindow())
		, _audio(app.getAudio())
	{
	}

	App& LuaApp::getReal() noexcept
	{
		return _app.get();
	}

	const App& LuaApp::getReal() const noexcept
	{
		return _app.get();
	}

	AssetContext& LuaApp::getAssets() noexcept
	{
		return _app.get().getAssets();
	}

	LuaWindow& LuaApp::getWindow() noexcept
	{
		return _win;
	}

	LuaInput& LuaApp::getInput() noexcept
	{
		return _input;
	}

	LuaAudioSystem& LuaApp::getAudio() noexcept
	{
		return _audio;
	}

	LuaApp& LuaApp::addUpdater1(const sol::protected_function& func) noexcept
	{
		if (func)
		{
			_updaterFunctions.push_back(func);
		}
		return *this;
	}

	LuaApp& LuaApp::addUpdater2(const sol::table& table) noexcept
	{
		_updaterTables.push_back(table);
		return *this;
	}

	bool LuaApp::removeUpdater1(const sol::protected_function& func) noexcept
	{
		auto itr = std::find(_updaterFunctions.begin(), _updaterFunctions.end(), func);
		if (itr == _updaterFunctions.end())
		{
			return false;
		}
		_updaterFunctions.erase(itr);
		return true;
	}

	bool LuaApp::removeUpdater2(const sol::table& table) noexcept
	{
		auto itr = std::find(_updaterTables.begin(), _updaterTables.end(), table);
		if (itr == _updaterTables.end())
		{
			return false;
		}
		_updaterTables.erase(itr);
		return true;
	}

	bool LuaApp::removeComponent(const sol::object& type)
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return getReal().removeComponent(typeId.value());
		}
		return removeLuaComponent(type);
	}

	bool LuaApp::hasComponent(const sol::object& type) const
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return getReal().hasComponent(typeId.value());
		}
		return hasLuaComponent(type);
	}

	bool LuaApp::hasLuaComponent(const sol::object& type) const noexcept
	{
		return _luaComponents && _luaComponents->contains(type);
	}

	void LuaApp::addLuaComponent(const sol::table& comp)
	{
		if (!_luaComponents)
		{
			_luaComponents = getReal().addComponent<LuaAppComponentContainer>();
		}
		_luaComponents->add(comp);
	}

	bool LuaApp::removeLuaComponent(const sol::object& type) noexcept
	{
		if (!_luaComponents)
		{
			return false;
		}
		return _luaComponents->remove(type);
	}

	sol::object LuaApp::getLuaComponent(const sol::object& type) noexcept
	{
		if (!_luaComponents)
		{
			return sol::nil;
		}
		return _luaComponents->get(type);
	}

	void LuaApp::update(float deltaTime) noexcept
	{
		for(auto& func : _updaterFunctions)
		{
			auto result = func(deltaTime);
			LuaUtils::checkResult("running function updater", result);
		}
		LuaUtils::callTableDelegates(_updaterTables, "update", "running table updater",
			[deltaTime](auto& func, auto& self) {
				return func(self, deltaTime);
			});
	}

	bool LuaApp::getDebug() noexcept
	{
#if _DEBUG
		return true;
#else
		return false;
#endif
	}

	void LuaApp::bind(sol::state_view& lua) noexcept
	{
		LuaAssets::bind(lua);
		LuaWindow::bind(lua);
		LuaInput::bind(lua);
		LuaAudioSystem::bind(lua);

		lua.new_usertype<LuaApp>("App", sol::no_constructor,
			"assets", sol::property(&LuaApp::getAssets),
			"window", sol::property(&LuaApp::getWindow),
			"input", sol::property(&LuaApp::getInput),
			"audio", sol::property(&LuaApp::getAudio),
			"debug", sol::property(&LuaApp::getDebug),
			"add_updater", sol::overload(&LuaApp::addUpdater1, &LuaApp::addUpdater2),
			"remove_updater", sol::overload(&LuaApp::removeUpdater1, &LuaApp::removeUpdater2),
			"has_component", &LuaApp::hasComponent,
			"remove_component", &LuaApp::removeComponent,
			"add_lua_component", &LuaApp::addLuaComponent,
			"get_lua_component", &LuaApp::getLuaComponent
		);
	}

	LuaError::LuaError(const std::string& msg, const sol::error& error)
		: _msg(msg)
		, error(error)
	{
	}

	const char* LuaError::what() const noexcept
	{
		return _msg.c_str();
	}

	LuaRunnerApp::LuaRunnerApp() noexcept
		: _impl(std::make_unique<LuaRunnerAppImpl>(*this))
	{
	}

	LuaRunnerApp::~LuaRunnerApp() noexcept
	{
		// intentionally left blank for the unique_ptr<LuaRunnerAppImpl> forward declaration
	}

	std::optional<int32_t> LuaRunnerApp::setup(const std::vector<std::string>& args)
	{
		return _impl->setup(args);
	}

	void LuaRunnerApp::init()
	{
		App::init();
		return _impl->init();
	}

	void LuaRunnerApp::shutdown()
	{
		App::shutdown();
		_impl->shutdown();
	}

	void LuaRunnerApp::update(float deltaTime)
	{
		App::update(deltaTime);
		_impl->update(deltaTime);
	}

	void LuaRunnerApp::render() const
	{
		App::render();
		_impl->render();
	}

	LuaRunnerAppImpl::LuaRunnerAppImpl(App& app) noexcept
		: _app(app)
	{
	}

	static void luaPrint(sol::variadic_args args) noexcept
	{
		std::ostringstream oss;
		for (auto arg : args)
		{
			oss << arg.as<std::string>() << " ";
		}
		StreamUtils::logDebug(oss.str());
	}

	void LuaRunnerAppImpl::luaDebugScreenText(const glm::uvec2& pos, const std::string& msg) noexcept
	{
		_dbgTexts.emplace_back(pos, msg);
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

		if (!importAssets(cmdName, cmdLine))
		{
			return -3;
		}
		auto mainPath = findMainLua(cmdName, cmdLine);
		if (!mainPath)
		{
			return -2;
		}
		_mainLuaPath = mainPath.value();
		auto result = loadLua(_mainLuaPath);
		if (result)
		{
			unloadLua();
		}
		return result;
	}

	std::optional<int32_t> LuaRunnerAppImpl::loadLua(const std::filesystem::path& mainPath)
	{
		auto mainDir = mainPath.parent_path().string();

		_lua = std::make_unique<sol::state>();
		auto& lua = *_lua;
		lua.open_libraries(
			sol::lib::base, sol::lib::package, sol::lib::io,
			sol::lib::table, sol::lib::string, sol::lib::coroutine,
			sol::lib::math, sol::lib::os
		);
#ifndef _NDEBUG
		lua.open_libraries(sol::lib::debug);
#endif

		auto addStaticLib = [&lua](auto& lib, const std::string& name)
		{
			lua.script(DataView::fromStatic(lib).stringView(), name);
		};

		LuaMath::bind(lua);
		LuaShape::bind(lua);
		LuaScene::bind(lua);
		LuaApp::bind(lua);

#ifdef DARMOK_RMLUI
		LuaRmluiAppComponent::bind(lua);
#ifdef RMLUI_DEBUGGER
		LuaRmluiDebuggerAppComponent::bind(lua);
#endif
#endif

		addStaticLib(lua_darmok_lib_table, "darmok/table.lua");
		addStaticLib(lua_darmok_lib_string, "darmok/string.lua");
		addStaticLib(lua_darmok_lib_class, "darmok/class.lua");
		addStaticLib(lua_darmok_lib_base, "darmok/base.lua");

		_luaApp.emplace(_app);
		lua["app"] = std::ref(_luaApp.value());
		lua.set_function("print", luaPrint);
		lua.set_function("debug_screen_text", [this](const VarLuaTable<glm::uvec2>& pos, const std::string& msg) {
			luaDebugScreenText(LuaGlm::tableGet(pos), msg);
		});

		if (!mainDir.empty())
		{
			addPackagePath(mainDir);
		}

#define XSTR(V) #V
#define STR(V) XSTR(V)
#ifdef LUA_PATH
		addPackagePath(STR(LUA_PATH));
#endif

#ifdef LUA_CPATH
		addPackagePath(STR(LUA_CPATH), true);
#endif

		auto result = lua.script_file(mainPath.string());
		if (!result.valid())
		{
			LuaUtils::logError("running main", result);
			throw LuaError("running lua main", result);
		}
		auto relm = result[0];
		if (relm.is<int>())
		{
			return relm.get<int>();
		}
		return std::nullopt;
	}

	void LuaRunnerAppImpl::unloadLua() noexcept
	{
		_luaApp.reset();
		_lua.reset();
	}

	std::optional<std::filesystem::path> LuaRunnerAppImpl::findMainLua(const std::string& cmdName, const bx::CommandLine& cmdLine) noexcept
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
			mainPath = mainPathArg;
			if (std::filesystem::is_directory(mainPath))
			{
				for (std::filesystem::path path : possiblePaths)
				{
					path = mainPath / path;
					if (std::filesystem::exists(path))
					{
						return path;
					}
				}
			}
			if (std::filesystem::exists(mainPath))
			{
				return mainPath;
			}
			help(cmdName, "could not find specified main lua script");
			return std::nullopt;
		}
		for (auto& path : possiblePaths)
		{
			if (std::filesystem::exists(path))
			{
				return path;
			}
		}
		help(cmdName, "could not find main lua script");
		return std::nullopt;
	}

	void LuaRunnerAppImpl::addPackagePath(const std::string& path, bool binary) noexcept
	{
		static const char sep = ';';
		std::string fpath(path);
		std::replace(fpath.begin(), fpath.end(), ',', sep);
		auto& lua = *_lua;
		auto key = binary ? "cpath" : "path";
		std::string current = lua["package"][key];

		for (auto& sub : StringUtils::split(fpath, sep))
		{
			std::filesystem::path spath(sub);
			if (!spath.filename().string().contains('?') && spath.extension().empty())
			{
				if (binary)
				{
#if BX_PLATFORM_WINDOWS
					spath /= "?.dll";
#else
					spath /= "?.so";
#endif
				}
				else
				{
					spath /= "?.lua";
				}
			}
			current += (!current.empty() ? ";" : "") + std::filesystem::absolute(spath).string();
		}
		lua["package"][key] = current;
	}

	void LuaRunnerAppImpl::init()
	{
		if (_lua == nullptr)
		{
			auto r = loadLua(_mainLuaPath);
		}
		auto& lua = *_lua;
		sol::safe_function init = lua["init"];
		if (init)
		{
			auto result = init();
			if (!result.valid())
			{
				LuaUtils::logError("running init", result);
				throw LuaError("running init", result);
			}
		}
		_luaApp->addUpdater1(lua["update"]);
	}

	std::string LuaRunnerAppImpl::_defaultAssetInputPath = "assets";
	std::string LuaRunnerAppImpl::_defaultAssetOutputPath = "runtime_assets";
	std::string LuaRunnerAppImpl::_defaultAssetCachePath = "asset_cache";

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
			return true;
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
		if (cachePath == nullptr && (cmdLine.hasArg('c', "asset-cache") || std::filesystem::is_directory(_defaultAssetCachePath)))
		{
			cachePath = _defaultAssetCachePath.c_str();
		}
		if (cachePath != nullptr)
		{
			importer.setCachePath(cachePath);
		}

		if (cmdLine.hasArg('d', "dry"))
		{
			for (auto& output : importer.getOutputs())
			{
				std::cout << output.string() << std::endl;
			}
			return false;
		}

		importer(std::cout);

		if (std::filesystem::is_directory(outputPath))
		{
			_app.getAssets().setBasePath(outputPath);
		}

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
		std::cout << "  -d, --dry                   Do not process assets, just print output files." << std::endl;
		std::cout << "  --bgfx-shaderc              Path of the bgfx shaderc executable (used to process bgfx shaders)." << std::endl;
		std::cout << "  --bgfx-shader-include       Path of the bgfx shader include dir (used to process bgfx shaders)." << std::endl;
	}

	void LuaRunnerAppImpl::update(float deltaTime)
	{
		if (_luaApp)
		{
			_luaApp->update(deltaTime);
		}
	}

	void LuaRunnerAppImpl::render() noexcept
	{
		for (auto& text : _dbgTexts)
		{
			bgfx::dbgTextPrintf(text.pos.x, text.pos.y, 0x0f, text.message.c_str());
		}
		_dbgTexts.clear();
	}

	void LuaRunnerAppImpl::shutdown() noexcept
	{
		unloadLua();
	}
}