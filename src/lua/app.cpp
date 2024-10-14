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
#include "render_chain.hpp"

#include "generated/lua/string.h"
#include "generated/lua/table.h"
#include "generated/lua/middleclass.h"
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

	void LuaApp::quit() noexcept
	{
		_app.get().quit();
	}

	bool LuaApp::toggleDebugFlag(uint32_t flag) noexcept
	{
		return _app.get().toggleDebugFlag(flag);
	}

	void LuaApp::setDebugFlag1(uint32_t flag) noexcept
	{
		return _app.get().setDebugFlag(flag);
	}

	void LuaApp::setDebugFlag2(uint32_t flag, bool enabled) noexcept
	{
		return _app.get().setDebugFlag(flag, enabled);
	}

	bool LuaApp::getDebugFlag(uint32_t flag) const noexcept
	{
		return _app.get().getDebugFlag(flag);
	}

	bool LuaApp::toggleResetFlag(uint32_t flag) noexcept
	{
		return _app.get().toggleResetFlag(flag);
	}

	bool LuaApp::getResetFlag(uint32_t flag) const noexcept
	{
		return _app.get().getResetFlag(flag);
	}

	void LuaApp::setResetFlag1(uint32_t flag) noexcept
	{
		return _app.get().setResetFlag(flag);
	}

	void LuaApp::setResetFlag2(uint32_t flag, bool enabled) noexcept
	{
		return _app.get().setResetFlag(flag, enabled);
	}

	void LuaApp::setRendererType(bgfx::RendererType::Enum renderer)
	{
		return _app.get().setRendererType(renderer);
	}

	AssetContext& LuaApp::getAssets() noexcept
	{
		return _app.get().getAssets();
	}

	Window& LuaApp::getWindow() noexcept
	{
		return _app.get().getWindow();
	}

	Input& LuaApp::getInput() noexcept
	{
		return _app.get().getInput();
	}

	LuaAudioSystem& LuaApp::getAudio() noexcept
	{
		return _audio;
	}

	LuaApp& LuaApp::addUpdater(const sol::object& updater) noexcept
	{
		LuaDelegate dlg(updater, "update");
		if (dlg)
		{
			_updaters.push_back(std::move(dlg));
		}
		return *this;
	}

	bool LuaApp::removeUpdater(const sol::object& updater) noexcept
	{
		auto itr = std::remove(_updaters.begin(), _updaters.end(), updater);
		if (itr == _updaters.end())
		{
			return false;
		}
		_updaters.erase(itr, _updaters.end());
		return true;
	}

	bool LuaApp::removeUpdater3(const LuaDelegate& dlg) noexcept
	{
		auto itr = std::remove(_updaters.begin(), _updaters.end(), dlg);
		if (itr == _updaters.end())
		{
			return false;
		}
		_updaters.erase(itr, _updaters.end());
		return true;
	}

	LuaCoroutine LuaApp::startCoroutine(const sol::function& func) noexcept
	{
		return _coroutineRunner.startCoroutine(func);
	}

	bool LuaApp::stopCoroutine(const LuaCoroutine& coroutine) noexcept
	{
		return _coroutineRunner.stopCoroutine(coroutine);
	}

	bool LuaApp::removeComponent(const sol::object& type)
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return getReal().removeComponent(typeId.value());
		}
		return false;
	}

	bool LuaApp::hasComponent(const sol::object& type) const
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return getReal().hasComponent(typeId.value());
		}
		return false;
	}

	void LuaApp::addLuaComponent(const sol::table& table)
	{
		auto comp = std::make_unique<LuaAppComponent>(table, *this);
		auto typeId = comp->getType();
		_app.get().addComponent(typeId, std::move(comp));
	}

	sol::object LuaApp::getLuaComponent(const sol::object& type) noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto comp = _app.get().getComponent(typeId.value()))
			{
				auto& luaComp = static_cast<LuaAppComponent&>(*comp);
				return luaComp.getReal();
			}
		}
		return sol::nil;
	}

	void LuaApp::update(float deltaTime, sol::state_view& lua) noexcept
	{
		updateUpdaters(deltaTime);
		_coroutineRunner.update(deltaTime);
	}

	void LuaApp::updateUpdaters(float deltaTime) noexcept
	{
		auto updaters = _updaters;
		for (auto& updater : updaters)
		{
			auto result = updater(deltaTime);
			if (auto finished = LuaUtils::checkResult("running updater", result))
			{
				removeUpdater3(updater);
			}
		}
	}	

	bool LuaApp::getDebug() noexcept
	{
#if _DEBUG
		return true;
#else
		return false;
#endif
	}

	bool LuaApp::getPaused() const noexcept
	{
		return _app.get().isPaused();
	}

	void LuaApp::setPaused(bool paused) noexcept
	{
		_app.get().setPaused(paused);
	}

	void LuaApp::bind(sol::state_view& lua) noexcept
	{
		LuaAssets::bind(lua);
		LuaWindow::bind(lua);
		LuaInput::bind(lua);
		LuaAudioSystem::bind(lua);
		LuaCoroutineRunner::bind(lua);
		LuaRenderChain::bind(lua);
		LuaTypeFilter::bind(lua);

		lua.create_named_table("DebugFlag",
			"NONE", BGFX_DEBUG_NONE,
			"WIREFRAME", BGFX_DEBUG_WIREFRAME,
			"IFH", BGFX_DEBUG_IFH,
			"STATS", BGFX_DEBUG_STATS,
			"TEXT", BGFX_DEBUG_TEXT,
			"PROFILER", BGFX_DEBUG_PROFILER
		);

		lua.create_named_table("ResetFlag",
			"NONE", BGFX_RESET_NONE,
			"FULLSCREEN", BGFX_RESET_FULLSCREEN,
			"VSYNC", BGFX_RESET_VSYNC,
			"MAXANISOTROPY", BGFX_RESET_MAXANISOTROPY,
			"CAPTURE", BGFX_RESET_CAPTURE,
			"FLUSH_AFTER_RENDER", BGFX_RESET_FLUSH_AFTER_RENDER,
			"FLIP_AFTER_RENDER", BGFX_RESET_FLIP_AFTER_RENDER,
			"SRGB_BACKBUFFER", BGFX_RESET_SRGB_BACKBUFFER,
			"HDR10", BGFX_RESET_HDR10,
			"HIDPI", BGFX_RESET_HIDPI,
			"DEPTH_CLAMP", BGFX_RESET_DEPTH_CLAMP,
			"SUSPEND", BGFX_RESET_SUSPEND,
			"TRANSPARENT_BACKBUFFER", BGFX_RESET_TRANSPARENT_BACKBUFFER
		);

		lua.new_enum<bgfx::RendererType::Enum>("RendererType", {
			{ "Direct3D11", bgfx::RendererType::Direct3D11 },
			{ "Direct3D12", bgfx::RendererType::Direct3D12 },
			{ "OpenGL", bgfx::RendererType::OpenGL },
			{ "OpenGLES", bgfx::RendererType::OpenGLES },
			{ "Vulkan", bgfx::RendererType::Vulkan },
			{ "Metal", bgfx::RendererType::Metal },
		});

		lua.new_usertype<LuaApp>("App", sol::no_constructor,
			"assets", sol::property(&LuaApp::getAssets),
			"window", sol::property(&LuaApp::getWindow),
			"input", sol::property(&LuaApp::getInput),
			"audio", sol::property(&LuaApp::getAudio),
			"debug", sol::property(&LuaApp::getDebug),
			"paused", sol::property(&LuaApp::getPaused, &LuaApp::setPaused),
			"add_updater", &LuaApp::addUpdater,
			"remove_updater", &LuaApp::removeUpdater,
			"start_coroutine", &LuaApp::startCoroutine,
			"stop_coroutine", &LuaApp::stopCoroutine,
			"has_component", &LuaApp::hasComponent,
			"remove_component", &LuaApp::removeComponent,
			"add_lua_component", &LuaApp::addLuaComponent,
			"get_lua_component", &LuaApp::getLuaComponent,
			"get_debug_flag", &LuaApp::getDebugFlag,
			"toggle_debug_flag", &LuaApp::toggleDebugFlag,
			"set_debug_flag", sol::overload(&LuaApp::setDebugFlag1, &LuaApp::setDebugFlag2),
			"get_reset_flag", &LuaApp::getResetFlag,
			"toggle_reset_flag", &LuaApp::toggleResetFlag,
			"set_debug_flag", sol::overload(&LuaApp::setResetFlag1, &LuaApp::setResetFlag2),
			"renderer_type", sol::property(&LuaApp::setRendererType),
			"quit", &LuaApp::quit
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

	LuaAppDelegate::LuaAppDelegate(App& app) noexcept
		: _impl(std::make_unique<LuaAppDelegateImpl>(app))
	{
	}

	LuaAppDelegate::~LuaAppDelegate() noexcept
	{
		// intentionally left blank for the unique_ptr<LuaAppDelegateImpl> forward declaration
	}

	std::optional<int32_t> LuaAppDelegate::setup(const std::vector<std::string>& args)
	{
		return _impl->setup(args);
	}

	void LuaAppDelegate::init()
	{
		_impl->init();
	}

	void LuaAppDelegate::earlyShutdown()
	{
		_impl->earlyShutdown();
	}

	void LuaAppDelegate::shutdown()
	{
		_impl->shutdown();
	}

	void LuaAppDelegate::update(float deltaTime)
	{
		_impl->update(deltaTime);
	}

	void LuaAppDelegate::render() const
	{
		_impl->render();
	}

	LuaAppDelegateImpl::LuaAppDelegateImpl(App& app) noexcept
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

	void LuaAppDelegateImpl::luaDebugScreenText(const glm::uvec2& pos, const std::string& msg) noexcept
	{
		_dbgTexts.emplace_back(pos, msg);
	}

	std::optional<int> LuaAppDelegateImpl::setup(const std::vector<std::string>& args)
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

	std::optional<int32_t> LuaAppDelegateImpl::loadLua(const std::filesystem::path& mainPath)
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

		auto addStaticLib = [&lua](auto& lib, const std::string& name, bool require = false)
		{
			auto content = DataView::fromStatic(lib).stringView();
			if (require)
			{
				lua.require_script(name, content, true, name);
			}
			else
			{
				lua.script(content, name);
			}
		};

		LuaMath::bind(lua);
		LuaShape::bind(lua);
		LuaScene::bind(lua);
		LuaApp::bind(lua);

#ifdef DARMOK_RMLUI
		LuaRmluiRenderer::bind(lua);
#ifdef RMLUI_DEBUGGER
		LuaRmluiDebuggerComponent::bind(lua);
#endif
#endif

		addStaticLib(lua_darmok_lib_middleclass, "darmok/middleclass", true);

		addStaticLib(lua_darmok_lib_table, "darmok/table");
		addStaticLib(lua_darmok_lib_string, "darmok/string");
		addStaticLib(lua_darmok_lib_base, "darmok/base");

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

	void LuaAppDelegateImpl::unloadLua() noexcept
	{
		_luaApp.reset();
		_lua.reset();
	}

	std::optional<std::filesystem::path> LuaAppDelegateImpl::findMainLua(const std::string& cmdName, const bx::CommandLine& cmdLine) noexcept
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

	void LuaAppDelegateImpl::addPackagePath(const std::string& path, bool binary) noexcept
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

	void LuaAppDelegateImpl::init()
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
		_luaApp->addUpdater(lua["update"]);
	}

	std::string LuaAppDelegateImpl::_defaultAssetInputPath = "assets";
	std::string LuaAppDelegateImpl::_defaultAssetOutputPath = "runtime_assets";
	std::string LuaAppDelegateImpl::_defaultAssetCachePath = "asset_cache";

	bool LuaAppDelegateImpl::importAssets(const std::string& cmdName, const bx::CommandLine& cmdLine)
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
			_app.getAssets().addBasePath(outputPath);
		}

		return true;
	}

	void LuaAppDelegateImpl::version(const std::string& name) noexcept
	{
		std::cout << name << ": darmok lua runner." << std::endl;
	}

	void LuaAppDelegateImpl::help(const std::string& name, const char* error) noexcept
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

	void LuaAppDelegateImpl::update(float deltaTime)
	{
		if (_luaApp)
		{
			_luaApp->update(deltaTime, *_lua);
		}
	}

	void LuaAppDelegateImpl::render() noexcept
	{
		for (auto& text : _dbgTexts)
		{
			bgfx::dbgTextPrintf(text.pos.x, text.pos.y, 0x0f, text.message.c_str());
		}
		_dbgTexts.clear();
	}

	void LuaAppDelegateImpl::earlyShutdown()
	{
		if (!_lua)
		{
			return;
		}
		sol::safe_function shutdown = (*_lua)["shutdown"];
		if (shutdown)
		{
			auto result = shutdown();
			if (!result.valid())
			{
				LuaUtils::logError("running shutown", result);
				throw LuaError("running shutown", result);
			}
		}
	}

	void LuaAppDelegateImpl::shutdown() noexcept
	{
		unloadLua();
	}

	LuaAppComponent::LuaAppComponent(const sol::table& table, LuaApp& app) noexcept
		: _table(table)
		, _app(app)
	{
	}

	entt::id_type LuaAppComponent::getType() const noexcept
	{
		return LuaUtils::getTypeId(_table).value();
	}

	sol::object LuaAppComponent::getReal() const noexcept
	{
		return _table;
	}

	const LuaTableDelegateDefinition LuaAppComponent::_initDef("init", "app component init");
	const LuaTableDelegateDefinition LuaAppComponent::_shutdownDef("shutdown", "app component shutdown");
	const LuaTableDelegateDefinition LuaAppComponent::_renderResetDef("render_reset", "app component render reset");
	const LuaTableDelegateDefinition LuaAppComponent::_updateDef("update", "app component update");

	void LuaAppComponent::init(App& app)
	{
		_initDef(_table, _app);
	}

	void LuaAppComponent::shutdown()
	{
		_shutdownDef(_table);
	}

	void LuaAppComponent::renderReset()
	{
		_renderResetDef(_table);
	}

	void LuaAppComponent::update(float deltaTime)
	{
		_updateDef(_table, deltaTime);
	}
}