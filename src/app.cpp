
#include "app.hpp"
#include "platform.hpp"
#include "input.hpp"
#include "window.hpp"
#include <darmok/app.hpp>
#include <darmok/color.hpp>
#include <darmok/stream.hpp>

#include <bx/filepath.h>
#include <bx/timer.h>
#include <iostream>
#include <algorithm>

#if BX_PLATFORM_EMSCRIPTEN
#	include <emscripten.h>
#endif // BX_PLATFORM_EMSCRIPTEN

namespace darmok
{
	int32_t main(int32_t argc, const char* const* argv, std::unique_ptr<App>&& app)
	{
		try
		{
			std::vector<std::string> args(argc);
			for (int i = 0; i < argc; ++i)
			{
				args[i] = argv[i];
			}
			auto r = app->setup(args);
			if (r)
			{
				return r.value();
			}
		}
		catch (const std::exception& ex)
		{
			app->onException(App::Phase::Setup, ex);
			return -1;
		}
		return Platform::get().run(std::make_unique<AppRunner>(std::move(app)));
	}

#if BX_PLATFORM_EMSCRIPTEN
	static App* s_app;
	static void emscriptenUpdateApp()
	{
		s_app->update();
	}
#endif // BX_PLATFORM_EMSCRIPTEN

	AppRunner::AppRunner(std::unique_ptr<App>&& app) noexcept
		: _app(std::move(app))
	{
	}

