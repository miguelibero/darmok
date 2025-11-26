#include "lua/app.hpp"
#include <darmok/app.hpp>
#include <darmok/lua.hpp>
#include <darmok/scene.hpp>
#include <darmok/string.hpp>
#include <darmok/asset.hpp>
#include <darmok/audio.hpp>
#include <darmok/window.hpp>
#include <darmok/stream.hpp>
#include <darmok/data.hpp>

#include "lua/asset.hpp"
#include "lua/input.hpp"
#include "lua/audio.hpp"
#include "lua/scene.hpp"
#include "lua/window.hpp"
#include "lua/math.hpp"
#include "lua/shape.hpp"
#include "lua/texture.hpp"
#include "lua/skeleton.hpp"
#include "lua/render_chain.hpp"

#include "generated/lua/string.h"
#include "generated/lua/table.h"
#include "generated/lua/middleclass.h"
#include "generated/lua/class.h"
#include "generated/lua/base.h"
#include "generated/lua/bit.h"

#ifdef DARMOK_RMLUI
#include <darmok/rmlui.hpp>
#include "lua/rmlui.hpp"
#ifdef _DEBUG
#define RMLUI_DEBUGGER
#include "lua/rmlui_debug.hpp"
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

	expected<void, std::string> LuaAppUpdater::update(float deltaTime) noexcept
	{
		return _delegate.tryGet<void>(deltaTime);
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

	LuaCoroutine LuaApp::startCoroutine(App& app, const sol::function& func)
	{
		auto runner = LuaUtils::unwrapExpected(app.getOrAddComponent<LuaCoroutineRunner>());
		return runner.get().startCoroutine(func);
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
		LuaUtils::unwrapExpected(app.addComponent(std::make_unique<LuaAppComponent>(table)));
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
		LuaAssetContext::bind(lua);
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

	LuaError::LuaError(std::string_view msg, const sol::error& error)
		: _msg{ msg }
		, error{ error }
	{
		LuaUtils::logError(msg, error);
	}

	const char* LuaError::what() const noexcept
	{
		return _msg.c_str();
	}

	LuaAppDelegate::LuaAppDelegate(App& app) noexcept
		: _impl{ std::make_unique<LuaAppDelegateImpl>(app) }
	{
	}

	LuaAppDelegate::~LuaAppDelegate() noexcept = default;

	expected<int32_t, std::string> LuaAppDelegate::setup(const CmdArgs& args) noexcept
	{
		return _impl->setup(args);
	}

	expected<void, std::string> LuaAppDelegate::init() noexcept
	{
		return _impl->init();
	}

	expected<void, std::string> LuaAppDelegate::earlyShutdown() noexcept
	{
		return _impl->earlyShutdown();
	}

	expected<void, std::string> LuaAppDelegate::shutdown() noexcept
	{
		return _impl->shutdown();
	}

	expected<void, std::string> LuaAppDelegate::render() const noexcept
	{
		return _impl->render();
	}

	LuaAppDelegateImpl::LuaAppDelegateImpl(App& app) noexcept
		: _app{ app }
	{
	}

	static void luaPrint(sol::variadic_args args) noexcept
	{
		std::vector<std::string> strArgs;
		strArgs.reserve(args.size());
		std::transform(args.begin(), args.end(), std::back_inserter(strArgs),
			[](const auto& arg) { return arg.as<std::string>(); });
		StreamUtils::log(StringUtils::join(", ", strArgs) + "\n");
	}

	void LuaAppDelegateImpl::luaDebugScreenText(const glm::uvec2& pos, const std::string& msg) noexcept
	{
		_dbgTexts.emplace_back(pos, msg);
	}

	expected<int32_t, std::string> LuaAppDelegateImpl::setup(const CmdArgs& args) noexcept
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

	expected<int32_t, std::string> LuaAppDelegateImpl::loadLua(const std::filesystem::path& mainPath) noexcept
	{
		auto mainDir = mainPath.parent_path().string();

		_lua = std::make_unique<sol::state>();
		auto& lua = *_lua;

		lua.open_libraries(
			sol::lib::base, sol::lib::package, sol::lib::io,
			sol::lib::table, sol::lib::string, sol::lib::coroutine,
			sol::lib::math, sol::lib::os

#ifdef DARMOK_LUAJIT
			, sol::lib::ffi, sol::lib::jit, sol::lib::bit32
#endif
#ifndef _NDEBUG
			, sol::lib::debug
#endif
			);

		auto addStaticLib = [&lua](auto& lib, const std::string& name, bool create_global = false)
		{
			auto content = DataView::fromStatic(lib).stringView();
			lua.require_script(name, content, create_global);
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
		addStaticLib(lua_darmok_lib_bit, "darmok/bit", true);

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

		auto result = lua.safe_script_file(mainPath.string(), sol::script_pass_on_error);
		return LuaUtils::wrapResult<int32_t>(result);
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

	expected<void, std::string> LuaAppDelegateImpl::init() noexcept
	{
		if (_lua == nullptr)
		{
			auto result = loadLua(_mainLuaPath);
			if(!result)
			{
				return unexpected<std::string>{result.error()};
			}
		}
		auto& lua = *_lua;
		if (sol::protected_function init = lua["init"])
		{
			auto result = LuaUtils::wrapResult<void>(init());
			if (!result)
			{
				return result;
			}
		}
		LuaApp::addUpdater(_app, lua["update"]);
		return {};
	}

	bool LuaAppDelegateImpl::importAssets(const CommandLineFileImporterConfig& cfg) noexcept
	{
		if (cfg.dry || cfg.inputPath.empty())
		{
			return false;
		}

		DarmokAssetFileImporter importer{ cfg };
		if (!importer(std::cout))
		{
			return false;
		}

		if (std::filesystem::is_directory(cfg.outputPath))
		{
			_app.addAssetsRootPath(cfg.outputPath);
		}
		return true;
	}

	expected<void, std::string> LuaAppDelegateImpl::render() noexcept
	{
		for (auto& text : _dbgTexts)
		{
			bgfx::dbgTextPrintf(text.pos.x, text.pos.y, 0x0f, text.message.c_str());
		}
		_dbgTexts.clear();
		return {};
	}

	expected<void, std::string> LuaAppDelegateImpl::earlyShutdown() noexcept
	{
		if (!_lua)
		{
			return unexpected<std::string>{"lua not loaded"};
		}
		sol::protected_function shutdown = (*_lua)["shutdown"];
		if (!shutdown)
		{
			return {};
		}
		return LuaUtils::wrapResult<void>(shutdown());
	}

	expected<void, std::string> LuaAppDelegateImpl::shutdown() noexcept
	{
		unloadLua();
		return {};
	}

	LuaAppComponent::LuaAppComponent(const sol::table& table) noexcept
		: _table{ table }
	{
	}

	sol::object LuaAppComponent::getReal() const noexcept
	{
		return _table;
	}

	const LuaTableDelegateDefinition LuaAppComponent::_initDef{ "init" };
	const LuaTableDelegateDefinition LuaAppComponent::_shutdownDef{ "shutdown" };
	const LuaTableDelegateDefinition LuaAppComponent::_renderResetDef{ "render_reset" };
	const LuaTableDelegateDefinition LuaAppComponent::_updateDef{ "update" };

	expected<void, std::string> LuaAppComponent::init(App& app) noexcept
	{
		return _initDef.tryGet<void>(_table, app);
	}

	expected<void, std::string> LuaAppComponent::shutdown() noexcept
	{
		return _shutdownDef.tryGet<void>(_table);
	}

	expected<bgfx::ViewId, std::string> LuaAppComponent::renderReset(bgfx::ViewId viewId) noexcept
	{
		return _renderResetDef.tryGet<bgfx::ViewId>(_table, viewId);
	}

	expected<void, std::string>  LuaAppComponent::update(float deltaTime) noexcept
	{
		return _updateDef.tryGet<void>(_table, deltaTime);
	}
}