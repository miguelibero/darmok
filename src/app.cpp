#include "detail/app.hpp"
#include "detail/input.hpp"
#include "detail/window.hpp"
#include "detail/audio_mini.hpp"
#include "detail/asset.hpp"

#include <darmok/app.hpp>
#include <darmok/color.hpp>
#include <darmok/stream.hpp>
#include <darmok/string.hpp>

#include <algorithm>

#include <bx/filepath.h>
#include <bx/timer.h>
#include <bx/file.h>
#include <bimg/bimg.h>
#include <fmt/format.h>
#include <magic_enum/magic_enum_format.hpp>

#if BX_PLATFORM_EMSCRIPTEN
#	include <emscripten.h>
#endif // BX_PLATFORM_EMSCRIPTEN

namespace darmok
{
	int32_t main(int32_t argc, const char* argv[], std::unique_ptr<IAppDelegateFactory>&& factory)
	{
		auto app = std::make_unique<App>(std::move(factory));
		const CmdArgs args(argv, argc);
		auto& appRef = *app;
		auto runner = std::make_unique<AppRunner>(std::move(app), args);
		if (auto code = runner->setup())
		{
			return code.value();
		}
		auto result = Platform::get().run(std::move(runner));
		if (!result)
		{
			appRef.onUnexpected(AppPhase::Run, result.error());
			return -1;
		}
		return result.value();
	}

	AppRunner::AppRunner(std::unique_ptr<App>&& app, const CmdArgs& args) noexcept
		: _app{ std::move(app) }
		, _args{ args }
		, _setupDone{ false }
	{
	}

	int AppRunner::operator()() noexcept
	{
		bool success = true;
		while (success)
		{
			if (auto code = setup())
			{
				return code.value();
			}
			auto result = AppRunResult::Continue;
			if (init())
			{
				while (result == AppRunResult::Continue)
				{
					result = run();
				}
			}
			else
			{
				success = false;
			}
			if (!shutdown())
			{
				success = false;
			}
			if (result == AppRunResult::Exit)
			{
				break;
			}
		}

		return success ? 0 : -1;
	}

	std::optional<int32_t> AppRunner::setup() noexcept
	{
		if (_setupDone)
		{
			return std::nullopt;
		}

		_setupDone = true;
		auto result = _app->setup(_args);
		if(!result)
		{
			_app->onUnexpected(AppPhase::Setup, result.error());
			return -1;
		}
		if (auto v = result.value())
		{
			return v;
		}
		return std::nullopt;
	}

	bool AppRunner::init() noexcept
	{
		auto result = _app->init();
		if (!result)
		{
			_app->onUnexpected(AppPhase::Init, result.error());
			return false;
		}
		return true;
	}

#if BX_PLATFORM_EMSCRIPTEN
	static App* s_app;
	static AppRunResult s_appRunResult;
	static void emscriptenRunApp()
	{
		s_appRunResult = s_app->run();
	}
#endif // BX_PLATFORM_EMSCRIPTEN

	AppRunResult AppRunner::run() noexcept
	{
		auto result = AppRunResult::Continue;

#if BX_PLATFORM_EMSCRIPTEN
		s_app = app.get();
		emscripten_set_main_loop(&emscriptenRunApp, -1, 1);
		result = s_appRunResult;
#else
		while (result == AppRunResult::Continue)
		{
			auto runResult = _app->run();
			if (!runResult)
			{
				_app->onUnexpected(AppPhase::Update, runResult.error());
				return AppRunResult::Exit;
			}
			result = runResult.value();
		}
#endif // BX_PLATFORM_EMSCRIPTEN
		return result;
	}

	bool AppRunner::shutdown() noexcept
	{
		auto result = _app->shutdown();
		if (!result)
		{
			_app->onUnexpected(AppPhase::Shutdown, result.error());
			return false;
		}
		_setupDone = false;
		return true;
	}

