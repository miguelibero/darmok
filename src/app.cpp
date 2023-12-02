
#include <darmok/app.hpp>
#include "platform.hpp"
#include "imgui.hpp"
#include "input.hpp"
#include "dbg.h"

#include <bx/bx.h>
#include <bx/file.h>
#include <bx/readerwriter.h>
#include <bgfx/bgfx.h>

#if BX_PLATFORM_EMSCRIPTEN
#	include <emscripten.h>
#endif // BX_PLATFORM_EMSCRIPTEN



namespace darmok
{
	static bx::FileReaderI* s_fileReader = NULL;
	static bx::FileWriterI* s_fileWriter = NULL;
	static std::string s_currentDir;
	static bool s_exit = false;
	static uint32_t s_debug = BGFX_DEBUG_NONE;
	static uint32_t s_reset = BGFX_RESET_VSYNC;
	static bool s_needsReset = false;

	extern bx::AllocatorI* getDefaultAllocator();
	bx::AllocatorI* g_allocator = getDefaultAllocator();

	class FileReader final : public bx::FileReader
	{
		typedef bx::FileReader super;

	public:
		virtual bool open(const bx::FilePath& filePath, bx::Error* err) override
		{
			auto absFilePath = s_currentDir + filePath.getCPtr();
			return super::open(absFilePath.c_str(), err);
		}
	};

	class FileWriter final : public bx::FileWriter
	{
		typedef bx::FileWriter super;

	public:
		virtual bool open(const bx::FilePath& filePath, bool append, bx::Error* err) override
		{
			auto absFilePath = s_currentDir + filePath.getCPtr();
			return super::open(absFilePath.c_str(), append, err);
		}
	};

	void setCurrentDir(const std::string& dir)
	{
		s_currentDir = dir;
	}

#if ENTRY_CONFIG_IMPLEMENT_DEFAULT_ALLOCATOR
	bx::AllocatorI* getDefaultAllocator()
	{
BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4459); // warning C4459: declaration of 's_allocator' hides global declaration
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wshadow");
		static bx::DefaultAllocator s_allocator;
		return &s_allocator;
BX_PRAGMA_DIAGNOSTIC_POP();
	}
