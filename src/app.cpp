
#include "app.hpp"
#include "platform.hpp"
#include "imgui.hpp"
#include "input.hpp"

#include <darmok/app.hpp>
#include <bx/filepath.h>

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
	{
	}

	AppImpl& AppImpl::get()
	{
		static AppImpl instance;
		return instance;
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
			{ KeyboardBindingKey { KeyboardKey::F1,			KeyboardModifiers::None },			true, toggleDebugTextBinding },
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
		Context::get().getWindow().toggleFullscreen();
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

	void AppImpl::toggleResetFlag(uint32_t flag)
	{
		setResetFlag(flag, !getResetFlag(flag));
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

	void AppImpl::toggleDebugFlag(uint32_t flag)
	{
		setDebugFlag(flag, !getDebugFlag(flag));
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

	bool AppImpl::processEvents()
	{
		bool needsReset = false;
		while(!_exit)
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

		if(needsReset || _needsReset)
		{
			auto& size = Context::get().getWindow().getSize();
			bgfx::reset(size.width, size.height, getResetFlags());
			Input::get().getMouse().getImpl().setResolution(size);
			_needsReset = false;
		}

		return _exit;
	}

	App::App()
	{
	}

	void App::init(const std::vector<std::string>& args)
	{
		bx::FilePath fp(args[0].c_str());
		auto basePath = fp.getPath();

		AppImpl::get().setCurrentDir(std::string(basePath.getPtr(), basePath.getLength()));
		AppImpl::get().addBindings();

		bgfx::Init init;
		auto& win = Context::get().getWindow();
		init.platformData.ndt = Window::getNativeDisplayHandle();
		init.platformData.nwh = win.getNativeHandle();
		init.platformData.type = win.getNativeHandleType();
		init.resolution.reset = AppImpl::get().getResetFlags();
		bgfx::init(init);

		ImguiContext::get().init();
	}

	int App::shutdown()
	{
		AppImpl::get().removeBindings();

		ImguiContext::get().shutdown();

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool App::update()
	{
		if (AppImpl::get().processEvents())
		{
			return false;
		}

		auto& win = Context::get().getWindow();
		bgfx::ViewId viewId = 0;

		auto input = Input::get().getImpl().popState();
		auto& imgui = ImguiContext::get();

		imgui.beginFrame(win.getHandle(), viewId, input);

		imguiDraw();

		imgui.endFrame();

		// Set view 0 default viewport.
		auto& size = win.getSize();
		bgfx::setViewRect(viewId, 0, 0, uint16_t(size.width), uint16_t(size.height));

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(viewId);

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();

		update(input);

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();
		return true;
	}

	void App::imguiDraw()
	{
	}

	void App::update(const InputState& input)
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

}
