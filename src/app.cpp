
#include "app.hpp"
#include "imgui.hpp"
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
		auto& win = Window::get(Window::DefaultHandle);
		win.toggleFullscreen();
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
		{ Key::Esc,			 to_underlying(KeyModifier::None),		true, exitAppBinding },
		{ Key::KeyQ,         to_underlying(KeyModifier::LeftCtrl),	true, exitAppBinding },
		{ Key::KeyQ,         to_underlying(KeyModifier::RightCtrl),	true, exitAppBinding },
		{ Key::KeyF,         to_underlying(KeyModifier::LeftCtrl),	true, fullscreenToggleBinding },
		{ Key::KeyF,         to_underlying(KeyModifier::RightCtrl),	true, fullscreenToggleBinding },
		{ Key::Return,       to_underlying(KeyModifier::RightAlt),	true, fullscreenToggleBinding },
		{ Key::F1,           to_underlying(KeyModifier::None),		true, toggleDebugStatsBinding },
		{ Key::F1,           to_underlying(KeyModifier::None),		true, toggleDebugTextBinding },
		{ Key::F1,           to_underlying(KeyModifier::LeftCtrl),	true, toggleDebugIfhBinding },
		{ Key::F1,           to_underlying(KeyModifier::LeftShift), true, disableDebugFlagsBinding },
		{ Key::F3,           to_underlying(KeyModifier::None),      true, toggleDebugWireFrameBinding },
		{ Key::F6,           to_underlying(KeyModifier::None),      true, toggleDebugProfilerBinding },
		{ Key::F7,           to_underlying(KeyModifier::None),      true, toggleResetVsyncBinding },
		{ Key::F8,           to_underlying(KeyModifier::None),      true, toggleResetMsaaBinding },
		{ Key::F9,           to_underlying(KeyModifier::None),      true, toggleResetFlushAfterRenderBinding },
		{ Key::F10,          to_underlying(KeyModifier::None),      true, toggleResetHidpiBinding },
		{ Key::Print,        to_underlying(KeyModifier::None),      true, screenshotBinding },
		{ Key::KeyP,         to_underlying(KeyModifier::LeftCtrl),  true, screenshotBinding },
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

		inputInit();
		inputAddBindings("bindings", std::vector<InputBinding>(s_bindings));

		auto result = ::_main_(argc, (char**)argv);

		setCurrentDir("");

		inputRemoveBindings("bindings");
		inputShutdown();

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
			auto ev = pollEvent();
			if (ev == nullptr)
			{
				break;
			}
			auto result = Event::process(*ev);
			if (result == Event::Result::Exit)
			{
				return true;
			}
			if (result == Event::Result::Reset)
			{
				needsReset = true;
			}
			inputProcess();
		};

		if(needsReset || s_needsReset)
		{
			auto& size = Window::get().getSize();
			bgfx::reset(size.width, size.height, getResetFlags());
			inputSetMouseResolution(size);
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

	Event::Result Event::process(Event& ev)
	{
		switch (ev._type)
		{
		case Event::Exit:
			return Result::Exit;
		case Event::CharInput:
			static_cast<CharInputEvent&>(ev).process();
			break;
		case Event::GamepadAxisChanged:
			static_cast<GamepadAxisChangedEvent&>(ev).process();
			break;
		case Event::GamepadConnection:
			static_cast<GamepadConnectionEvent&>(ev).process();
			break;
		case Event::KeyPressed:
			static_cast<KeyPressedEvent&>(ev).process();
			break;
		case Event::MouseMoved:
			static_cast<MouseMovedEvent&>(ev).process();
			break;
		case Event::MouseButtonPressed:
			static_cast<MouseButtonPressedEvent&>(ev).process();
			break;
		case Event::WindowSizeChanged:
		{
			auto reset = static_cast<WindowSizeChangedEvent&>(ev).process();
			if (reset)
			{
				return Result::Reset;
			}
			break;
		}
		case Event::WindowCreated:
			static_cast<WindowCreatedEvent&>(ev).process();
			break;
		case Event::WindowSuspended:
			static_cast<WindowSuspendedEvent&>(ev).process();
			break;
		case Event::FileDropped:
			static_cast<FileDroppedEvent&>(ev).process();
			break;
		default:
			break;
		}
		return {};
	}

	GamepadAxisChangedEvent::GamepadAxisChangedEvent(GamepadHandle gampad, GamepadAxis axis, int32_t value)
		: Event(GamepadAxisChanged)
		, _gamepad(gampad)
		, _axis(axis)
		, _value(value)
	{
	}

	void GamepadAxisChangedEvent::process()
	{
		inputSetGamepadAxis(_gamepad, _axis, _value);
	}

	CharInputEvent::CharInputEvent(const Utf8Char& data)
		: Event(CharInput)
		, _data(data)
	{
	}

	void CharInputEvent::process()
	{
		inputPushChar(_data);
	}

	GamepadConnectionEvent::GamepadConnectionEvent(GamepadHandle gamepad, bool connected)
		: Event(GamepadConnection)
		, _gamepad(gamepad)
		, _connected(connected)
	{
	}

	void GamepadConnectionEvent::process()
	{
		DBG("gamepad %d, %d", _gamepad.idx, _connected);
	}

	KeyPressedEvent::KeyPressedEvent(Key key, uint8_t modifiers, bool down)
		: Event(KeyPressed)
		, _key(key)
		, _modifiers(modifiers)
		, _down(down)
	{
	}

	void KeyPressedEvent::process()
	{
		inputSetKeyState(_key, _modifiers, _down);
	}

	MouseMovedEvent::MouseMovedEvent(const MousePosition& pos)
		: Event(MouseMoved)
		, _pos(pos)
	{
	}

	void MouseMovedEvent::process()
	{
		inputSetMousePos(_pos);
	}

	MouseButtonPressedEvent::MouseButtonPressedEvent(MouseButton button, bool down)
		: Event(MouseButtonPressed)
		, _button(button)
		, _down(down)
	{
	}

	void MouseButtonPressedEvent::process()
	{
		inputSetMouseButtonState(_button, _down);
	}

	WindowSizeChangedEvent::WindowSizeChangedEvent(WindowHandle window, const WindowSize& size)
		: Event(WindowSizeChanged)
		, _window(window)
		, _size(size)
	{
	}

	bool WindowSizeChangedEvent::process()
	{
		auto& win = Window::get(_window);
		if (win._size == _size)
		{
			return false;
		}
		win._size = _size;
		return true;
	}

	WindowHandle WindowSizeChangedEvent::getWindowHandle()
	{
		return _window;
	}

	WindowPositionChangedEvent::WindowPositionChangedEvent(WindowHandle window, const WindowPosition& pos)
		: Event(WindowSizeChanged)
		, _window(window)
		, _pos(pos)
	{
	}

	void WindowPositionChangedEvent::process()
	{
		auto& win = Window::get(_window);
		win._pos = _pos;
	}

	WindowCreatedEvent::WindowCreatedEvent(WindowHandle window, void* nativeHandle, const WindowCreationOptions& options)
		:Event(WindowCreated)
		, _window(window)
		, _nativeHandle(nativeHandle)
		, _options(options)
	{
	}

	void WindowCreatedEvent::process()
	{
		auto& win = Window::get(_window);
		win._handle = _window;
		win._size = _options.size;
		win._pos = _options.pos;
		win._title = _options.title;
		win._flags = _options.flags;
	}

	WindowDestroyedEvent::WindowDestroyedEvent(WindowHandle window)
		:Event(WindowSuspended)
		, _window(window)
	{
	}

	void WindowDestroyedEvent::process()
	{
		auto& win = Window::get();
		win._handle = Window::InvalidHandle;
		win._size = {};
		win._pos = {};
		win._title = {};
		win._flags = WindowFlags::None;
	}

	WindowSuspendedEvent::WindowSuspendedEvent(WindowHandle window, WindowSuspendPhase phase)
		:Event(WindowSuspended)
		, _window(window)
		, _phase(phase)
	{
	}

	void WindowSuspendedEvent::process()
	{
	}

	FileDroppedEvent::FileDroppedEvent(WindowHandle window, const std::string& filePath)
		:Event(FileDropped)
		, _window(window)
		, _filePath(filePath)
	{
	}

	void FileDroppedEvent::process()
	{
		DBG("drop file %s", _filePath.c_str());
		auto& win = Window::get(_window);
		win._dropFilePath = _filePath;
	}


	void EventQueue::postGamepadAxisChangedEvent(GamepadHandle gamepad, GamepadAxis axis, int32_t value)
	{
		_queue.push(std::make_unique<GamepadAxisChangedEvent>(gamepad, axis, value));
	}

	void EventQueue::postCharInputEvent(const Utf8Char& data)
	{
		_queue.push(std::make_unique<CharInputEvent>(data));
	}

	void EventQueue::postExitEvent()
	{
		auto ev = std::make_unique<Event>(Event::Exit);
		_queue.push(std::move(ev));
	}

	void EventQueue::postGamepadConnectionEvent(GamepadHandle gamepad, bool connected)
	{
		_queue.push(std::make_unique<GamepadConnectionEvent>(gamepad, connected));
	}

	void EventQueue::postKeyPressedEvent(Key key, uint8_t modifiers, bool down)
	{
		_queue.push(std::make_unique<KeyPressedEvent>(key, modifiers, down));
	}

	void EventQueue::postMouseMovedEvent(const MousePosition& pos)
	{
		_queue.push(std::make_unique<MouseMovedEvent>(pos));
	}

	void EventQueue::postMouseButtonPressedEvent(MouseButton button, bool down)
	{
		_queue.push(std::make_unique<MouseButtonPressedEvent>(button, down));
	}

	void EventQueue::postWindowSizeChangedEvent(WindowHandle window, const WindowSize& size)
	{
		_queue.push(std::make_unique<WindowSizeChangedEvent>(window, size));
	}

	void EventQueue::postWindowPositionChangedEvent(WindowHandle window, const WindowPosition& pos)
	{
		_queue.push(std::make_unique<WindowPositionChangedEvent>(window, pos));
	}

	void EventQueue::postWindowCreatedEvent(WindowHandle window, void* nativeHandle, const WindowCreationOptions& options)
	{
		_queue.push(std::make_unique<WindowCreatedEvent>(window, nativeHandle, options));
	}

	void EventQueue::postWindowDestroyedEvent(WindowHandle window)
	{
		_queue.push(std::make_unique<WindowDestroyedEvent>(window));
	}

	void EventQueue::postWindowSuspendedEvent(WindowHandle window, WindowSuspendPhase phase)
	{
		_queue.push(std::make_unique<WindowSuspendedEvent>(window, phase));
	}

	void EventQueue::postFileDroppedEvent(WindowHandle window, const std::string& filePath)
	{
		_queue.push(std::make_unique<FileDroppedEvent>(window, filePath));
	}

	std::unique_ptr<Event> EventQueue::poll()
	{
		if (_queue.size() == 0)
		{
			return nullptr;
		}
		auto ev = std::move(_queue.front());
		_queue.pop();
		return std::move(ev);
	}

	void SimpleApp::init(const std::vector<std::string>& args)
	{
		bx::FilePath fp(args[0].c_str());
		auto basePath = fp.getPath();
		setCurrentDir(std::string(basePath.getPtr(), basePath.getLength()));

		bgfx::Init init;
		auto& win = Window::get();
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
		if (darmok::processEvents())
		{
			return false;
		}

		auto& win = Window::get();
		bgfx::ViewId viewId = 0;

		_lastMousePos = inputPopMouse();
		_lastChar = inputPopChar();

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
