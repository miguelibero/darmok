
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
		: _init(false)
		, _exit(false)
		, _debug(BGFX_DEBUG_NONE)
		, _reset(BGFX_RESET_VSYNC)
		, _mainWindowSuspended(false)
		, _needsReset(false)
		, _lastUpdate(0)
		, _targetUpdateDeltaTime(0.)
	{
	}

	const float AppImpl::defaultTargetUpdateDeltaTime = 1.0F / 30.0F;
	const int AppImpl::maxInstantLogicUpdates = 10;

	float AppImpl::updateTimePassed() noexcept
	{
		const int64_t now = bx::getHPCounter();
		const float timePassed = (now - _lastUpdate) / double(bx::getHPFrequency());
		_lastUpdate = bx::getHPCounter();
		return timePassed;
	}

	void AppImpl::init(App& app, const std::vector<std::string>& args, double targetUpdateDeltaTime)
	{
		_viewWindows.clear();
		_viewWindows[0] = Window::DefaultHandle;

		bx::FilePath filePath(args[0].c_str());
		auto basePath = filePath.getPath();

		setCurrentDir(std::string(basePath.getPtr(), basePath.getLength()));
		addBindings();

		for (auto& component : _appComponents)
		{
			component->init();
		}
		for(auto& elm : _viewComponents)
		{
			for (auto& component : elm.second)
			{
				component->init(elm.first);
			}
		}

		_lastUpdate = bx::getHPCounter();
		_targetUpdateDeltaTime = targetUpdateDeltaTime;
		_init = true;
	}

	void AppImpl::shutdown()
	{
		for (auto& component : _appComponents)
		{
			component->shutdown();
		}
		_appComponents.clear();
		for (auto& elm : _viewComponents)
		{
			for (auto& component : elm.second)
			{
				component->shutdown();
			}
		}
		_viewComponents.clear();
		_init = false;
	}

	void AppImpl::updateLogic(float deltaTime)
	{
		Input::get().getImpl().update();

		for (auto& component : _appComponents)
		{
			component->updateLogic(deltaTime);
		}
		for (auto& elm : _viewComponents)
		{
			for (auto& component : elm.second)
			{
				component->updateLogic(deltaTime);
			}
		}
	}

	void AppImpl::beforeWindowRender(const WindowHandle& window, bgfx::ViewId viewId)
	{
	}

	void AppImpl::beforeRender(bgfx::ViewId viewId)
	{
		for (auto& component : _appComponents)
		{
			component->beforeRender(viewId);
		}
		auto itr = _viewComponents.find(viewId);
		if (itr != _viewComponents.end())
		{
			for (auto& component : itr->second)
			{
				component->beforeRender();
			}
		}
	}

	void AppImpl::render(bgfx::ViewId viewId)
	{
		for (auto& component : _appComponents)
		{
			component->render(viewId);
		}
		auto itr = _viewComponents.find(viewId);
		if (itr != _viewComponents.end())
		{
			for (auto& component : itr->second)
			{
				component->render();
			}
		}
	}

	void AppImpl::afterRender(bgfx::ViewId viewId)
	{
		for (auto& component : _appComponents)
		{
			component->afterRender(viewId);
		}
		auto itr = _viewComponents.find(viewId);
		if (itr != _viewComponents.end())
		{
			for (auto& component : itr->second)
			{
				component->afterRender();
			}
		}
	}

	void AppImpl::afterWindowRender(const WindowHandle& window, bgfx::ViewId lastViewId)
	{
	}

	const std::string AppImpl::_bindingsName = "main";

	void AppImpl::setCurrentDir(const std::string& dir) noexcept
	{
		_currentDir = dir;
	}

	const std::string& AppImpl::getCurrentDir() const noexcept
	{
		return _currentDir;
	}

	void AppImpl::removeBindings() noexcept
	{
		Input::get().removeBindings(_bindingsName);
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
		auto fullscreen = []() { WindowContext::get().getWindow().toggleFullscreen(); };
		auto debugStats = [this]() { toggleDebugFlag(BGFX_DEBUG_STATS); };
		auto debugText = [this]() { toggleDebugFlag(BGFX_DEBUG_TEXT); };
		auto debugIfh = [this]() { toggleDebugFlag(BGFX_DEBUG_IFH); };
		auto debugWireframe = [this]() { toggleDebugFlag(BGFX_DEBUG_WIREFRAME); };
		auto debugProfiler = [this]() { toggleDebugFlag(BGFX_DEBUG_PROFILER); };
		auto disableDebug = [this]() { 
			setDebugFlag(BGFX_DEBUG_STATS, false);
			setDebugFlag(BGFX_DEBUG_TEXT, false);
		};
		auto resetVsync = [this]() { toggleResetFlag(BGFX_RESET_VSYNC); };
		auto resetMsaa = [this]() { toggleResetFlag(BGFX_RESET_MSAA_X16); };
		auto resetFlush = [this]() { toggleResetFlag(BGFX_RESET_FLUSH_AFTER_RENDER); };
		auto resetFlip = [this]() { toggleResetFlag(BGFX_RESET_FLIP_AFTER_RENDER); };
		auto resetHidpi = [this]() { toggleResetFlag(BGFX_RESET_HIDPI); };
		auto resetDepthClamp = [this]() { toggleResetFlag(BGFX_RESET_DEPTH_CLAMP); };
		auto screenshot = [this]() {
			time_t timeVal;
			time(&timeVal);
			auto filePath = getCurrentDir() + "temp/screenshot-" + std::to_string(timeVal);
			WindowContext::get().getWindow().requestScreenshot(filePath);
		};

		Input::get().addBindings(_bindingsName, {
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
			{ KeyboardBindingKey { KeyboardKey::F2,			KeyboardModifiers::None},			true, resetFlip },
			{ KeyboardBindingKey { KeyboardKey::F3,			KeyboardModifiers::None},			true, debugWireframe },
			{ KeyboardBindingKey { KeyboardKey::F4,			KeyboardModifiers::None},			true, resetDepthClamp },
			{ KeyboardBindingKey { KeyboardKey::F6,			KeyboardModifiers::None},			true, debugProfiler },
			{ KeyboardBindingKey { KeyboardKey::F7,			KeyboardModifiers::None},			true, resetVsync },
			{ KeyboardBindingKey { KeyboardKey::F8,			KeyboardModifiers::None},			true, resetMsaa },
			{ KeyboardBindingKey { KeyboardKey::F9,			KeyboardModifiers::None},			true, resetFlush },
			{ KeyboardBindingKey { KeyboardKey::F10,		KeyboardModifiers::None},			true, resetHidpi },
			{ KeyboardBindingKey { KeyboardKey::Print,		KeyboardModifiers::None},			true, screenshot },
			{ KeyboardBindingKey { KeyboardKey::KeyP,		KeyboardModifiers::LeftCtrl },		true, screenshot },
			{ KeyboardBindingKey { KeyboardKey::KeyP,		KeyboardModifiers::RightCtrl },		true, screenshot },
			});
	}

	bool AppImpl::toggleResetFlag(uint32_t flag) noexcept
	{
		auto value = !getResetFlag(flag);
		setResetFlag(flag, value);
		return value;
	}

	void AppImpl::setResetFlag(uint32_t flag, bool enabled) noexcept
	{
		auto reset = setFlag(_reset, flag, enabled);
		if (_reset != reset)
		{
			_reset = reset;
			_needsReset = true;
		}
	}

	bool AppImpl::getResetFlag(uint32_t flag) const noexcept
	{
		return static_cast<bool>(_reset & flag);
	}

	uint32_t AppImpl::getResetFlags() const noexcept
	{
		return _reset;
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
		bool needsReset = false;
		while (!_exit)
		{
			auto patEv = PlatformContext::get().pollEvent();
			if (patEv == nullptr)
			{
				break;
			}
			auto result = PlatformEvent::process(*patEv);
			if (result == PlatformEvent::Result::Exit)
			{
				return true;
			}
			if (result == PlatformEvent::Result::Reset)
			{
				needsReset = true;
			}
			Input::get().process();
		};

		if (needsReset || _needsReset)
		{
			const auto& size = WindowContext::get().getWindow().getSize();
			bgfx::reset(size.x, size.y, getResetFlags());
			Input::get().getMouse().getImpl().setResolution(size);
			_needsReset = false;
		}

		return _exit;
	}

	void AppImpl::addComponent(std::unique_ptr<AppComponent>&& component) noexcept
	{
		if (_init)
		{
			component->init();
		}
		_appComponents.push_back(std::move(component));
	}

	void AppImpl::addViewComponent(bgfx::ViewId viewId, std::unique_ptr<ViewComponent>&& component) noexcept
	{
		if (_init)
		{
			component->init(viewId);
		}
		_viewComponents[viewId].push_back(std::move(component));
	}

	void AppImpl::setWindowView(bgfx::ViewId viewId, const WindowHandle& window) noexcept
	{
		_viewWindows[viewId] = window;
	}

	WindowHandle AppImpl::getViewWindow(bgfx::ViewId viewId) const noexcept
	{
		auto itr = _viewWindows.find(viewId);
		if (itr == _viewWindows.end())
		{
			return Window::InvalidHandle;
		}
		return itr->second;
	}

	const ViewWindows& AppImpl::getViewWindows() const noexcept
	{
		return _viewWindows;
	}

	std::vector<bgfx::ViewId> AppImpl::getWindowViews(const WindowHandle& window) const noexcept
	{
		std::vector<bgfx::ViewId> views;
		for (const auto& pair : _viewWindows)
		{
			if (pair.second.idx == window.idx)
			{
				views.push_back(pair.first);
			}
		}
		return views;
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
		bgfx::Init init;
		auto& win = WindowContext::get().getWindow();
		const auto& size = win.getSize();
		init.platformData.ndt = Window::getNativeDisplayHandle();
		init.platformData.nwh = win.getNativeHandle();
		init.platformData.type = win.getNativeHandleType();
		init.debug = true;
		init.resolution.width = size.x;
		init.resolution.height = size.y;
		init.resolution.reset = _impl->getResetFlags();
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

		for (auto& win : WindowContext::get().getWindows())
		{
			if (!win.isRunning())
			{
				continue;
			}
			const auto& handle = win.getHandle();
			auto viewIds = _impl->getWindowViews(handle);
			if (viewIds.empty())
			{
				continue;
			}

			auto& firstViewId = viewIds.front();
			beforeWindowRender(handle, firstViewId);
			_impl->beforeWindowRender(handle, firstViewId);

			auto fbh = win.getImpl().getFrameBuffer();
			const auto& size = win.getSize();

			for (auto& viewId : viewIds)
			{
				if (isValid(fbh))
				{
					bgfx::setViewFrameBuffer(viewId, fbh);
				}
				// set view default viewport.
				bgfx::setViewRect(viewId, 0, 0, uint16_t(size.x), uint16_t(size.y));

				// this dummy draw call is here to make sure that view is cleared
				// if no other draw calls are submitted to view.
				bgfx::touch(viewId);

				// use debug font to print information about this example.
				bgfx::dbgTextClear();

				beforeRender(viewId);
				_impl->beforeRender(viewId);

				render(viewId);
				_impl->render(viewId);

				afterRender(viewId);
				_impl->afterRender(viewId);

				if (isValid(fbh))
				{
					bgfx::setViewFrameBuffer(viewId, { bgfx::kInvalidHandle });
				}
			}

			auto& lastViewId = viewIds.back();
			afterWindowRender(handle, lastViewId);
			_impl->afterWindowRender(handle, lastViewId);
		}

		// advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		return true;
	}

	[[nodiscard]] const AppImpl& App::getImpl() const noexcept
	{
		return *_impl;
	}

	[[nodiscard]] AppImpl& App::getImpl() noexcept
	{
		return *_impl;
	}

	void App::updateLogic(float deltaTime)
	{
	}

	const uint32_t _clearColor = 0x303030ff;

	void App::beforeWindowRender(const WindowHandle& window, bgfx::ViewId viewId)
	{
		bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH | BGFX_CLEAR_COLOR, _clearColor);
	}

	void App::beforeRender(bgfx::ViewId viewId)
	{
	}

	void App::render(bgfx::ViewId viewId)
	{
	}

	void App::afterRender(bgfx::ViewId viewId)
	{
	}

	void App::afterWindowRender(const WindowHandle& window, bgfx::ViewId viewId)
	{
	}

	void App::toggleDebugFlag(uint32_t flag) noexcept
	{
		_impl->toggleDebugFlag(flag);
	}

	void App::setDebugFlag(uint32_t flag, bool enabled) noexcept
	{
		_impl->setDebugFlag(flag, enabled);
	}

	void App::toggleResetFlag(uint32_t flag) noexcept
	{
		_impl->toggleResetFlag(flag);
	}
	
	void App::setResetFlag(uint32_t flag, bool enabled) noexcept
	{
		_impl->setResetFlag(flag, enabled);
	}

	void App::addComponent(std::unique_ptr<AppComponent>&& component) noexcept
	{
		_impl->addComponent(std::move(component));
	}

	void App::addViewComponent(bgfx::ViewId viewId, std::unique_ptr<ViewComponent>&& component) noexcept
	{
		_impl->addViewComponent(viewId, std::move(component));
	}

	void App::setWindowView(bgfx::ViewId viewId, const WindowHandle& window) noexcept
	{
		_impl->setWindowView(viewId, window);
	}

	WindowHandle App::getViewWindow(bgfx::ViewId viewId) const noexcept
	{
		return _impl->getViewWindow(viewId);
	}

	void AppComponent::init()
	{
	}

	void AppComponent::shutdown()
	{
	}

	void AppComponent::updateLogic(float deltaTime)
	{
	}

	void AppComponent::beforeRender(bgfx::ViewId viewId)
	{
	}

	void AppComponent::render(bgfx::ViewId viewId)
	{
	}

	void AppComponent::afterRender(bgfx::ViewId viewId)
	{
	}

	void ViewComponent::init(bgfx::ViewId viewId)
	{
	}

	void ViewComponent::shutdown()
	{
	}

	void ViewComponent::updateLogic(float deltaTime)
	{
	}

	void ViewComponent::beforeRender()
	{
	}

	void ViewComponent::render()
	{
	}

	void ViewComponent::afterRender()
	{
	}
}
