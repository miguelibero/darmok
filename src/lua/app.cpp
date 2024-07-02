#include "app.hpp"
#include <darmok/app.hpp>
#include <darmok/lua.hpp>
#include <darmok/scene.hpp>
#include <darmok/rmlui.hpp>
#include <darmok/string.hpp>
#include <darmok/asset.hpp>
#include <darmok/stream.hpp>
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

	std::optional<int32_t> LuaRunnerApp::setup(const std::vector<std::string>& args)
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

		auto r = importAssets(cmdName, cmdLine);
		if (r)
		{
			return r.value();
		}
		return findMainLua(cmdName, cmdLine);
	}

	std::optional<int32_t> LuaRunnerAppImpl::findMainLua(const std::string& cmdName, const bx::CommandLine& cmdLine) noexcept
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
			if (std::filesystem::is_directory(_mainLua))
			{
				for (auto path : possiblePaths)
				{
					path = _mainLua / path;
					if (std::filesystem::exists(path))
					{
						_mainLua = path;
						return std::nullopt;
					}
				}
			}
			if (std::filesystem::exists(_mainLua))
			{
				return std::nullopt;
			}
			help(cmdName, "could not find specified main lua script");
			return -1;
		}
		for (auto& path : possiblePaths)
		{
			if (std::filesystem::exists(path))
			{
				_mainLua = path;
				return std::nullopt;
			}
		}
		help(cmdName, "could not find main lua script");
		return -1;
	}

	void LuaRunnerAppImpl::addPackagePath(const std::string& path, bool binary) noexcept
	{
		static const char sep = ';';
		std::string fpath(path);
		std::replace(fpath.begin(), fpath.end(), ',', sep);
		auto& lua = *_lua;
		auto key = binary ? "cpath" : "path";
		std::string current = lua["package"][key];

		for (auto sub : StringUtils::split(fpath, sep))
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

	static void luaPrint(sol::variadic_args args) noexcept
	{
		std::ostringstream oss;
		for (auto arg : args)
		{
			oss << arg.as<std::string>() << " ";
		}
		StreamUtils::logDebug(oss.str());
	}

	void LuaRunnerAppImpl::init(App& app)
	{
		auto mainDir = _mainLua.parent_path().string();

		_lua = std::make_unique<sol::state>();
		auto& lua = *_lua;
		lua.open_libraries(
			sol::lib::base, sol::lib::package, sol::lib::io,
			sol::lib::table, sol::lib::string, sol::lib::coroutine,
			sol::lib::math
		);
#ifndef _NDEBUG
		lua.open_libraries(sol::lib::debug);
#endif

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
		lua.set_function("print", luaPrint);

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

	std::optional<int32_t> LuaRunnerAppImpl::importAssets(const std::string& cmdName, const bx::CommandLine& cmdLine)
	{
		const char* inputPath = nullptr;
		cmdLine.hasArg(inputPath, 'i', "asset-input");
		if (inputPath == nullptr && std::filesystem::exists(_defaultAssetInputPath))
		{
			inputPath = _defaultAssetInputPath.c_str();
		}
		if (inputPath == nullptr)
		{
			return std::nullopt;
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

		if (cmdLine.hasArg('d', "dry"))
		{
			for (auto& output : importer.getOutputs())
			{
				std::cout << output.string() << std::endl;
			}
			return 0;
		}

		importer(std::cout);
		return std::nullopt;
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