	AppImpl::AppImpl(App& app, std::unique_ptr<IAppDelegateFactory>&& factory) noexcept
		: _assets{ { _dataLoader, _allocator } }
		, _app{ app }
		, _delegateFactory{ std::move(factory) }
		, _runResult{ AppRunResult::Continue }
		, _running{ false }
		, _paused{ false }
		, _renderReset{ false }
		, _debugFlags{ BGFX_DEBUG_NONE }
		, _resetFlags{ BGFX_RESET_NONE }
		, _clearColor{ Colors::fromNumber(0x303030ff) }
		, _activeResetFlags{ BGFX_RESET_NONE }
		, _lastUpdate{ 0 }
		, _updateConfig{ AppUpdateConfig::getDefaultConfig() }
		, _plat{ Platform::get() }
		, _window{ _plat }
		, _renderSize{ 0 }
		, _rendererType{ bgfx::RendererType::Count }
	{
		constexpr auto defaultAssetsPath = "assets";
		addAssetsRootPath(defaultAssetsPath);
	}

	Input& AppImpl::getInput() noexcept
	{
		return _input;
	}

	const Input& AppImpl::getInput() const noexcept
	{
		return _input;
	}

	Window& AppImpl::getWindow() noexcept
	{
		return _window;
	}

	const Window& AppImpl::getWindow() const noexcept
	{
		return _window;
	}

	AssetContext& AppImpl::getAssets() noexcept
	{
		return _assets;
	}

	const AssetContext& AppImpl::getAssets() const noexcept
	{
		return _assets;
	}

	Platform& AppImpl::getPlatform() noexcept
	{
		return _plat;
	}

	const Platform& AppImpl::getPlatform() const noexcept
	{
		return _plat;
	}

	tf::Executor& AppImpl::getTaskExecutor()
	{
		return _taskExecutor.value();
	}

	const tf::Executor& AppImpl::getTaskExecutor() const
	{
		return _taskExecutor.value();
	}

#ifdef DARMOK_MINIAUDIO
	AudioSystem& AppImpl::getAudio() noexcept
	{
		return _audio;
	}

	const AudioSystem& AppImpl::getAudio() const noexcept
	{
		return _audio;
	}
#endif

	const AppUpdateConfig& AppUpdateConfig::getDefaultConfig() noexcept
	{
		static const AppUpdateConfig config
		{
			1.0F / 30.0F,
			10
		};
		return config;
	}

	float AppImpl::updateTimePassed() noexcept
	{
		auto now = bx::getHPCounter();
		auto timePassed = float(now - _lastUpdate) / float(bx::getHPFrequency());
		_lastUpdate = bx::getHPCounter();
		return timePassed;
	}

	void AppImpl::setUpdateConfig(const AppUpdateConfig& config) noexcept
	{
		_updateConfig = config;
	}

	void AppImpl::setPaused(bool paused) noexcept
	{
		_paused = paused;
	}

	bool AppImpl::isPaused() const noexcept
	{
		return _paused;
	}

	bool AppImpl::setAssetsBasePath(const std::filesystem::path& path) noexcept
	{
		return _dataLoader.setBasePath(path);
	}

	bool AppImpl::addAssetsRootPath(const std::filesystem::path& path) noexcept
	{
		return _dataLoader.addRootPath(path);
	}

	bool AppImpl::removeAssetsRootPath(const std::filesystem::path& path) noexcept
	{
		return _dataLoader.removeRootPath(path);
	}

	void AppImpl::addUpdater(std::unique_ptr<IAppUpdater>&& updater) noexcept
	{
		_updaters.insert(std::move(updater));
	}

	void AppImpl::addUpdater(IAppUpdater& updater) noexcept
	{
		_updaters.insert(updater);
	}

	bool AppImpl::removeUpdater(const IAppUpdater& updater) noexcept
	{
		return _updaters.erase(updater);
	}

	size_t AppImpl::removeUpdaters(const IAppUpdaterFilter& filter) noexcept
	{
		return _updaters.eraseIf(filter);
	}

	expected<int32_t, std::string> AppImpl::setup(const CmdArgs& args) noexcept
	{
		if (_delegateFactory)
		{
			_delegate = (*_delegateFactory)(_app);
		}
		if (_delegate)
		{
			return _delegate->setup(args);
		}
		return 0;
	}