#endif // ENTRY_CONFIG_IMPLEMENT_DEFAULT_ALLOCATOR

	void exitAppBinding()
	{
		s_exit = true;
	};

	void fullscreenToggleBinding()
	{
		Context::get().getWindow().toggleFullscreen();
	}

	uint32_t setFlag(uint32_t flags, uint32_t flag, bool enabled)
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

	void toggleResetFlag(uint32_t flag)
	{
		setResetFlag(flag, !getResetFlag(flag));
	}

	void setResetFlag(uint32_t flag, bool enabled)
	{
		auto reset = setFlag(s_reset, flag, enabled);
		if (s_reset != reset)
		{
			s_reset = reset;
			s_needsReset = true;
		}
	}

	bool getResetFlag(uint32_t flag)
	{
		return s_reset & flag;
	}

	uint32_t getResetFlags()
	{
		return s_reset;
	}

	void toggleDebugFlag(uint32_t flag)
	{
		setDebugFlag(flag, !getDebugFlag(flag));
	}

	void setDebugFlag(uint32_t flag, bool enabled)
	{
		s_debug = setFlag(s_debug, flag, enabled);
		bgfx::setDebug(s_debug);
	}

	bool getDebugFlag(uint32_t flag)
	{
		return s_debug & flag;
	}

	void toggleDebugStatsBinding()
	{
		toggleDebugFlag(BGFX_DEBUG_STATS);
	}

	void toggleDebugTextBinding()
	{
		toggleDebugFlag(BGFX_DEBUG_TEXT);
	}

	void toggleDebugIfhBinding()
	{
		toggleDebugFlag(BGFX_DEBUG_IFH);
	}

	void toggleDebugWireFrameBinding()
	{
		toggleDebugFlag(BGFX_DEBUG_WIREFRAME);
	}

	void toggleDebugProfilerBinding()
	{
		toggleDebugFlag(BGFX_DEBUG_PROFILER);
	}

	void disableDebugFlagsBinding()
	{
		setDebugFlag(BGFX_DEBUG_STATS, false);
		setDebugFlag(BGFX_DEBUG_TEXT, false);
	}

	void toggleResetVsyncBinding()
	{
		toggleResetFlag(BGFX_RESET_VSYNC);
	}

	void toggleResetMsaaBinding()
	{
		toggleResetFlag(BGFX_RESET_MSAA_X16);
	}

	void toggleResetFlushAfterRenderBinding()
	{
		toggleResetFlag(BGFX_RESET_FLUSH_AFTER_RENDER);
	}

	void toggleResetFlipAfterRenderBinding()
	{
		toggleResetFlag(BGFX_RESET_FLIP_AFTER_RENDER);
	}

	void toggleResetHidpiBinding()
	{
		toggleResetFlag(BGFX_RESET_HIDPI);
	}

	void resetDepthClampBinding()
	{
		toggleResetFlag(BGFX_RESET_DEPTH_CLAMP);
	}

	void screenshotBinding()
	{
		bgfx::FrameBufferHandle fbh = BGFX_INVALID_HANDLE;

		time_t tt;
		time(&tt);

		auto filePath = s_currentDir + "temp/screenshot-" + std::to_string(tt);
		// bgfx::CallbackI::screenShot = 
		bgfx::requestScreenShot(fbh, filePath.c_str());
	}

	static const std::vector<InputBinding> s_bindings =
	{
		{ KeyboardInputBinding { KeyboardKey::Esc,		KeyboardModifiers::None	},			true, exitAppBinding },
		{ KeyboardInputBinding { KeyboardKey::KeyQ,		KeyboardModifiers::LeftCtrl	},		true, exitAppBinding },
		{ KeyboardInputBinding { KeyboardKey::KeyQ,		KeyboardModifiers::RightCtrl },		true, exitAppBinding },
		{ KeyboardInputBinding { KeyboardKey::KeyF,     KeyboardModifiers::LeftCtrl	},		true, fullscreenToggleBinding },
		{ KeyboardInputBinding { KeyboardKey::KeyF,     KeyboardModifiers::RightCtrl },		true, fullscreenToggleBinding },
		{ KeyboardInputBinding { KeyboardKey::Return,   KeyboardModifiers::LeftAlt },		true, fullscreenToggleBinding },
		{ KeyboardInputBinding { KeyboardKey::Return,   KeyboardModifiers::RightAlt },		true, fullscreenToggleBinding },
		{ KeyboardInputBinding { KeyboardKey::F1,		KeyboardModifiers::None },			true, toggleDebugStatsBinding },
		{ KeyboardInputBinding { KeyboardKey::F1,		KeyboardModifiers::None },			true, toggleDebugTextBinding },
		{ KeyboardInputBinding { KeyboardKey::F1,		KeyboardModifiers::LeftCtrl	},		true, toggleDebugIfhBinding },
		{ KeyboardInputBinding { KeyboardKey::F1,		KeyboardModifiers::RightCtrl },		true, toggleDebugIfhBinding },
		{ KeyboardInputBinding { KeyboardKey::F1,		KeyboardModifiers::LeftShift },		true, disableDebugFlagsBinding },
		{ KeyboardInputBinding { KeyboardKey::F1,		KeyboardModifiers::RightShift },	true, disableDebugFlagsBinding },
		{ KeyboardInputBinding { KeyboardKey::F3,		KeyboardModifiers::None},			true, toggleDebugWireFrameBinding },
		{ KeyboardInputBinding { KeyboardKey::F6,		KeyboardModifiers::None},			true, toggleDebugProfilerBinding },
		{ KeyboardInputBinding { KeyboardKey::F7,		KeyboardModifiers::None},			true, toggleResetVsyncBinding },
		{ KeyboardInputBinding { KeyboardKey::F8,		KeyboardModifiers::None},			true, toggleResetMsaaBinding },
		{ KeyboardInputBinding { KeyboardKey::F9,		KeyboardModifiers::None},			true, toggleResetFlushAfterRenderBinding },
		{ KeyboardInputBinding { KeyboardKey::F10,		KeyboardModifiers::None},			true, toggleResetHidpiBinding },
		{ KeyboardInputBinding { KeyboardKey::Print,	KeyboardModifiers::None},			true, screenshotBinding },
		{ KeyboardInputBinding { KeyboardKey::KeyP,		KeyboardModifiers::LeftCtrl },		true, screenshotBinding },
		{ KeyboardInputBinding { KeyboardKey::KeyP,		KeyboardModifiers::RightCtrl },		true, screenshotBinding },
	};

