#include "app.hpp"
#include "input.hpp"
#include "window.hpp"

#include <darmok/app.hpp>
#include <darmok/color.hpp>
#include <darmok/stream.hpp>
#include <darmok/string.hpp>

#include <bx/filepath.h>
#include <bx/timer.h>
#include <iostream>
#include <algorithm>

#include <bx/file.h>
#include <bimg/bimg.h>

#if BX_PLATFORM_EMSCRIPTEN
#	include <emscripten.h>
#endif // BX_PLATFORM_EMSCRIPTEN

namespace darmok
{
	int32_t main(int32_t argc, const char* const* argv, std::unique_ptr<IAppDelegateFactory>&& factory)
	{
		auto app = std::make_unique<App>(std::move(factory));
		std::vector<std::string> args(argc);
		for (int i = 0; i < argc; ++i)
		{
			args[i] = argv[i];
		}
		auto runner = std::make_unique<AppRunner>(std::move(app), args);
		if (auto r = runner->setup())
		{
			return r.value();
		}
		return Platform::get().run(std::move(runner));
	}

	AppRunner::AppRunner(std::unique_ptr<App>&& app, const std::vector<std::string>& args) noexcept
		: _app(std::move(app))
		, _args(args)
		, _setupDone(false)
	{
	}

	int AppRunner::operator()() noexcept
	{
		bool success = true;
		while (success)
		{
			if (auto r = setup())
			{
				return r.value();
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
		try
		{
			_setupDone = true;
			return _app->setup(_args);
		}
		catch (const std::exception& ex)
		{
			_app->onException(App::Phase::Setup, ex);
			return -1;
		}
	}

	bool AppRunner::init() noexcept
	{
		try
		{
			_app->init();
			return true;
		}
		catch (const std::exception& ex)
		{
			_app->onException(App::Phase::Init, ex);
			return false;
		}
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
		try
		{
			auto result = AppRunResult::Continue;

#if BX_PLATFORM_EMSCRIPTEN
			s_app = app.get();
			emscripten_set_main_loop(&emscriptenRunApp, -1, 1);
			result = s_appRunResult;
#else
			while (result == AppRunResult::Continue)
			{
				result = _app->run();
			}
#endif // BX_PLATFORM_EMSCRIPTEN
			return result;
		}
		catch (const std::exception& ex)
		{
			_app->onException(App::Phase::Update, ex);
			return AppRunResult::Exit;
		}
	}

	bool AppRunner::shutdown() noexcept
	{
		try
		{
			_app->shutdown();
			_setupDone = false;
			return true;
		}
		catch (const std::exception& ex)
		{
			_app->onException(App::Phase::Shutdown, ex);
			return false;
		}
	}

	AppImpl::AppImpl(App& app, std::unique_ptr<IAppDelegateFactory>&& factory) noexcept
		: _app(app)
		, _delegateFactory(std::move(factory))
		, _runResult(AppRunResult::Continue)
		, _running(false)
		, _paused(false)
		, _debugFlags(BGFX_DEBUG_NONE)
		, _resetFlags(BGFX_RESET_NONE)
		, _clearColor(Colors::fromNumber(0x303030ff))
		, _activeResetFlags(BGFX_RESET_NONE)
		, _lastUpdate(0)
		, _updateConfig(AppUpdateConfig::getDefaultConfig())
		, _plat(Platform::get())
		, _window(_plat)
		, _pixelSize(0)
		, _rendererType(bgfx::RendererType::Count)
	{
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

	RenderGraphDefinition& AppImpl::getRenderGraph() noexcept
	{
		return _renderGraphDef;
	}

	const RenderGraphDefinition& AppImpl::getRenderGraph() const noexcept
	{
		return _renderGraphDef;
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

	std::optional<int32_t> AppImpl::setup(const std::vector<std::string>& args)
	{
		if (_delegateFactory)
		{
			_delegate = (*_delegateFactory)(_app);
		}
		if (_delegate)
		{
			return _delegate->setup(args);
		}
		return std::nullopt;
	}

	void AppImpl::bgfxInit()
	{
		bgfx::Init init;
		_pixelSize = _window.getPixelSize();
		_videoMode = _window.getVideoMode();
		init.platformData.ndt = _plat.getDisplayHandle();
		init.platformData.nwh = _plat.getWindowHandle();
		init.platformData.type = _plat.getWindowHandleType();
		init.debug = true;
		init.resolution.width = _pixelSize.x;
		init.resolution.height = _pixelSize.y;
		init.resolution.reset = _resetFlags;
		init.callback = &BgfxCallbacks::get();
		init.type = _rendererType;
		bgfx::init(init);

		bgfx::setDebug(_debugFlags);
		_activeResetFlags = _resetFlags;

		bgfx::setPaletteColor(0, UINT32_C(0x00000000));
		//bgfx::setPaletteColor(1, UINT32_C(0x303030ff));
		bgfx::setPaletteColor(1, Colors::toNumber(_clearColor));
		bgfx::setPaletteColor(2, UINT32_C(0xFFFFFFFF));
	}

	AppImpl::ComponentRefs AppImpl::copyComponentContainer() const noexcept
	{
		ComponentRefs refs;
		for (auto& [type, component] : _components)
		{
			refs.emplace_back(*component);
		}
		return refs;
	}

	void AppImpl::init()
	{
		_lastUpdate = bx::getHPCounter();
		_runResult = AppRunResult::Continue;
		_renderGraphDef.clear();

		bgfxInit();

		auto maxEncoders = bgfx::getCaps()->limits.maxEncoders;
		_taskExecutor.emplace(maxEncoders - 1);

		_input.getKeyboard().addListener(*this);
		_assets.init(_app);
		_audio.init();

		_renderGraphDef.addPass(*this);

		auto components = copyComponentContainer();

		_running = true;

		for (auto& comp : components)
		{
			comp.get().init(_app);
		}

		if (_delegate)
		{
			_delegate->init();
		}

	}

	void AppImpl::renderReset()
	{
		bgfx::reset(_pixelSize.x, _pixelSize.y, _activeResetFlags);
		_renderGraph.reset();
		_renderGraphDef.clear();
		_renderGraphDef.addPass(*this);

		if (_delegate)
		{
			_delegate->renderReset();
		}

		for (auto& comp : copyComponentContainer())
		{
			comp.get().renderReset();
		}
	}

	void AppImpl::shutdown()
	{
		if (_taskExecutor)
		{
			_taskExecutor->wait_for_all();
			_taskExecutor.reset();
		}

		_running = false;
		_renderGraph.reset();
		_renderGraphDef.clear();

		auto components = copyComponentContainer();
		for (auto itr = components.rbegin(); itr != components.rend(); ++itr)
		{
			itr->get().shutdown();
		}
		_components.clear();

		_input.getKeyboard().removeListener(*this);
		_audio.shutdown();
		_assets.shutdown();

		if (_delegate)
		{
			_delegate->shutdown();
			// delete delegate before bgfx shutdown to ensure no dangling resources
			_delegate.reset();
		}

		// Shutdown bgfx.
		bgfx::shutdown();
	}

	void AppImpl::update(float deltaTime)
	{
		if (!_paused)
		{
			if (_delegate)
			{
				_delegate->update(deltaTime);
			}

			for (auto& comp : copyComponentContainer())
			{
				comp.get().update(deltaTime);
			}
		}

		_assets.update();
		_audio.update();

		auto rgHash = _renderGraphDef.hash();
		if (!_renderGraph || _renderGraph->hash() != rgHash)
		{
			_taskExecutor->wait_for_all();
			auto& rg = _renderGraph.emplace(_renderGraphDef);
			rg.configureView(0);
		}
	}

	void AppImpl::renderPassConfigure(bgfx::ViewId viewId)
	{
		bgfx::setViewRect(viewId, 0, 0, bgfx::BackbufferRatio::Equal);
		const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL;
		uint8_t clearColor = 1;
		bgfx::setViewClear(viewId, clearFlags, 1.F, 0U,
			clearColor, clearColor, clearColor, clearColor,
			clearColor, clearColor, clearColor, clearColor);
	}

	void AppImpl::renderPassExecute(IRenderGraphContext& context)
	{
		// just clear the screen
		context.getEncoder().touch(context.getViewId());
	}

	void AppImpl::renderPassDefine(RenderPassDefinition& def)
	{
		def.setName("App clear");
		def.setPriority(IRenderGraphNode::kMaxPriority);
	}

	void AppImpl::afterUpdate(float deltaTime)
	{
		_input.getImpl().afterUpdate(deltaTime);

		auto& pixelSize = _app.getWindow().getPixelSize();
		auto& videoMode = _app.getWindow().getVideoMode();
		if (_pixelSize != pixelSize || _videoMode != videoMode || _activeResetFlags != _resetFlags)
		{
			_pixelSize = pixelSize;
			_videoMode = videoMode;
			_activeResetFlags = _resetFlags;
			renderReset();
		}
	}

	void AppImpl::render() const
	{
		if (_renderGraph && _taskExecutor)
		{
			_renderGraph.value()(_taskExecutor.value());
		}
		if (_delegate)
		{
			_delegate->render();
		}
	}

	static uint32_t setFlag(uint32_t flags, uint32_t flag, bool enabled) noexcept
	{
		if (enabled)
		{
			return flags | flag;
		}
		return flags & ~flag;
	}

	void AppImpl::onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down)
	{
		if (!down)
		{
			return;
		}
		// TODO: only enable these in debug builds
		handleDebugShortcuts(key, modifiers);
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

	void AppImpl::setRendererType(bgfx::RendererType::Enum renderer)
	{
		// Count means default renderer chosen by bgfx based on the platform
		if (renderer != bgfx::RendererType::Count)
		{
			auto& renderers = getSupportedRenderers();
			auto itr = std::find(renderers.begin(), renderers.end(), renderer);
			if (itr == renderers.end())
			{
				throw std::invalid_argument("renderer not supported");
			}
		}
		if (renderer == bgfx::getCaps()->rendererType)
		{
			return;
		}
		_rendererType = renderer;
		if (_running)
		{
			_runResult = AppRunResult::Restart;
		}
	}

	void AppImpl::setNextRenderer()
	{
		auto renderer = bgfx::getCaps()->rendererType;
		auto& renderers = getSupportedRenderers();
		auto itr = std::find(renderers.begin(), renderers.end(), renderer);
		size_t i = 0;
		if (itr != renderers.end())
		{
			i = std::distance(renderers.begin(), itr);
			i = (i + 1) % renderers.size();
		}
		setRendererType(renderers.at(i));
	}

	void AppImpl::requestNextVideoMode()
	{
		VideoMode mode = _window.getVideoMode();
		mode.screenMode = (WindowScreenMode)((to_underlying(mode.screenMode) + 1) % to_underlying(WindowScreenMode::Count));
		_window.requestVideoMode(mode);
	}

	void AppImpl::handleDebugShortcuts(KeyboardKey key, const KeyboardModifiers& modifiers)
	{
		static const KeyboardModifiers ctrl{ KeyboardModifier::Ctrl };
		static const KeyboardModifiers alt{ KeyboardModifier::Alt };
		static const KeyboardModifiers shift{ KeyboardModifier::Shift };

		if ((key == KeyboardKey::Esc && modifiers.empty())
			|| (key == KeyboardKey::KeyQ && modifiers == ctrl))
		{
			_runResult = AppRunResult::Exit;
			return;
		}
		if ((key == KeyboardKey::Return && modifiers == alt)
			|| (key == KeyboardKey::KeyF && modifiers == ctrl))
		{
			requestNextVideoMode();
			return;
		}
		if (key == KeyboardKey::F1)
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
			return;
		}
		if (key == KeyboardKey::F2 && modifiers.empty())
		{
			toggleDebugFlag(BGFX_DEBUG_TEXT);
			return;
		}
		if (key == KeyboardKey::F3 && modifiers.empty())
		{
			toggleDebugFlag(BGFX_DEBUG_WIREFRAME);
			return;
		}
		if (key == KeyboardKey::F4 && modifiers.empty())
		{
			toggleDebugFlag(BGFX_DEBUG_PROFILER);
			return;
		}
		if ((key == KeyboardKey::F5 && modifiers.empty())
			|| (key == KeyboardKey::KeyR && modifiers == ctrl))
		{
			_runResult = AppRunResult::Restart;
			return;
		}
		if ((key == KeyboardKey::F5 && modifiers == ctrl))
		{
			setNextRenderer();
			return;
		}
		if ((key == KeyboardKey::Print && modifiers.empty())
			|| (key == KeyboardKey::KeyP && modifiers == ctrl))
		{
			auto filePath = "temp/screenshot-" + StringUtils::getTimeSuffix();
			bgfx::requestScreenShot(BGFX_INVALID_HANDLE, filePath.c_str());
			return;
		}
		if (key == KeyboardKey::Print && modifiers == ctrl)
		{
			toggleTaskflowProfile();
			return;
		}
		if (key == KeyboardKey::Pause)
		{
			_paused = !_paused;
			return;
		}
	}

	void AppImpl::toggleTaskflowProfile()
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
		std::filesystem::path basePath = "temp";
		auto suffix = StringUtils::getTimeSuffix();
		{
			auto filePath = basePath / ("taskflow-" + suffix + ".json");
			std::ofstream out(filePath);
			out << "[";
			_taskObserver->dump(out);
			_taskObserver.reset();
			out << "]";
		}

		{
			auto filePath = basePath / ("rendergraph-" + suffix + ".graphviz");
			std::ofstream out(filePath);
			_renderGraph->dump(out);
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

	AppRunResult AppImpl::processEvents()
	{
		while (_runResult == AppRunResult::Continue)
		{
			auto patEv = getPlatform().pollEvent();
			if (patEv == nullptr)
			{
				break;
			}
			PlatformEvent::process(*patEv, _input, _window);
			if (_window.getPhase() == WindowPhase::Destroyed)
			{
				return AppRunResult::Exit;
			}
		};
		return _runResult;
	}

	void AppImpl::addComponent(entt::id_type type, std::unique_ptr<IAppComponent>&& component) noexcept
	{
		if (_running)
		{
			component->init(_app);
		}
		_components.emplace_back(type, std::move(component));
	}

	AppImpl::Components::iterator AppImpl::findComponent(entt::id_type type) noexcept
	{
		return std::find_if(_components.begin(), _components.end(), [type](auto& elm) { return elm.first == type; });
	}

	AppImpl::Components::const_iterator AppImpl::findComponent(entt::id_type type) const noexcept
	{
		return std::find_if(_components.begin(), _components.end(), [type](auto& elm) { return elm.first == type; });
	}

	bool AppImpl::removeComponent(entt::id_type type) noexcept
	{
		auto itr = findComponent(type);
		if (itr == _components.end())
		{
			return false;
		}
		_components.erase(itr);
		return true;
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
		return itr->second.get();
	}

	OptionalRef<const IAppComponent> AppImpl::getComponent(entt::id_type type) const noexcept
	{
		auto itr = findComponent(type);
		if (itr == _components.end())
		{
			return nullptr;
		}
		return itr->second.get();
	}

	App::App(std::unique_ptr<IAppDelegateFactory>&& factory) noexcept
		: _impl(std::make_unique<AppImpl>(*this, std::move(factory)))
	{
	}

	App::~App() noexcept
	{
		// intentionally left blank for the unique_ptr<AppImpl> forward declaration
	}

	std::optional<int32_t> App::setup(const std::vector<std::string>& args)
	{
		return _impl->setup(args);
	}

	void App::init()
	{
		_impl->processEvents();
		_impl->init();
	}

	void App::onException(Phase phase, const std::exception& ex) noexcept
	{
		std::stringstream ss("[DARMOK] exception running app ");
		switch (phase)
		{
		case Phase::Init:
			ss << "init";
			break;
		case Phase::Update:
			ss << "update";
			break;
		case Phase::Shutdown:
			ss << "shutdown";
			break;
		}
		ss << ": " << ex.what();
		StreamUtils::logDebug(ss.str());
	}

	void App::shutdown()
	{
		_impl->shutdown();
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

	RenderGraphDefinition& App::getRenderGraph() noexcept
	{
		return _impl->getRenderGraph();
	}

	const RenderGraphDefinition& App::getRenderGraph() const noexcept
	{
		return _impl->getRenderGraph();
	}

	tf::Executor& App::getTaskExecutor()
	{
		return _impl->getTaskExecutor();
	}

	const tf::Executor& App::getTaskExecutor() const
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

	AppRunResult App::run()
	{
		auto result = _impl->processEvents();
		if (result != AppRunResult::Continue)
		{
			return result;
		}

		_impl->deltaTimeCall([this](float deltaTime) {
			_impl->update(deltaTime);
			update(deltaTime);
			_impl->afterUpdate(deltaTime);
		});

		render();

		// advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		return AppRunResult::Continue;
	}

	void App::update(float deltaTime)
	{
	}

	void App::setUpdateConfig(const AppUpdateConfig& config) noexcept
	{
		_impl->setUpdateConfig(config);
	}

	void App::render() const
	{
		bgfx::touch(0);

		_impl->render();

		bgfx::dbgTextClear(); // use debug font to print information
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

	void App::setRendererType(bgfx::RendererType::Enum renderer)
	{
		_impl->setRendererType(renderer);
	}

	void App::addComponent(entt::id_type type, std::unique_ptr<IAppComponent>&& component) noexcept
	{
		_impl->addComponent(type, std::move(component));
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

	BgfxFatalException::BgfxFatalException(const char* filePath, uint16_t line, bgfx::Fatal::Enum code, const char* msg)
		: std::exception(msg)
		, filePath(filePath)
		, line(line)
		, code(code)
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
	)
	{
		if (bgfx::Fatal::DebugCheck == code)
		{
			bx::debugBreak();
			return;
		}
		throw BgfxFatalException(filePath, line, code, str);
	}

	void BgfxCallbacks::traceVargs(
		const char* filePath
		, uint16_t line
		, const char* format
		, va_list argList
	)
	{
		auto str = StringUtils::vsprintf(format, argList);
		bx::debugOutput(bx::StringView(str.data(), str.size()));
	}

	// bgfx tracy integration problems
	// https://github.com/bkaradzic/bgfx/pull/3308

	void BgfxCallbacks::profilerBegin(
		const char* name
		, uint32_t abgr
		, const char* filePath
		, uint16_t line
	)
	{
		// TODO: vcpkg bgfx build without profiler enabled
	}

	void BgfxCallbacks::profilerBeginLiteral(
		const char* name
		, uint32_t abgr
		, const char* filePath
		, uint16_t line
	)
	{
		// TODO: vcpkg bgfx build without profiler enabled
	}

	void BgfxCallbacks::profilerEnd()
	{
		// TODO: vcpkg bgfx build without profiler enabled
	}

	uint32_t BgfxCallbacks::cacheReadSize(uint64_t id)
	{
		std::scoped_lock lock(_cacheMutex);
		auto itr = _cache.find(id);
		if (itr == _cache.end())
		{
			return 0;
		}
		return itr->second.size();
	}

	bool BgfxCallbacks::cacheRead(uint64_t id, void* dataPtr, uint32_t size)
	{
		std::scoped_lock lock(_cacheMutex);
		auto itr = _cache.find(id);
		if (itr == _cache.end())
		{
			return false;
		}
		auto& data = itr->second;
		if (data.size() < size)
		{
			size = data.size();
		}
		std::memcpy(dataPtr, data.ptr(), size);
		return true;
	}

	void BgfxCallbacks::cacheWrite(uint64_t id, const void* data, uint32_t size)
	{
		_cache.emplace(id, Data(data, size));
	}

	void BgfxCallbacks::screenShot(
		const char* filePath
		, uint32_t width
		, uint32_t height
		, uint32_t pitch
		, const void* data
		, uint32_t size
		, bool yflip
	)
	{
		std::filesystem::path path(filePath);
		auto ext = StringUtils::getFileExt(path.filename().string());
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
			checkError(err);
		}
	}

	void BgfxCallbacks::captureBegin(
		uint32_t width
		, uint32_t height
		, uint32_t pitch
		, bgfx::TextureFormat::Enum format
		, bool yflip
	)
	{
		// TODO
	}

	void BgfxCallbacks::captureEnd()
	{
		// TODO
	}

	void BgfxCallbacks::captureFrame(const void* data, uint32_t size)
	{
		// TODO
	}
}