	void AppImpl::bgfxInit() noexcept
	{
		bgfx::Init init;
		_renderSize = _window.getSize();
		_videoMode = _window.getVideoMode();
		init.platformData.ndt = _plat.getDisplayHandle();
		init.platformData.nwh = _plat.getWindowHandle();
		init.platformData.type = _plat.getWindowHandleType();
		init.debug = _debugFlags != BGFX_DEBUG_NONE;
		init.resolution.width = _renderSize.x;
		init.resolution.height = _renderSize.y;
		init.resolution.reset = _resetFlags;
		init.callback = &BgfxCallbacks::get();
		// _rendererType = bgfx::RendererType::Vulkan;
		init.type = _rendererType;
		bgfx::init(init);

		bgfx::setDebug(_debugFlags);
		_activeResetFlags = _resetFlags;

		bgfx::setPaletteColor(0, UINT32_C(0x00000000));
		//bgfx::setPaletteColor(1, UINT32_C(0x303030ff));
		bgfx::setPaletteColor(1, Colors::toNumber(_clearColor));
		bgfx::setPaletteColor(2, UINT32_C(0xFFFFFFFF));
	}

	expected<void, std::string> AppImpl::init() noexcept
	{
		_lastUpdate = bx::getHPCounter();
		_runResult = AppRunResult::Continue;

		bgfxInit();

		auto maxEncoders = bgfx::getCaps()->limits.maxEncoders;
		_taskExecutor.emplace(maxEncoders - 1);

		_input.getKeyboard().addListener(*this);
		auto assetsResult = _assets.getImpl().init(_app);
		if (!assetsResult)
		{
			return unexpected<std::string>{"assets: " + assetsResult.error() };
		}
#ifdef DARMOK_MINIAUDIO
		auto audioResult = _audio.getImpl().init();
		if (!audioResult)
		{
			return unexpected<std::string>{"audio: " + audioResult.error() };
		}
#endif

		_running = true;
		_renderReset = true;

		std::vector<std::string> errors;
		for (const auto& comp : Components{_components})
		{
			auto result = comp->init(_app);
			if(!result)
			{
				errors.push_back(std::move(result).error());
			}
		}

		if (_delegate)
		{
			auto result = _delegate->init();
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}

		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> AppImpl::shutdown() noexcept
	{
		std::vector<std::string> errors;

		if (_delegate)
		{
			auto result = _delegate->earlyShutdown();
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}

		if (_taskExecutor)
		{
			_taskExecutor->wait_for_all();
			_taskExecutor.reset();
		}

		_running = false;

		{
			auto components = Components{_components};
			for (auto itr = components.rbegin(); itr != components.rend(); ++itr)
			{
				auto result = (*itr)->shutdown();
				if (!result)
				{
					errors.push_back(std::move(result).error());
				}
			}
		}
		_components.clear();
		_updaters.clear();

		_input.getImpl().shutdown();
		auto assetsResult = _assets.getImpl().shutdown();
		if(!assetsResult)
		{
			errors.push_back("assets: " + assetsResult.error());
		}
		_window.getImpl().shutdown();
#ifdef DARMOK_MINIAUDIO
        _audio.getImpl().shutdown();
#endif

		if (_delegate)
		{
			auto result = _delegate->shutdown();
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
			// delete delegate before bgfx shutdown to ensure no dangling resources
			_delegate.reset();
		}

		// Shutdown bgfx.
		bgfx::shutdown();
		return StringUtils::joinExpectedErrors(errors);
	}

	void AppImpl::quit() noexcept
	{
		_runResult = AppRunResult::Exit;
	}

	void AppImpl::onUnexpected(AppPhase phase, const std::string& error) noexcept
	{
		std::ostringstream out{ "[DARMOK] error running app " };
		switch (phase)
		{
		case AppPhase::Setup:
			out << "setup";
			break;
		case AppPhase::Init:
			out << "init";
			break;
		case AppPhase::Update:
			out << "update";
			break;
		case AppPhase::Shutdown:
			out << "shutdown";
			break;
		}
		out << ": " << error;
		StreamUtils::log(out.str());
	}

	expected<void, std::string> AppImpl::update(float deltaTime) noexcept
	{
		std::vector<std::string> errors;
		if (!_paused)
		{
			if (_delegate)
			{
				auto result = _delegate->update(deltaTime);
				if (!result)
				{
					errors.push_back(std::move(result).error());
				}
			}

			for (const auto& comp : Components{_components})
			{
				auto result = comp->update(deltaTime);
				if (!result)
				{
					errors.push_back(std::move(result).error());
				}
			}
			for (auto& updater : _updaters.copy())
			{
				auto result = updater.update(deltaTime);
				if (!result)
				{
					errors.push_back(std::move(result).error());
				}
			}
		}

		auto assetsResult = _assets.getImpl().update();
		if(!assetsResult)
		{
			errors.push_back("assets: " + assetsResult.error());
		}

#ifdef DARMOK_MINIAUDIO
		_audio.getImpl().update();
#endif
		_input.getImpl().afterUpdate(deltaTime);

		const auto& renderSize = _app.getWindow().getSize();
		const auto& videoMode = _app.getWindow().getVideoMode();
		if (_renderSize != renderSize || _videoMode != videoMode || _activeResetFlags != _resetFlags)
		{
			_renderSize = renderSize;
			_videoMode = videoMode;
			_activeResetFlags = _resetFlags;
			_renderReset = true;
		}
		if (_renderReset)
		{
			_renderReset = false;
			auto result = renderReset();
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	void AppImpl::requestRenderReset() noexcept
	{
		_renderReset = true;
	}

	expected<bgfx::ViewId, std::string> AppImpl::renderReset() noexcept
	{
		bgfx::reset(_renderSize.x, _renderSize.y, _activeResetFlags);

		auto numViews = bgfx::getCaps()->limits.maxViews;
		for (bgfx::ViewId i = 0; i < numViews; ++i)
		{
			bgfx::resetView(i);
		}

		bgfx::ViewId viewId = 0;

		{
			bgfx::setViewName(viewId, "App clear");
			bgfx::setViewRect(viewId, 0, 0, bgfx::BackbufferRatio::Equal);
			const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL;
			static const uint8_t clearColor = 1;
			bgfx::setViewClear(viewId, clearFlags, 1.F, 0U,
				clearColor, clearColor, clearColor, clearColor,
				clearColor, clearColor, clearColor, clearColor);
			++viewId;
		}

		if (_delegate)
		{
			auto result = _delegate->renderReset(viewId);
			if (!result)
			{
				return result;
			}
			viewId = result.value();
		}

		for (const auto& comp : Components{_components})
		{
			auto result = comp->renderReset(viewId);
			if (!result)
			{
				return result;
			}
			viewId = result.value();
		}
		return viewId;
	}

	expected<void, std::string> AppImpl::render() const noexcept
	{
		bgfx::touch(0);
		bgfx::dbgTextClear();
		std::vector<std::string> errors;
		if (_delegate)
		{
			auto result = _delegate->render();
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		for (const auto& comp : Components{_components})
		{
			auto result = comp->render();
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	static uint32_t setFlag(uint32_t flags, uint32_t flag, bool enabled) noexcept
	{
		if (enabled)
		{
			return flags | flag;
		}
		return flags & ~flag;
	}

	expected<void, std::string> AppImpl::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) noexcept
	{
		if (!down)
		{
			return {};
		}
		// TODO: only enable these in debug builds
		return handleDebugShortcuts(key, modifiers);
	}

	const std::vector<bgfx::RendererType::Enum>& AppImpl::getSupportedRenderers() noexcept
	{
		static const std::vector<bgfx::RendererType::Enum> renderers {
#if BX_PLATFORM_WINDOWS
			bgfx::RendererType::Direct3D11, bgfx::RendererType::Direct3D12,
#endif
#if BX_PLATFORM_OSX
			bgfx::RendererType::Metal,
#endif
#if BX_PLATFORM_IOS || BX_PLATFORM_ANDROID
			bgfx::RendererType::OpenGLES,
#endif
			bgfx::RendererType::Vulkan,
			bgfx::RendererType::OpenGL,
		};

		return renderers;
	}

	expected<void, std::string> AppImpl::setRendererType(bgfx::RendererType::Enum renderer) noexcept
	{
		// Count means default renderer chosen by bgfx based on the platform
		if (renderer != bgfx::RendererType::Count)
		{
			const auto& renderers = getSupportedRenderers();
			auto itr = std::find(renderers.begin(), renderers.end(), renderer);
			if (itr == renderers.end())
			{
				return unexpected<std::string>{ "renderer not supported" };
			}
		}
		if (renderer == bgfx::getCaps()->rendererType)
		{
			return {};
		}
		_rendererType = renderer;
		if (_running)
		{
			_runResult = AppRunResult::Restart;
		}
		return {};
	}

	expected<void, std::string> AppImpl::setNextRenderer() noexcept
	{
		auto renderer = bgfx::getCaps()->rendererType;
		const auto& renderers = getSupportedRenderers();
		auto itr = std::find(renderers.begin(), renderers.end(), renderer);
		size_t i = 0;
		if (itr != renderers.end())
		{
			i = std::distance(renderers.begin(), itr);
			i = (i + 1) % renderers.size();
		}
		return setRendererType(renderers.at(i));
	}

	void AppImpl::requestNextVideoMode() noexcept
	{
		VideoMode mode = _window.getVideoMode();
		mode.screenMode = (WindowScreenMode)((toUnderlying(mode.screenMode) + 1) % toUnderlying(WindowScreenMode::Count));
		_window.requestVideoMode(mode);
	}

	expected<void, std::string> AppImpl::handleDebugShortcuts(KeyboardKey key, const KeyboardModifiers& modifiers) noexcept
	{
		static const KeyboardModifiers ctrl{ Keyboard::Definition::Ctrl };
		static const KeyboardModifiers alt{ Keyboard::Definition::Alt };
		static const KeyboardModifiers shift{ Keyboard::Definition::Shift };

		if (key == Keyboard::Definition::KeyQ && modifiers == ctrl)
		{
			quit();
			return {};
		}
		if ((key == Keyboard::Definition::Return && modifiers == alt)
			|| (key == Keyboard::Definition::KeyF && modifiers == ctrl))
		{
			requestNextVideoMode();
			return {};
		}
		if (key == Keyboard::Definition::F1)
		{
			if (modifiers == ctrl)
			{
				toggleDebugFlag(BGFX_DEBUG_IFH);
			}
			else if (modifiers == shift)
			{
				setDebugFlag(BGFX_DEBUG_STATS, false);
				setDebugFlag(BGFX_DEBUG_TEXT, false);
			}
			else if(modifiers.empty())
			{
				toggleDebugFlag(BGFX_DEBUG_STATS);
			}
			return {};
		}
		if (key == Keyboard::Definition::F2 && modifiers.empty())
		{
			toggleDebugFlag(BGFX_DEBUG_TEXT);
			return {};
		}
		if (key == Keyboard::Definition::F3 && modifiers.empty())
		{
			toggleDebugFlag(BGFX_DEBUG_WIREFRAME);
			return {};
		}
		if (key == Keyboard::Definition::F4 && modifiers.empty())
		{
			toggleDebugFlag(BGFX_DEBUG_PROFILER);
			return {};
		}
		if ((key == Keyboard::Definition::F5 && modifiers.empty())
			|| (key == Keyboard::Definition::KeyR && modifiers == ctrl))
		{
			_runResult = AppRunResult::Restart;
			return {};
		}
		if ((key == Keyboard::Definition::F5 && modifiers == ctrl))
		{
			return setNextRenderer();
		}
		if ((key == Keyboard::Definition::Print && modifiers.empty())
			|| (key == Keyboard::Definition::KeyP && modifiers == ctrl))
		{
			auto filePath = "temp/screenshot-" + getTimeSuffix();
			bgfx::requestScreenShot(BGFX_INVALID_HANDLE, filePath.c_str());
			return {};
		}
		if (key == Keyboard::Definition::Print && modifiers == ctrl)
		{
			toggleTaskflowProfile();
			return {};
		}
		if (key == Keyboard::Definition::Pause)
		{
			_paused = !_paused;
			return {};
		}
		return {};
	}

	std::string AppImpl::getTimeSuffix() noexcept
	{
		time_t val;
		time(&val);
		return std::to_string(val);
	}

	void AppImpl::toggleTaskflowProfile() noexcept
	{
		if (!_taskExecutor)
		{
			return;
		}
		if (_taskObserver == nullptr)
		{
			_taskObserver = _taskExecutor->make_observer<tf::TFProfObserver>();
			return;
		}
		static const std::filesystem::path basePath = "temp";
		auto suffix = getTimeSuffix();
		{
			auto filePath = basePath / ("taskflow-" + suffix + ".json");
			std::ofstream out(filePath);
			out << "[";
			_taskObserver->dump(out);
			_taskObserver.reset();
			out << "]";
		}
	}

	bool AppImpl::toggleDebugFlag(uint32_t flag) noexcept
	{
		auto value = !getDebugFlag(flag);
		setDebugFlag(flag, value);
		return value;
	}

	void AppImpl::setDebugFlag(uint32_t flag, bool enabled) noexcept
	{
		_debugFlags = setFlag(_debugFlags, flag, enabled);
		if (_running)
		{
			bgfx::setDebug(_debugFlags);
		}
	}

	bool AppImpl::getDebugFlag(uint32_t flag) const noexcept
	{
		return static_cast<bool>(_debugFlags & flag);
	}

	bool AppImpl::toggleResetFlag(uint32_t flag) noexcept
	{
		auto value = !getResetFlag(flag);
		setResetFlag(flag, value);
		return value;
	}

	void AppImpl::setResetFlag(uint32_t flag, bool enabled) noexcept
	{
		_resetFlags = setFlag(_resetFlags, flag, enabled);
	}

	bool AppImpl::getResetFlag(uint32_t flag) const noexcept
	{
		return static_cast<bool>(_resetFlags & flag);
	}

	void AppImpl::setClearColor(const Color& color) noexcept
	{
		if (_clearColor != color)
		{
			_clearColor = color;
			bgfx::setPaletteColor(1, Colors::toNumber(_clearColor));
		}
	}

	AppRunResult AppImpl::processEvents() noexcept
	{
		while (_runResult == AppRunResult::Continue)
		{
			auto patEv = getPlatform().pollEvent();
			if (patEv == nullptr)
			{
				break;
			}
			auto processResult = PlatformEvent::process(*patEv, _input, _window);
			if (!processResult)
			{
				onUnexpected(AppPhase::Update, processResult.error());
			}
			if (_window.getPhase() == WindowPhase::Destroyed)
			{
				return AppRunResult::Exit;
			}
		};
		return _runResult;
	}

	expected<void, std::string> AppImpl::addComponent(std::unique_ptr<IAppComponent>&& component) noexcept
	{
		if (auto type = component->getAppComponentType())
		{
			removeComponent(type);
		}
		if (_running)
		{
			auto result = component->init(_app);
			if (!result)
			{
				return result;
			}
		}
		_components.push_back(std::move(component));
		return {};
	}

	AppImpl::Components::iterator AppImpl::findComponent(entt::id_type type) noexcept
	{
		return std::find_if(_components.begin(), _components.end(),
			[type](auto& comp) { return comp->getAppComponentType() == type; });
	}

	AppImpl::Components::const_iterator AppImpl::findComponent(entt::id_type type) const noexcept
	{
		return std::find_if(_components.begin(), _components.end(),
			[type](auto& comp) { return comp->getAppComponentType() == type; });
	}

	bool AppImpl::removeComponent(entt::id_type type) noexcept
	{
		auto itr = findComponent(type);
		auto found = itr != _components.end();
		if (found)
		{
			_components.erase(itr);
		}
		return found;
	}

	bool AppImpl::hasComponent(entt::id_type type) const noexcept
	{
		auto itr = findComponent(type);
		return itr != _components.end();
	}

	OptionalRef<IAppComponent> AppImpl::getComponent(entt::id_type type) noexcept
	{
		auto itr = findComponent(type);
		if (itr == _components.end())
		{
			return nullptr;
		}
		return **itr;
	}

	OptionalRef<const IAppComponent> AppImpl::getComponent(entt::id_type type) const noexcept
	{
		auto itr = findComponent(type);
		if (itr == _components.end())
		{
			return nullptr;
		}
		return **itr;
	}

	App::App(std::unique_ptr<IAppDelegateFactory>&& factory) noexcept
		: _impl{ std::make_unique<AppImpl>(*this, std::move(factory)) }
	{
	}

	App::~App() noexcept = default;

	expected<int32_t, std::string> App::setup(const CmdArgs& args) noexcept
	{
		return _impl->setup(args);
	}

	expected<void, std::string> App::init() noexcept
	{
		_impl->processEvents();
		return _impl->init();
	}

	void App::onUnexpected(AppPhase phase, const std::string& error) noexcept
	{
		_impl->onUnexpected(phase, error);
	}

	expected<void, std::string> App::shutdown() noexcept
	{
		return _impl->shutdown();
	}

	Input& App::getInput() noexcept
	{
		return _impl->getInput();
	}

	const Input& App::getInput() const noexcept
	{
		return _impl->getInput();
	}

	Window& App::getWindow() noexcept
	{
		return _impl->getWindow();
	}

	const Window& App::getWindow() const noexcept
	{
		return _impl->getWindow();
	}

	AssetContext& App::getAssets() noexcept
	{
		return _impl->getAssets();
	}

	const AssetContext& App::getAssets() const noexcept
	{
		return _impl->getAssets();
	}

	tf::Executor& App::getTaskExecutor() noexcept
	{
		return _impl->getTaskExecutor();
	}

	const tf::Executor& App::getTaskExecutor() const noexcept
	{
		return _impl->getTaskExecutor();
	}

#ifdef DARMOK_MINIAUDIO

	AudioSystem& App::getAudio() noexcept
	{
		return _impl->getAudio();
	}

	const AudioSystem& App::getAudio() const noexcept
	{
		return _impl->getAudio();
	}

#endif

	expected<AppRunResult, std::string> App::run() noexcept
	{
		auto processResult = _impl->processEvents();
		if (processResult != AppRunResult::Continue)
		{
			return processResult;
		}

		std::vector<std::string> errors;
		_impl->deltaTimeCall([this, &errors](float deltaTime) {
			auto result = _impl->update(deltaTime);
			if(!result)
			{
				errors.push_back(std::move(result).error());
			}
		});

		auto result = render();
		if(!result)
		{
			errors.push_back(std::move(result).error());
		}

		bgfx::frame();

		if(!errors.empty())
		{
			return unexpected{ StringUtils::joinErrors(errors) };
		}

		return AppRunResult::Continue;
	}

	void App::quit() noexcept
	{
		_impl->quit();
	}

	void App::requestRenderReset() noexcept
	{
		_impl->requestRenderReset();
	}

	void App::setUpdateConfig(const AppUpdateConfig& config) noexcept
	{
		_impl->setUpdateConfig(config);
	}

	void App::setPaused(bool paused) noexcept
	{
		_impl->setPaused(paused);
	}

	bool App::isPaused() const noexcept
	{
		return _impl->isPaused();
	}

	bool App::setAssetsBasePath(const std::filesystem::path& path) noexcept
	{
		return _impl->setAssetsBasePath(path);
	}

	bool App::addAssetsRootPath(const std::filesystem::path& path) noexcept
	{
		return _impl->addAssetsRootPath(path);
	}

	bool App::removeAssetsRootPath(const std::filesystem::path& path) noexcept
	{
		return _impl->removeAssetsRootPath(path);
	}

	void App::addUpdater(std::unique_ptr<IAppUpdater>&& updater) noexcept
	{
		_impl->addUpdater(std::move(updater));
	}

	void App::addUpdater(IAppUpdater& updater) noexcept
	{
		_impl->addUpdater(updater);
	}

	bool App::removeUpdater(const IAppUpdater& updater) noexcept
	{
		return _impl->removeUpdater(updater);
	}

	size_t App::removeUpdaters(const IAppUpdaterFilter& filter) noexcept
	{
		return _impl->removeUpdaters(filter);
	}

	expected<void, std::string> App::render() const noexcept
	{
		return _impl->render();
	}

	bool App::toggleDebugFlag(uint32_t flag) noexcept
	{
		return _impl->toggleDebugFlag(flag);
	}

	void App::setDebugFlag(uint32_t flag, bool enabled) noexcept
	{
		_impl->setDebugFlag(flag, enabled);
	}

	bool App::getDebugFlag(uint32_t flag) const noexcept
	{
		return _impl->getDebugFlag(flag);
	}

	bool App::toggleResetFlag(uint32_t flag) noexcept
	{
		return _impl->toggleResetFlag(flag);
	}

	void App::setResetFlag(uint32_t flag, bool enabled) noexcept
	{
		_impl->setResetFlag(flag, enabled);
	}

	bool App::getResetFlag(uint32_t flag) const noexcept
	{
		return _impl->getResetFlag(flag);
	}

	expected<void, std::string> App::setRendererType(bgfx::RendererType::Enum renderer) noexcept
	{
		return _impl->setRendererType(renderer);
	}

	expected<void, std::string> App::addComponent(std::unique_ptr<IAppComponent>&& component) noexcept
	{
		return _impl->addComponent(std::move(component));
	}

	bool App::removeComponent(entt::id_type type) noexcept
	{
		return _impl->removeComponent(type);
	}

	bool App::hasComponent(entt::id_type type) const noexcept
	{
		return _impl->hasComponent(type);
	}

	OptionalRef<IAppComponent> App::getComponent(entt::id_type type) noexcept
	{
		return _impl->getComponent(type);
	}

	OptionalRef<const IAppComponent> App::getComponent(entt::id_type type) const noexcept
	{
		return _impl->getComponent(type);
	}

	BgfxCallbacks::BgfxCallbacks() noexcept
		: _cache()
	{
	}

	BgfxCallbacks& BgfxCallbacks::get() noexcept
	{
		static BgfxCallbacks instance;
		return instance;
	}

	void BgfxCallbacks::fatal(
		const char* filePath
		, uint16_t line
		, bgfx::Fatal::Enum code
		, const char* str
	) noexcept
	{
		if (bgfx::Fatal::DebugCheck == code)
		{
			// TODO: is this fatal normal?
			if (!std::string_view{ str }.starts_with("Destroying already destroyed uniform "))
			{
				bx::debugBreak();
			}
			return;
		}
		StreamUtils::log(fmt::format("{} {}", code, str), true);
	}

	void BgfxCallbacks::traceVargs(
		const char* filePath
		, uint16_t line
		, const char* format
		, va_list argList
	) noexcept
	{
		std::string str;
		bx::stringPrintfVargs(str, format, argList);
        bx::debugOutput(bx::StringView{str.data(), static_cast<int32_t>(str.size())});
	}

	// bgfx tracy integration problems
	// https://github.com/bkaradzic/bgfx/pull/3308

	void BgfxCallbacks::profilerBegin(
		const char* name
		, uint32_t abgr
		, const char* filePath
		, uint16_t line
	) noexcept
	{
		// TODO: vcpkg bgfx build without profiler enabled
	}

	void BgfxCallbacks::profilerBeginLiteral(
		const char* name
		, uint32_t abgr
		, const char* filePath
		, uint16_t line
	) noexcept
	{
		// TODO: vcpkg bgfx build without profiler enabled
	}

	void BgfxCallbacks::profilerEnd() noexcept
	{
		// TODO: vcpkg bgfx build without profiler enabled
	}

	uint32_t BgfxCallbacks::cacheReadSize(uint64_t resId) noexcept
	{
		const std::lock_guard lock(_cacheMutex);
		auto itr = _cache.find(resId);
		if (itr == _cache.end())
		{
			return 0;
		}
		return static_cast<uint32_t>(itr->second.size());
	}

	bool BgfxCallbacks::cacheRead(uint64_t resId, void* dataPtr, uint32_t size) noexcept
	{
		const std::lock_guard lock(_cacheMutex);
		auto itr = _cache.find(resId);
		if (itr == _cache.end())
		{
			return false;
		}
		auto& data = itr->second;
		if (data.size() < size)
		{
			size = static_cast<uint32_t>(data.size());
		}
		std::memcpy(dataPtr, data.ptr(), size);
		return true;
	}

	void BgfxCallbacks::cacheWrite(uint64_t resId, const void* data, uint32_t size) noexcept
	{
		_cache.emplace(resId, Data(data, size));
	}

	void BgfxCallbacks::screenShot(
		const char* filePath
		, uint32_t width
		, uint32_t height
		, uint32_t pitch
		, const void* data
		, uint32_t /* size */
		, bool yflip
	) noexcept
	{
		std::filesystem::path path(filePath);
		auto ext = path.extension().string();
		if (ext.empty())
		{
			path += ".png";
		}
		bx::FileWriter writer;
		if (bx::open(&writer, path.string().c_str()))
		{
			auto format = bimg::TextureFormat::BGRA8;
			bx::Error err;
			bimg::imageWritePng(&writer, width, height, pitch, data, format, yflip, &err);
			bx::close(&writer);
			if (!err.isOk())
			{
				StreamUtils::log(fmt::format("screenshot error: {}", err.getMessage().getCPtr()));
			}
		}
	}

	void BgfxCallbacks::captureBegin(
		uint32_t width
		, uint32_t height
		, uint32_t pitch
		, bgfx::TextureFormat::Enum format
		, bool yflip
	) noexcept
	{
		// TODO
	}

	void BgfxCallbacks::captureEnd() noexcept
	{
		// TODO
	}

	void BgfxCallbacks::captureFrame(const void* data, uint32_t size) noexcept
	{
		// TODO
	}
}