	int AppRunner::operator()() noexcept
	{
		bool success = false;
		if (init())
		{
			if (update())
			{
				success = true;
			}
		}
		if (!shutdown())
		{
			success = false;
		}

		// destroy app before the bgfx shutdown to guarantee no dangling resources
		_app.reset();

		// Shutdown bgfx.
		bgfx::shutdown();

		return success ? 0 : -1;
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

	bool AppRunner::update() noexcept
	{
		try
		{
#if BX_PLATFORM_EMSCRIPTEN
			s_app = app.get();
			emscripten_set_main_loop(&emscriptenUpdateApp, -1, 1);
#else
			while (_app->update())
			{
			}
#endif // BX_PLATFORM_EMSCRIPTEN
			return true;
		}
		catch (const std::exception& ex)
		{
			_app->onException(App::Phase::Update, ex);
			return false;
		}
	}

	bool AppRunner::shutdown() noexcept
	{
		try
		{
			_app->shutdown();
			return true;
		}
		catch (const std::exception& ex)
		{
			_app->onException(App::Phase::Shutdown, ex);
			return false;
		}
	}

	AppImpl::AppImpl(App& app) noexcept
		: _app(app)
		, _exit(false)
		, _running(false)
		, _debug(BGFX_DEBUG_NONE)
		, _lastUpdate(0)
		, _config(AppConfig::getDefaultConfig())
		, _plat(Platform::get())
		, _window(_plat)
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

	const AppConfig& AppConfig::getDefaultConfig() noexcept
	{
		static const AppConfig config
		{
			1.0F / 30.0F,
			10,
			Colors::fromNumber(0x303030ff)
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

	void AppImpl::setConfig(const AppConfig& config) noexcept
	{
		_config = config;
	}

	void AppImpl::init()
	{
		bgfx::Init init;
		const auto& size = _window.getSize();
		init.platformData.ndt = _plat.getDisplayHandle();
		init.platformData.nwh = _plat.getWindowHandle();
		init.platformData.type = _plat.getWindowHandleType();
		init.debug = true;
		init.resolution.width = size.x;
		init.resolution.height = size.y;
		// init.resolution.reset = ?;
		// init.type = bgfx::RendererType::Vulkan;
		bgfx::init(init);

		bgfx::setPaletteColor(0, UINT32_C(0x00000000));
		//bgfx::setPaletteColor(1, UINT32_C(0x303030ff));
		bgfx::setPaletteColor(1, Colors::toNumber(_config.clearColor));

		addBindings();

		for (auto& elm : _sharedComponents)
		{
			elm.second->init(_app);
		}
		for (auto& component : _components)
		{
			component->init(_app);
		}

		_lastUpdate = bx::getHPCounter();
		_running = true;
	}

	void AppImpl::shutdown()
	{
		_running = false;
		for (auto& component : _components)
		{
			component->shutdown();
		}
		for (auto& elm : _sharedComponents)
		{
			elm.second->shutdown();
		}
		_components.clear();
		_sharedComponents.clear();
		_input.getImpl().clearBindings();
	}

	void AppImpl::updateLogic(float deltaTime)
	{
		cleanSharedComponents();
		for (auto& elm : _sharedComponents)
		{
			elm.second->updateLogic(deltaTime);
		}
		for (auto& component : _components)
		{
			component->updateLogic(deltaTime);
		}
		_input.getImpl().update();
	}

	bgfx::ViewId AppImpl::render(bgfx::ViewId viewId) const
	{
		for (auto& elm : _sharedComponents)
		{
			viewId = elm.second->render(viewId);
		}
		for (auto& component : _components)
		{
			viewId = component->render(viewId);
		}
		return viewId;
	}

	const std::string AppImpl::_bindingsName = "debug";

	void AppImpl::removeBindings() noexcept
	{
		_input.removeBindings(_bindingsName);
	}

	static uint32_t setFlag(uint32_t flags, uint32_t flag, bool enabled) noexcept
	{
		if (enabled)
		{
			return flags | flag;
		}
		return flags & ~flag;
	}

	void AppImpl::addBindings() noexcept
	{
		auto exit = [this]() { triggerExit(); };
		auto fullscreen = [this]() {
			auto mode = (WindowMode)((to_underlying(_window.getMode()) + 1) % to_underlying(WindowMode::Count));
			_window.requestMode(mode);
		};
		auto debugStats = [this]() { toggleDebugFlag(BGFX_DEBUG_STATS); };
		auto debugText = [this]() { toggleDebugFlag(BGFX_DEBUG_TEXT); };
		auto debugIfh = [this]() { toggleDebugFlag(BGFX_DEBUG_IFH); };
		auto debugWireframe = [this]() { toggleDebugFlag(BGFX_DEBUG_WIREFRAME); };
		auto debugProfiler = [this]() { toggleDebugFlag(BGFX_DEBUG_PROFILER); };
		auto disableDebug = [this]() { 
			setDebugFlag(BGFX_DEBUG_STATS, false);
			setDebugFlag(BGFX_DEBUG_TEXT, false);
		};
		auto screenshot = [this]() {
			time_t timeVal;
			time(&timeVal);
			auto filePath = "temp/screenshot-" + std::to_string(timeVal);
			bgfx::requestScreenShot(BGFX_INVALID_HANDLE, filePath.c_str());
		};

		auto reloadApp = [this]() {
			_app.shutdown();
			_app.init();
		};

		_input.addBindings(_bindingsName, {
			{ KeyboardBindingKey { KeyboardKey::Esc,		to_underlying(KeyboardModifier::None) },		true, exit },
			{ KeyboardBindingKey { KeyboardKey::KeyQ,		to_underlying(KeyboardModifier::LeftCtrl) },	true, exit },
			{ KeyboardBindingKey { KeyboardKey::KeyQ,		to_underlying(KeyboardModifier::RightCtrl) },	true, exit },
			{ KeyboardBindingKey { KeyboardKey::KeyF,		to_underlying(KeyboardModifier::LeftCtrl) },	true, fullscreen },
			{ KeyboardBindingKey { KeyboardKey::KeyF,		to_underlying(KeyboardModifier::RightCtrl) },	true, fullscreen },
			{ KeyboardBindingKey { KeyboardKey::Return,		to_underlying(KeyboardModifier::LeftAlt) },		true, fullscreen },
			{ KeyboardBindingKey { KeyboardKey::Return,		to_underlying(KeyboardModifier::RightAlt) },	true, fullscreen },
			{ KeyboardBindingKey { KeyboardKey::F1,			to_underlying(KeyboardModifier::None) },		true, debugStats },
			{ KeyboardBindingKey { KeyboardKey::F1,			to_underlying(KeyboardModifier::LeftAlt) },		true, debugText },
			{ KeyboardBindingKey { KeyboardKey::F1,			to_underlying(KeyboardModifier::RightAlt) },	true, debugText },
			{ KeyboardBindingKey { KeyboardKey::F1,			to_underlying(KeyboardModifier::LeftCtrl) },	true, debugIfh },
			{ KeyboardBindingKey { KeyboardKey::F1,			to_underlying(KeyboardModifier::RightCtrl) },	true, debugIfh },
			{ KeyboardBindingKey { KeyboardKey::F1,			to_underlying(KeyboardModifier::LeftShift) },	true, disableDebug },
			{ KeyboardBindingKey { KeyboardKey::F1,			to_underlying(KeyboardModifier::RightShift) },	true, disableDebug },
			{ KeyboardBindingKey { KeyboardKey::F3,			to_underlying(KeyboardModifier::None) },		true, debugWireframe },
			{ KeyboardBindingKey { KeyboardKey::F5,			to_underlying(KeyboardModifier::None) },		true, reloadApp },
			{ KeyboardBindingKey { KeyboardKey::F6,			to_underlying(KeyboardModifier::None) },		true, debugProfiler },
			{ KeyboardBindingKey { KeyboardKey::Print,		to_underlying(KeyboardModifier::None) },		true, screenshot },
			{ KeyboardBindingKey { KeyboardKey::KeyP,		to_underlying(KeyboardModifier::LeftCtrl) },	true, screenshot },
			{ KeyboardBindingKey { KeyboardKey::KeyP,		to_underlying(KeyboardModifier::RightCtrl) },	true, screenshot },
			{ KeyboardBindingKey { KeyboardKey::KeyR,		to_underlying(KeyboardModifier::LeftCtrl) },	true, reloadApp },
			{ KeyboardBindingKey { KeyboardKey::KeyR,		to_underlying(KeyboardModifier::RightCtrl) },	true, reloadApp },
			});
	}

	bool AppImpl::toggleDebugFlag(uint32_t flag) noexcept
	{
		auto value = !getDebugFlag(flag);
		setDebugFlag(flag, value);
		return value;
	}

	void AppImpl::setDebugFlag(uint32_t flag, bool enabled) noexcept
	{
		_debug = setFlag(_debug, flag, enabled);
		bgfx::setDebug(_debug);
	}

	bool AppImpl::getDebugFlag(uint32_t flag) const noexcept
	{
		return static_cast<bool>(_debug & flag);
	}

	void AppImpl::triggerExit() noexcept
	{
		_exit = true;
	}

	bool AppImpl::processEvents()
	{
		while (!_exit)
		{
			auto patEv = getPlatform().pollEvent();
			if (patEv == nullptr)
			{
				break;
			}
			PlatformEvent::process(*patEv, _input, _window);
			if (_window.getPhase() == WindowPhase::Destroyed)
			{
				return true;
			}
		};
		_input.processBindings();
		return _exit;
	}
	std::shared_ptr<AppComponent> AppImpl::getSharedComponent(size_t typeHash, SharedAppComponentCreationCallback callback)
	{
		cleanSharedComponents();
		auto itr = _sharedComponents.find(typeHash);
		if (itr != _sharedComponents.end())
		{
			return itr->second;
		}
		auto component = std::shared_ptr<AppComponent>(callback());
		auto r = _sharedComponents.emplace(typeHash, component);
		if (_running)
		{
			component->init(_app);
		}
		return component;
	}

	void AppImpl::cleanSharedComponents() noexcept
	{
		for (auto itr = _sharedComponents.begin(), last = _sharedComponents.end(); itr != last;)
		{
			if (itr->second.use_count() == 1)
			{
				itr = _sharedComponents.erase(itr);
			}
			else
			{
				++itr;
			}
		}
	}

	void AppImpl::addComponent(std::unique_ptr<AppComponent>&& component) noexcept
	{
		if (_running)
		{
			component->init(_app);
		}
		_components.push_back(std::move(component));
	}

	bool AppImpl::removeComponent(AppComponent& component) noexcept
	{
		auto ptr = &component;
		auto itr1 = std::find_if(_components.begin(), _components.end(), [ptr](auto& comp) { return comp.get() == ptr;  });
		auto itr2 = std::find_if(_sharedComponents.begin(), _sharedComponents.end(), [ptr](auto& elm) { return elm.second.get() == ptr; });
		auto found = itr1 != _components.end() || itr2 != _sharedComponents.end();
		if (_running && found)
		{
			component.shutdown();
		}
		if (itr1 != _components.end())
		{
			_components.erase(itr1);
		}
		if (itr2 != _sharedComponents.end())
		{
			_sharedComponents.erase(itr2);
		}
		return found;
	}

	App::App() noexcept
		: _impl(std::make_unique<AppImpl>(*this))
	{
	}

	App::~App() noexcept
	{
		// intentionally left blank for the unique_ptr<AppImpl> forward declaration
	}

	std::optional<int32_t> App::setup(const std::vector<std::string>& args)
	{
		return std::nullopt;
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

	bool App::update()
	{
		if (_impl->processEvents())
		{
			return false;
		}

		_impl->update([this](float deltaTime) {
			updateLogic(deltaTime);
			_impl->updateLogic(deltaTime);
		});

		bgfx::ViewId viewId = 0;
		viewId = render(viewId);

		// advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		return true;
	}

	std::shared_ptr<AppComponent> App::getSharedComponent(size_t typeHash, SharedAppComponentCreationCallback callback)
	{
		return _impl->getSharedComponent(typeHash, callback);
	}

	void App::updateLogic(float deltaTime)
	{
	}

	void App::setConfig(const AppConfig& config) noexcept
	{
		_impl->setConfig(config);
	}

	bgfx::ViewId App::render(bgfx::ViewId viewId) const
	{
		auto& size = getWindow().getSize();

		// initial view 0 to clear the screen
		bgfx::setViewRect(viewId, 0, 0, size.x, size.y);
		bgfx::touch(viewId);
		bgfx::dbgTextClear(); // use debug font to print information
		bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH | BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL, 1.F, 0U, 1);
		viewId++;

		viewId = _impl->render(viewId);

		bgfx::setViewRect(viewId, 0, 0, size.x, size.y);
		bgfx::touch(viewId);

		return viewId;
	}

	void App::toggleDebugFlag(uint32_t flag) noexcept
	{
		_impl->toggleDebugFlag(flag);
	}

	void App::setDebugFlag(uint32_t flag, bool enabled) noexcept
	{
		_impl->setDebugFlag(flag, enabled);
	}

	void App::addComponent(std::unique_ptr<AppComponent>&& component) noexcept
	{
		_impl->addComponent(std::move(component));
	}

	bool App::removeComponent(AppComponent& component) noexcept
	{
		return _impl->removeComponent(component);
	}
}
