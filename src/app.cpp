
#include "app.hpp"
#include "platform.hpp"
#include "input.hpp"
#include "window.hpp"
#include <darmok/app.hpp>
#include <bx/filepath.h>
#include <bx/timer.h>


#if BX_PLATFORM_EMSCRIPTEN
#	include <emscripten.h>
#endif // BX_PLATFORM_EMSCRIPTEN

namespace darmok
{
	AppImpl::AppImpl() noexcept
		: _exit(false)
		, _debug(BGFX_DEBUG_NONE)
		, _lastUpdate(0)
		, _config(AppConfig::defaultConfig)
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

	const AppConfig AppConfig::defaultConfig =
	{
		1.0F / 30.0F,
		10,
	};

	float AppImpl::updateTimePassed() noexcept
	{
		const int64_t now = bx::getHPCounter();
		const float timePassed = (now - _lastUpdate) / double(bx::getHPFrequency());
		_lastUpdate = bx::getHPCounter();
		return timePassed;
	}

	void AppImpl::configure(const AppConfig& config) noexcept
	{
		_config = config;
	}

	void AppImpl::init(App& app, const std::vector<std::string>& args)
	{
		bx::FilePath filePath(args[0].c_str());
		auto basePath = filePath.getPath();
		_currentDir = std::string(basePath.getPtr(), basePath.getLength());

		addBindings();

		for (auto& component : _components)
		{
			component->init(app);
		}

		_lastUpdate = bx::getHPCounter();
		_app = app;
	}

	void AppImpl::shutdown()
	{
		for (auto& component : _components)
		{
			component->shutdown();
		}
		_components.clear();
		_app.reset();;
	}

	void AppImpl::updateLogic(float deltaTime)
	{
		_input.getImpl().update();

		for (auto& component : _components)
		{
			component->updateLogic(deltaTime);
		}
	}

	bgfx::ViewId AppImpl::render(bgfx::ViewId viewId)
	{
		for (auto& component : _components)
		{
			viewId = component->render(viewId);
		}
		return viewId;
	}

	const std::string AppImpl::_bindingsName = "main";

	const std::string& AppImpl::getCurrentDir() const noexcept
	{
		return _currentDir;
	}

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
			auto filePath = getCurrentDir() + "temp/screenshot-" + std::to_string(timeVal);
			bgfx::requestScreenShot(BGFX_INVALID_HANDLE, filePath.c_str());
		};

		_input.addBindings(_bindingsName, {
			{ KeyboardBindingKey { KeyboardKey::Esc,		KeyboardModifiers::None },			true, exit },
			{ KeyboardBindingKey { KeyboardKey::KeyQ,		KeyboardModifiers::LeftCtrl	},		true, exit },
			{ KeyboardBindingKey { KeyboardKey::KeyQ,		KeyboardModifiers::RightCtrl },		true, exit },
			{ KeyboardBindingKey { KeyboardKey::KeyF,		KeyboardModifiers::LeftCtrl	},		true, fullscreen },
			{ KeyboardBindingKey { KeyboardKey::KeyF,		KeyboardModifiers::RightCtrl },		true, fullscreen },
			{ KeyboardBindingKey { KeyboardKey::Return,		KeyboardModifiers::LeftAlt },		true, fullscreen },
			{ KeyboardBindingKey { KeyboardKey::Return,		KeyboardModifiers::RightAlt },		true, fullscreen },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::None },			true, debugStats },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::LeftAlt },		true, debugText },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::RightAlt },		true, debugText },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::LeftCtrl	},		true, debugIfh },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::RightCtrl },		true, debugIfh },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::LeftShift },		true, disableDebug },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::RightShift },	true, disableDebug },
			{ KeyboardBindingKey { KeyboardKey::F3,			KeyboardModifiers::None},			true, debugWireframe },
			{ KeyboardBindingKey { KeyboardKey::F6,			KeyboardModifiers::None},			true, debugProfiler },
			{ KeyboardBindingKey { KeyboardKey::Print,		KeyboardModifiers::None},			true, screenshot },
			{ KeyboardBindingKey { KeyboardKey::KeyP,		KeyboardModifiers::LeftCtrl },		true, screenshot },
			{ KeyboardBindingKey { KeyboardKey::KeyP,		KeyboardModifiers::RightCtrl },		true, screenshot },
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
			_input.process();
		};

		return _exit;
	}

	void AppImpl::addComponent(std::unique_ptr<AppComponent>&& component) noexcept
	{
		if (_app.hasValue())
		{
			component->init(_app.value());
		}
		_components.push_back(std::move(component));
	}

#if BX_PLATFORM_EMSCRIPTEN
	static App* s_app;
	static void updateApp()
	{
		s_app->update();
	}
#endif // BX_PLATFORM_EMSCRIPTEN

	int runApp(App& app, const std::vector<std::string>& args)
	{
		app.init(args);

#if BX_PLATFORM_EMSCRIPTEN
		s_app = _app;
		emscripten_set_main_loop(&updateApp, -1, 1);
#else
		while (app.update())
		{
		}
#endif // BX_PLATFORM_EMSCRIPTEN

		return app.shutdown();
	}

	int main(int argc, const char* const* argv)
	{
		//DBG(BX_COMPILER_NAME " / " BX_CPU_NAME " / " BX_ARCH_NAME " / " BX_PLATFORM_NAME);
		return ::_main_(argc, (char**)argv);
	}

	App::App() noexcept
		: _impl(std::make_unique<AppImpl>())
	{
	}

	App::~App() noexcept
	{
		// intentionally left blank for the unique_ptr<AppImpl> forward declaration
	}

	void App::init(const std::vector<std::string>& args)
	{
		_impl->processEvents();

		bgfx::Init init;
		const auto& size = _impl->getWindow().getSize();
		auto& plat = _impl->getPlatform();
		init.platformData.ndt = plat.getDisplayHandle();
		init.platformData.nwh = plat.getWindowHandle();
		init.platformData.type = plat.getWindowHandleType();
		init.debug = true;
		init.resolution.width = size.x;
		init.resolution.height = size.y;
		// init.resolution.reset = ?;
		bgfx::init(init);

		bgfx::setPaletteColor(0, UINT32_C(0x00000000));
		bgfx::setPaletteColor(1, UINT32_C(0x303030ff));

		_impl->init(*this, args);
	}

	int App::shutdown()
	{
		_impl->shutdown();
		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
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
		// TODO: will be called twice?
		getWindow().bgfxConfig(viewId);

		viewId = render(viewId);
		viewId = _impl->render(viewId);

		// advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		return true;
	}


	void App::updateLogic(float deltaTime)
	{
	}

	void App::configure(const AppConfig& config) noexcept
	{
		_impl->configure(config);
	}

	bgfx::ViewId App::render(bgfx::ViewId viewId)
	{
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

	void AppComponent::init(App& app)
	{
	}

	void AppComponent::shutdown()
	{
	}

	void AppComponent::updateLogic(float deltaTime)
	{
	} 

	bgfx::ViewId AppComponent::render(bgfx::ViewId viewId)
	{
		return viewId;
	}
}
