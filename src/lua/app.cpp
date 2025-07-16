#include "app.hpp"
#include <darmok/app.hpp>
#include <darmok/lua.hpp>
#include <darmok/scene.hpp>
#include <darmok/string.hpp>
#include <darmok/asset.hpp>
#include <darmok/audio.hpp>
#include <darmok/window.hpp>
#include <darmok/stream.hpp>
#include <darmok/data.hpp>

#include "asset.hpp"
#include "input.hpp"
#include "audio.hpp"
#include "scene.hpp"
#include "window.hpp"
#include "math.hpp"
#include "shape.hpp"
#include "texture.hpp"
#include "skeleton.hpp"
#include "render_chain.hpp"

#include "generated/lua/string.h"
#include "generated/lua/table.h"
#include "generated/lua/middleclass.h"
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

#include <filesystem>
#include <CLI/CLI.hpp>

namespace darmok
{	
	LuaAppUpdater::LuaAppUpdater(const sol::object& obj) noexcept
		: _delegate{ obj, "update" }
	{
	}

	void LuaAppUpdater::update(float deltaTime)
	{
		static const std::string desc = "running updater";
		LuaUtils::checkResult(desc, _delegate(deltaTime));
	}

	const LuaDelegate& LuaAppUpdater::getDelegate() const noexcept
	{
		return _delegate;
	}

	LuaAppUpdaterFilter::LuaAppUpdaterFilter(const sol::object& obj) noexcept
		: _object{ obj }
		, _type{ entt::type_hash<LuaAppUpdater>::value() }
	{
	}

	bool LuaAppUpdaterFilter::operator()(const IAppUpdater& updater) const noexcept
	{
		return updater.getAppUpdaterType() == _type && static_cast<const LuaAppUpdater&>(updater).getDelegate() == _object;
	}

	App& LuaApp::addUpdater(App& app, const sol::object& updater) noexcept
	{
		auto ptr = std::make_unique<LuaAppUpdater>(updater);
		if (ptr->getDelegate())
		{
			app.addUpdater(std::move(ptr));
		}
		return app;
	}

	bool LuaApp::removeUpdater(App& app, const sol::object& updater) noexcept
	{
		return app.removeUpdaters(LuaAppUpdaterFilter(updater)) > 0;
	}

	LuaCoroutine LuaApp::startCoroutine(App& app, const sol::function& func) noexcept
	{
		auto& runner = app.getOrAddComponent<LuaCoroutineRunner>();
		return runner.startCoroutine(func);
	}

	bool LuaApp::stopCoroutine(App& app, const LuaCoroutine& coroutine) noexcept
	{
		if (auto runner = app.getComponent<LuaCoroutineRunner>())
		{
			return runner->stopCoroutine(coroutine);
		}
		return false;
	}