#if BX_PLATFORM_EMSCRIPTEN
	static App* s_app;
	static void updateApp()
	{
		s_app->update();
	}
#endif // BX_PLATFORM_EMSCRIPTEN

	App::~App()
	{
	}

	int runApp(std::unique_ptr<App>&& app, const std::vector<std::string>& args)
	{
		app->init(args);

		bgfx::frame();

#if BX_PLATFORM_EMSCRIPTEN
		s_app = _app;
		emscripten_set_main_loop(&updateApp, -1, 1);
#else
		while (app->update())
		{
		}
#endif // BX_PLATFORM_EMSCRIPTEN

		return app->shutdown();
	}

	int main(int argc, const char* const* argv)
	{
		//DBG(BX_COMPILER_NAME " / " BX_CPU_NAME " / " BX_ARCH_NAME " / " BX_PLATFORM_NAME);

		s_fileReader = BX_NEW(g_allocator, FileReader);
		s_fileWriter = BX_NEW(g_allocator, FileWriter);

		Input::get().addBindings("main", std::vector<InputBinding>(s_bindings));

		auto result = ::_main_(argc, (char**)argv);

		setCurrentDir("");

		Input::get().removeBindings("main");

		bx::deleteObject(g_allocator, s_fileReader);
		s_fileReader = NULL;

		bx::deleteObject(g_allocator, s_fileWriter);
		s_fileWriter = NULL;

		return result;
	}

	bool processEvents()
	{
		bool needsReset = false;
		while(!s_exit)
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

		if(needsReset || s_needsReset)
		{
			auto& size = Context::get().getWindow().getSize();
			bgfx::reset(size.width, size.height, getResetFlags());
			Input::get().getMouse().getImpl().setResolution(size);
			s_needsReset = false;
		}

		return s_exit;
	}

	bx::FileReaderI& getFileReader()
	{
		return *s_fileReader;
	}

	bx::FileWriterI& getFileWriter()
	{
		return *s_fileWriter;
	}

	bx::AllocatorI& getAllocator()
	{
		if (NULL == g_allocator)
		{
			g_allocator = getDefaultAllocator();
		}

		return *g_allocator;
	}

	void SimpleApp::init(const std::vector<std::string>& args)
	{
		if (processEvents())
		{
			return;
		}

		bx::FilePath fp(args[0].c_str());
		auto basePath = fp.getPath();
		setCurrentDir(std::string(basePath.getPtr(), basePath.getLength()));

		bgfx::Init init;
		auto& win = Context::get().getWindow();
		init.platformData.ndt = Window::getNativeDisplayHandle();
		init.platformData.nwh = win.getNativeHandle();
		init.platformData.type = win.getNativeHandleType();
		init.resolution.reset = getResetFlags();
		bgfx::init(init);

		imguiCreate();
	}

	int SimpleApp::shutdown()
	{
		imguiDestroy();

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool SimpleApp::update()
	{
		if (processEvents())
		{
			return false;
		}

		auto& win = Context::get().getWindow();
		bgfx::ViewId viewId = 0;

		_lastMousePos = Input::get().getMouse().popRelativePosition();
		_lastChar = Input::get().getKeyboard().popChar();

		imguiBeginFrame(win.getHandle(), _lastChar, viewId);

		imguiDraw();

		imguiEndFrame();

		// Set view 0 default viewport.
		auto& size = win.getSize();
		bgfx::setViewRect(viewId, 0, 0, uint16_t(size.width), uint16_t(size.height));

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(viewId);

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();

		draw();
		
		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();
		return true;
	}

	void SimpleApp::imguiDraw()
	{
	}

	void SimpleApp::draw()
	{
	}

}
