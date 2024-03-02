
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
	AppImpl::AppImpl()
		: _exit(false)
		, _debug(BGFX_DEBUG_NONE)
		, _reset(BGFX_RESET_VSYNC)
		, _needsReset(false)
		, _init(false)
		, _lastUpdate(0)
		, _targetUpdateDeltaTime(0.)
	{
	}

	AppImpl& AppImpl::get()
	{
		static AppImpl instance;
		return instance;
	}

	const float AppImpl::defaultTargetUpdateDeltaTime = 1.0f / 30.0f;
	const int AppImpl::maxInstantLogicUpdates = 10;

	float AppImpl::updateTimePassed()
	{
		int64_t now = bx::getHPCounter();
		float timePassed = (now - _lastUpdate) / double(bx::getHPFrequency());
		_lastUpdate = bx::getHPCounter();
		return timePassed;
	}

	void AppImpl::init(App& app, const std::vector<std::string>& args, double targetUpdateDeltaTime)
	{
		_viewWindows.clear();
		_viewWindows[0] = Window::DefaultHandle;

		bx::FilePath fp(args[0].c_str());
		auto basePath = fp.getPath();

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

	void AppImpl::updateLogic(float dt)
	{
		Input::get().getImpl().update();

		for (auto& component : _appComponents)
		{
			component->updateLogic(dt);
		}
		for (auto& elm : _viewComponents)
		{
			for (auto& component : elm.second)
			{
				component->updateLogic(dt);
			}
		}
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

	const std::string AppImpl::_bindingsName = "main";

	void AppImpl::setCurrentDir(const std::string& dir)
	{
		_currentDir = dir;
	}

	void AppImpl::addBindings()
	{
		Input::get().addBindings(_bindingsName, {
			{ KeyboardBindingKey { KeyboardKey::Esc,		KeyboardModifiers::None },			true, exitAppBinding },
			{ KeyboardBindingKey { KeyboardKey::KeyQ,		KeyboardModifiers::LeftCtrl	},		true, exitAppBinding },
			{ KeyboardBindingKey { KeyboardKey::KeyQ,		KeyboardModifiers::RightCtrl },		true, exitAppBinding },
			{ KeyboardBindingKey { KeyboardKey::KeyF,		KeyboardModifiers::LeftCtrl	},		true, fullscreenToggleBinding },
			{ KeyboardBindingKey { KeyboardKey::KeyF,		KeyboardModifiers::RightCtrl },		true, fullscreenToggleBinding },
			{ KeyboardBindingKey { KeyboardKey::Return,		KeyboardModifiers::LeftAlt },		true, fullscreenToggleBinding },
			{ KeyboardBindingKey { KeyboardKey::Return,		KeyboardModifiers::RightAlt },		true, fullscreenToggleBinding },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::None },			true, toggleDebugStatsBinding },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::LeftAlt },		true, toggleDebugTextBinding },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::RightAlt },		true, toggleDebugTextBinding },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::LeftCtrl	},		true, toggleDebugIfhBinding },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::RightCtrl },		true, toggleDebugIfhBinding },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::LeftShift },		true, disableDebugFlagsBinding },
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::RightShift },	true, disableDebugFlagsBinding },
			{ KeyboardBindingKey { KeyboardKey::F3,			KeyboardModifiers::None},			true, toggleDebugWireFrameBinding },
			{ KeyboardBindingKey { KeyboardKey::F6,			KeyboardModifiers::None},			true, toggleDebugProfilerBinding },
			{ KeyboardBindingKey { KeyboardKey::F7,			KeyboardModifiers::None},			true, toggleResetVsyncBinding },
			{ KeyboardBindingKey { KeyboardKey::F8,			KeyboardModifiers::None},			true, toggleResetMsaaBinding },
			{ KeyboardBindingKey { KeyboardKey::F9,			KeyboardModifiers::None},			true, toggleResetFlushAfterRenderBinding },
			{ KeyboardBindingKey { KeyboardKey::F10,		KeyboardModifiers::None},			true, toggleResetHidpiBinding },
			{ KeyboardBindingKey { KeyboardKey::Print,		KeyboardModifiers::None},			true, screenshotBinding },
			{ KeyboardBindingKey { KeyboardKey::KeyP,		KeyboardModifiers::LeftCtrl },		true, screenshotBinding },
			{ KeyboardBindingKey { KeyboardKey::KeyP,		KeyboardModifiers::RightCtrl },		true, screenshotBinding },
		});
	}

	void AppImpl::removeBindings()
	{
		Input::get().removeBindings(_bindingsName);
	}

	void AppImpl::exitAppBinding()
	{
		AppImpl::get()._exit = true;
	};

	void AppImpl::fullscreenToggleBinding()
	{
		WindowContext::get().getWindow().toggleFullscreen();
	}

	static uint32_t setFlag(uint32_t flags, uint32_t flag, bool enabled)
	{
		if (enabled)
		{
			return flags | flag;
		}
		else
		{
			return flags & ~flag;
		}
	}

	bool AppImpl::toggleResetFlag(uint32_t flag)
	{
		auto v = !getResetFlag(flag);
		setResetFlag(flag, v);
		return v;
	}

	void AppImpl::setResetFlag(uint32_t flag, bool enabled)
	{
		auto reset = setFlag(_reset, flag, enabled);
		if (_reset != reset)
		{
			_reset = reset;
			_needsReset = true;
		}
	}

	bool AppImpl::getResetFlag(uint32_t flag)
	{
		return _reset & flag;
	}

	uint32_t AppImpl::getResetFlags()
	{
		return _reset;
	}

	bool AppImpl::toggleDebugFlag(uint32_t flag)
	{
		auto v = !getDebugFlag(flag);
		setDebugFlag(flag, v);
		return v;
	}

	void AppImpl::setDebugFlag(uint32_t flag, bool enabled)
	{
		_debug = setFlag(_debug, flag, enabled);
		bgfx::setDebug(_debug);
	}

	bool AppImpl::getDebugFlag(uint32_t flag)
	{
		return _debug & flag;
	}

	void AppImpl::toggleDebugStatsBinding()
	{
		AppImpl::get().toggleDebugFlag(BGFX_DEBUG_STATS);
	}

	void AppImpl::toggleDebugTextBinding()
	{
		AppImpl::get().toggleDebugFlag(BGFX_DEBUG_TEXT);
	}

	void AppImpl::toggleDebugIfhBinding()
	{
		AppImpl::get().toggleDebugFlag(BGFX_DEBUG_IFH);
	}

	void AppImpl::toggleDebugWireFrameBinding()
	{
		AppImpl::get().toggleDebugFlag(BGFX_DEBUG_WIREFRAME);
	}

	void AppImpl::toggleDebugProfilerBinding()
	{
		AppImpl::get().toggleDebugFlag(BGFX_DEBUG_PROFILER);
	}

	void AppImpl::disableDebugFlagsBinding()
	{
		AppImpl::get().setDebugFlag(BGFX_DEBUG_STATS, false);
		AppImpl::get().setDebugFlag(BGFX_DEBUG_TEXT, false);
	}

	void AppImpl::toggleResetVsyncBinding()
	{
		AppImpl::get().toggleResetFlag(BGFX_RESET_VSYNC);
	}

	void AppImpl::toggleResetMsaaBinding()
	{
		AppImpl::get().toggleResetFlag(BGFX_RESET_MSAA_X16);
	}

	void AppImpl::toggleResetFlushAfterRenderBinding()
	{
		AppImpl::get().toggleResetFlag(BGFX_RESET_FLUSH_AFTER_RENDER);
	}

	void AppImpl::toggleResetFlipAfterRenderBinding()
	{
		AppImpl::get().toggleResetFlag(BGFX_RESET_FLIP_AFTER_RENDER);
	}

	void AppImpl::toggleResetHidpiBinding()
	{
		AppImpl::get().toggleResetFlag(BGFX_RESET_HIDPI);
	}

	void AppImpl::resetDepthClampBinding()
	{
		AppImpl::get().toggleResetFlag(BGFX_RESET_DEPTH_CLAMP);
	}

	void AppImpl::screenshotBinding()
	{
		bgfx::FrameBufferHandle fbh = BGFX_INVALID_HANDLE;

		time_t tt;
		time(&tt);

		auto filePath = AppImpl::get()._currentDir + "temp/screenshot-" + std::to_string(tt);
		// bgfx::CallbackI::screenShot = 
		bgfx::requestScreenShot(fbh, filePath.c_str());
	}


	bool AppImpl::processEvents()
	{
		bool needsReset = false;
		while (!_exit)
		{
			auto ev = PlatformContext::get().pollEvent();
			if (ev == nullptr)
			{
				break;
			}
			auto result = PlatformEvent::process(*ev);
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
			auto& size = WindowContext::get().getWindow().getSize();
			bgfx::reset(size.x, size.y, getResetFlags());
			Input::get().getMouse().getImpl().setResolution(size);
			_needsReset = false;
		}

		return _exit;
	}

	void AppImpl::addComponent(std::unique_ptr<AppComponent>&& component)
	{
		if (_init)
		{
			component->init();
		}
		_appComponents.push_back(std::move(component));
	}

	void AppImpl::addViewComponent(bgfx::ViewId viewId, std::unique_ptr<ViewComponent>&& component)
	{
		if (_init)
		{
			component->init(viewId);
		}
		_viewComponents[viewId].push_back(std::move(component));
	}

	void AppImpl::setViewWindow(bgfx::ViewId viewId, const WindowHandle& window)
	{
		_viewWindows[viewId] = window;
	}

	WindowHandle AppImpl::getViewWindow(bgfx::ViewId viewId) const
	{
		auto itr = _viewWindows.find(viewId);
		if (itr == _viewWindows.end())
		{
			return Window::InvalidHandle;
		}
		return itr->second;
	}

	const ViewWindows& AppImpl::getViewWindows() const
	{
		return _viewWindows;
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

	void App::init(const std::vector<std::string>& args)
	{
		bgfx::Init init;
		auto& win = WindowContext::get().getWindow();
		auto& size = win.getSize();
		init.platformData.ndt = Window::getNativeDisplayHandle();
		init.platformData.nwh = win.getNativeHandle();
		init.platformData.type = win.getNativeHandleType();
		init.debug = true;
		init.resolution.width = size.x;
		init.resolution.height = size.y;
		init.resolution.reset = AppImpl::get().getResetFlags();
		init.type = bgfx::RendererType::Direct3D12;
		bgfx::init(init);

		bgfx::setPaletteColor(0, UINT32_C(0x00000000));
		bgfx::setPaletteColor(1, UINT32_C(0x303030ff));

		AppImpl::get().init(*this, args);
	}

	int App::shutdown()
	{
		AppImpl::get().shutdown();

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool App::update()
	{
		auto& impl = AppImpl::get();

		if (impl.processEvents())
		{
			return false;
		}

		impl.update([this, &impl](float dt) {
			updateLogic(dt);
			impl.updateLogic(dt);
		});

		for (auto elm : impl.getViewWindows())
		{
			auto& win = WindowContext::get().getWindow(elm.second);
			if (!win.isRunning())
			{
				continue;
			}
			auto viewId = elm.first;
			auto& handle = win.getHandle();
			auto fbh = win.getImpl().getFrameBuffer();

			if (isValid(fbh))
			{
				bgfx::setViewFrameBuffer(viewId, fbh);
			}

			// set view default viewport.
			auto& size = win.getSize();
			bgfx::setViewRect(viewId, 0, 0, uint16_t(size.x), uint16_t(size.y));

			// this dummy draw call is here to make sure that view is cleared
			// if no other draw calls are submitted to view.
			bgfx::touch(viewId);

			// use debug font to print information about this example.
			bgfx::dbgTextClear();

			beforeRender(viewId);
			impl.beforeRender(viewId);

			render(viewId);
			impl.render(viewId);

			afterRender(viewId);
			impl.afterRender(viewId);

			// advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			if (isValid(fbh))
			{
				bgfx::setViewFrameBuffer(viewId, { bgfx::kInvalidHandle });
			}
		}

		return true;
	}

	void App::updateLogic(float dt)
	{
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

	void App::toggleDebugFlag(uint32_t flag)
	{
		AppImpl::get().toggleDebugFlag(flag);
	}

	void App::setDebugFlag(uint32_t flag, bool enabled)
	{
		AppImpl::get().setDebugFlag(flag, enabled);
	}

	void App::toggleResetFlag(uint32_t flag)
	{
		AppImpl::get().toggleResetFlag(flag);
	}
	
	void App::setResetFlag(uint32_t flag, bool enabled)
	{
		AppImpl::get().setResetFlag(flag, enabled);
	}


	void App::addComponent(std::unique_ptr<AppComponent>&& component)
	{
		AppImpl::get().addComponent(std::move(component));
	}

	void App::addViewComponent(bgfx::ViewId viewId, std::unique_ptr<ViewComponent>&& component)
	{
		AppImpl::get().addViewComponent(viewId, std::move(component));
	}

	void App::setViewWindow(bgfx::ViewId viewId, const WindowHandle& window)
	{
		AppImpl::get().setViewWindow(viewId, window);
	}

	WindowHandle App::getViewWindow(bgfx::ViewId viewId) const
	{
		return AppImpl::get().getViewWindow(viewId);
	}

	void AppComponent::init()
	{
	}

	void AppComponent::shutdown()
	{
	}

	void AppComponent::updateLogic(float dt)
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

	void ViewComponent::updateLogic(float dt)
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