	bool LuaApp::removeComponent(App& app, const sol::object& type)
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return app.removeComponent(typeId.value());
		}
		return false;
	}

	bool LuaApp::hasComponent(const App& app, const sol::object& type)
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return app.hasComponent(typeId.value());
		}
		return false;
	}

	void LuaApp::addLuaComponent(App& app, const sol::table& table)
	{
		app.addComponent(std::make_unique<LuaAppComponent>(table));
	}

	sol::object LuaApp::getLuaComponent(App& app, const sol::object& type) noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto comp = app.getComponent(typeId.value()))
			{
				auto& luaComp = static_cast<LuaAppComponent&>(*comp);
				return luaComp.getReal();
			}
		}
		return sol::nil;
	}

	bool LuaApp::getDebug() noexcept
	{
#if _DEBUG
		return true;
#else
		return false;
#endif
	}

	void LuaApp::setDebugFlag(App& app, uint32_t flag) noexcept
	{
		app.setDebugFlag(flag);
	}

	void LuaApp::setResetFlag(App& app, uint32_t flag) noexcept
	{
		app.setResetFlag(flag);
	}

	void LuaApp::bind(sol::state_view& lua) noexcept
	{
		LuaAssets::bind(lua);
		LuaWindow::bind(lua);
		LuaInput::bind(lua);
		LuaAudioSystem::bind(lua);
		LuaCoroutineRunner::bind(lua);
		LuaRenderChain::bind(lua);

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

		LuaUtils::newEnum<bgfx::RendererType::Enum>(lua, "RendererType");

		lua.new_usertype<App>("App", sol::no_constructor,
			"assets", sol::property(sol::resolve<AssetContext&()>(&App::getAssets)),
			"window", sol::property(sol::resolve<Window&()>(&App::getWindow)),
			"input", sol::property(sol::resolve<Input&()>(&App::getInput)),
			"audio", sol::property(sol::resolve<AudioSystem&()>(&App::getAudio)),
			"debug", sol::property(&LuaApp::getDebug),
			"paused", sol::property(&App::isPaused, &App::setPaused),
			"add_updater", &LuaApp::addUpdater,
			"remove_updater", &LuaApp::removeUpdater,
			"start_coroutine", &LuaApp::startCoroutine,
			"stop_coroutine", &LuaApp::stopCoroutine,
			"has_component", &LuaApp::hasComponent,
			"remove_component", &LuaApp::removeComponent,
			"add_lua_component", &LuaApp::addLuaComponent,
			"get_lua_component", &LuaApp::getLuaComponent,
			"get_debug_flag", &App::getDebugFlag,
			"toggle_debug_flag", &App::toggleDebugFlag,
			"set_debug_flag", sol::overload(&App::setDebugFlag, &LuaApp::setDebugFlag),
			"get_reset_flag", &App::getResetFlag,
			"toggle_reset_flag", &App::toggleResetFlag,
			"set_debug_flag", sol::overload(&App::setResetFlag, &LuaApp::setResetFlag),
			"renderer_type", sol::property(&App::setRendererType),
			"quit", &App::quit
		);
	}

	LuaError::LuaError(const std::string& msg, const sol::error& error)
		: _msg{ msg }
		, error{ error }
	{
	}

	const char* LuaError::what() const noexcept
	{
		return _msg.c_str();
	}

	LuaAppDelegate::LuaAppDelegate(App& app) noexcept
		: _impl{ std::make_unique<LuaAppDelegateImpl>(app) }
	{
	}

	LuaAppDelegate::~LuaAppDelegate() noexcept
	{
		// intentionally left blank for the unique_ptr<LuaAppDelegateImpl> forward declaration
	}

	std::optional<int32_t> LuaAppDelegate::setup(const CmdArgs& args)
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

	std::optional<int32_t> LuaAppDelegateImpl::setup(const CmdArgs& args) noexcept
	{
		CLI::App cli{ "darmok lua" };

		CliConfig cfg;
		cli.set_version_flag("-v,--version", "VERSION " DARMOK_VERSION);
		cli.add_option("-m,--main-lua", cfg.mainPath, "Path to the main lua file (dir will be taken as package path).");
		auto importGroup = cli.add_option_group("Asset Importer");
		BaseCommandLineFileImporter::setup(*importGroup, cfg.assetImport);

		try
		{
			cli.parse(args.size(), args.data());
			cfg.assetImport.fix(cli);
			if (!importAssets(cfg.assetImport))
			{
				return -3;
			}
			auto mainPath = findMainLua(cfg.mainPath);
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
		catch (const CLI::ParseError& ex)
		{
			return cli.exit(ex);
		}
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

		addStaticLib(lua_darmok_lib_table, "darmok/table");
		addStaticLib(lua_darmok_lib_string, "darmok/string");
		addStaticLib(lua_darmok_lib_middleclass, "darmok/middleclass", true);
		addStaticLib(lua_darmok_lib_class, "darmok/class");
		addStaticLib(lua_darmok_lib_base, "darmok/base");

		lua["app"] = std::ref(_app);
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
		_lua.reset();
	}

	std::optional<std::filesystem::path> LuaAppDelegateImpl::findMainLua(const std::filesystem::path& mainPath)
	{
		static const std::vector<std::filesystem::path> possiblePaths = {
			"main.lua",
			"lua/main.lua",
			"assets/main.lua"
		};

		if (!mainPath.empty())
		{
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
			throw CLI::ValidationError("could not find specified main lua script");
		}
		for (auto& path : possiblePaths)
		{
			if (std::filesystem::exists(path))
			{
				return path;
			}
		}
		throw CLI::ValidationError("could not find main lua script");
		return std::nullopt;
	}

	void LuaAppDelegateImpl::addPackagePath(const std::string& path, bool binary) noexcept
	{
		static const char sep = ';';
		std::string fpath{ path };
		std::replace(fpath.begin(), fpath.end(), ',', sep);
		auto& lua = *_lua;
		auto key = binary ? "cpath" : "path";
		std::string current = lua["package"][key];

		for (auto& sub : StringUtils::split(fpath, sep))
		{
			std::filesystem::path spath(sub);
			if (!StringUtils::contains(spath.filename().string(), '?') && spath.extension().empty())
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
				throw LuaError{ "running init", result };
			}
		}
		LuaApp::addUpdater(_app, lua["update"]);
	}

	bool LuaAppDelegateImpl::importAssets(const CommandLineFileImporterConfig& cfg)
	{
		if (cfg.dry || cfg.inputPath.empty())
		{
			return false;
		}

		DarmokAssetFileImporter importer(cfg);
		importer(std::cout);

		if (std::filesystem::is_directory(cfg.outputPath))
		{
			_app.addAssetsBasePath(cfg.outputPath);
		}
		return true;
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

	LuaAppComponent::LuaAppComponent(const sol::table& table) noexcept
		: _table{ table }
	{
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
		_initDef(_table, app);
	}

	void LuaAppComponent::shutdown()
	{
		_shutdownDef(_table);
	}

	bgfx::ViewId LuaAppComponent::renderReset(bgfx::ViewId viewId)
	{
		return _renderResetDef(_table, viewId).as<bgfx::ViewId>();
	}

	void LuaAppComponent::update(float deltaTime)
	{
		_updateDef(_table, deltaTime);
	}
}