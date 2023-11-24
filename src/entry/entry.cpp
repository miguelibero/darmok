#include <bx/bx.h>
#include <bx/file.h>
#include <bx/sort.h>
#include <bgfx/bgfx.h>

#if BX_PLATFORM_EMSCRIPTEN
#	include <emscripten.h>
#endif // BX_PLATFORM_EMSCRIPTEN

#include "entry.hpp"
#include "dbg.h"
#include <darmok/input.hpp>
#include <darmok/utils.hpp>

namespace darmok
{
	static bx::FileReaderI* s_fileReader = NULL;
	static bx::FileWriterI* s_fileWriter = NULL;
	static std::string s_currentDir;


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

	static const std::string keyNames[] =
	{
		"None",
		"Esc",
		"Return",
		"Tab",
		"Space",
		"Backspace",
		"Up",
		"Down",
		"Left",
		"Right",
		"Insert",
		"Delete",
		"Home",
		"End",
		"PageUp",
		"PageDown",
		"Print",
		"Plus",
		"Minus",
		"LeftBracket",
		"RightBracket",
		"Semicolon",
		"Quote",
		"Comma",
		"Period",
		"Slash",
		"Backslash",
		"Tilde",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		"NumPad0",
		"NumPad1",
		"NumPad2",
		"NumPad3",
		"NumPad4",
		"NumPad5",
		"NumPad6",
		"NumPad7",
		"NumPad8",
		"NumPad9",
		"Key0",
		"Key1",
		"Key2",
		"Key3",
		"Key4",
		"Key5",
		"Key6",
		"Key7",
		"Key8",
		"Key9",
		"KeyA",
		"KeyB",
		"KeyC",
		"KeyD",
		"KeyE",
		"KeyF",
		"KeyG",
		"KeyH",
		"KeyI",
		"KeyJ",
		"KeyK",
		"KeyL",
		"KeyM",
		"KeyN",
		"KeyO",
		"KeyP",
		"KeyQ",
		"KeyR",
		"KeyS",
		"KeyT",
		"KeyU",
		"KeyV",
		"KeyW",
		"KeyX",
		"KeyY",
		"KeyZ",
		"GamepadA",
		"GamepadB",
		"GamepadX",
		"GamepadY",
		"GamepadThumbL",
		"GamepadThumbR",
		"GamepadShoulderL",
		"GamepadShoulderR",
		"GamepadUp",
		"GamepadDown",
		"GamepadLeft",
		"GamepadRight",
		"GamepadBack",
		"GamepadStart",
		"GamepadGuide",
	};
	BX_STATIC_ASSERT(to_underlying(Key::Count) == BX_COUNTOF(keyNames));

	const std::string& getKeyName(Key key)
	{
		BX_ASSERT(key < Key::Count, "Invalid key %d.", key);
		return keyNames[to_underlying(key)];
	}

	char keyToAscii(Key key, uint8_t modifiers)
	{
		const bool isAscii = (Key::Key0 <= key && key <= Key::KeyZ)
						  || (Key::Esc  <= key && key <= Key::Minus);
		if (!isAscii)
		{
			return '\0';
		}

		const bool isNumber = (Key::Key0 <= key && key <= Key::Key9);
		if (isNumber)
		{
			return '0' + char(to_underlying(key) - to_underlying(Key::Key0));
		}

		const bool isChar = (Key::KeyA <= key && key <= Key::KeyZ);
		if (isChar)
		{
			enum { ShiftMask = to_underlying(KeyModifier::LeftShift) | to_underlying(KeyModifier::RightShift) };

			const bool shift = !!( modifiers & ShiftMask );
			return (shift ? 'A' : 'a') + char(to_underlying(key) - to_underlying(Key::KeyA));
		}

		switch (key)
		{
		case Key::Esc:       return 0x1b;
		case Key::Return:    return '\n';
		case Key::Tab:       return '\t';
		case Key::Space:     return ' ';
		case Key::Backspace: return 0x08;
		case Key::Plus:      return '+';
		case Key::Minus:     return '-';
		default:             break;
		}

		return '\0';
	}

	bool setOrToggle(uint32_t& flags, const char* name, uint32_t bit, int first, int argc, char const* const* argv)
	{
		if (0 == bx::strCmp(argv[first], name))
		{
			int arg = first+1;
			if (argc > arg)
			{
				flags &= ~bit;

				bool set = false;
				bx::fromString(&set, argv[arg]);

				flags |= set ? bit : 0;
			}
			else
			{
				flags ^= bit;
			}

			return true;
		}

		return false;
	}

	static const std::vector<InputBinding> s_bindings =
	{
		{ Key::KeyQ,         to_underlying(KeyModifier::LeftCtrl),  1, NULL, "exit"                              },
		{ Key::KeyQ,         to_underlying(KeyModifier::RightCtrl), 1, NULL, "exit"                              },
		{ Key::KeyF,         to_underlying(KeyModifier::LeftCtrl),  1, NULL, "graphics fullscreen"               },
		{ Key::KeyF,         to_underlying(KeyModifier::RightCtrl), 1, NULL, "graphics fullscreen"               },
		{ Key::Return,       to_underlying(KeyModifier::RightAlt),  1, NULL, "graphics fullscreen"               },
		{ Key::F1,           to_underlying(KeyModifier::None),      1, NULL, "graphics stats"                    },
		{ Key::F1,           to_underlying(KeyModifier::LeftCtrl),  1, NULL, "graphics ifh"                      },
		{ Key::GamepadStart, to_underlying(KeyModifier::None),      1, NULL, "graphics stats"                    },
		{ Key::F1,           to_underlying(KeyModifier::LeftShift), 1, NULL, "graphics stats 0\ngraphics text 0" },
		{ Key::F3,           to_underlying(KeyModifier::None),      1, NULL, "graphics wireframe"                },
		{ Key::F6,           to_underlying(KeyModifier::None),      1, NULL, "graphics profiler"                 },
		{ Key::F7,           to_underlying(KeyModifier::None),      1, NULL, "graphics vsync"                    },
		{ Key::F8,           to_underlying(KeyModifier::None),      1, NULL, "graphics msaa"                     },
		{ Key::F9,           to_underlying(KeyModifier::None),      1, NULL, "graphics flush"                    },
		{ Key::F10,          to_underlying(KeyModifier::None),      1, NULL, "graphics hidpi"                    },
		{ Key::Print,        to_underlying(KeyModifier::None),      1, NULL, "graphics screenshot"               },
		{ Key::KeyP,         to_underlying(KeyModifier::LeftCtrl),  1, NULL, "graphics screenshot"               },
	};

#if BX_PLATFORM_EMSCRIPTEN
	static App* s_app;
	static void updateApp()
	{
		s_app->update();
	}
#endif // BX_PLATFORM_EMSCRIPTEN

	static char s_restartArgs[1024] = { '\0' };

	static ptrdiff_t s_offset = 0;

	App::~App()
	{
	}

	int runApp(std::unique_ptr<App>&& app, const std::vector<std::string>& args)
	{
		auto width = DARMOK_DEFAULT_WIDTH;
		auto height = DARMOK_DEFAULT_HEIGHT;
		setWindowSize(kDefaultWindowHandle, WindowSize(width, height));

		app->init(args);
		bgfx::frame();

#if BX_PLATFORM_EMSCRIPTEN
		s_app = _app;
		emscripten_set_main_loop(&updateApp, -1, 1);
#else
		while (app->update())
		{
			if (0 != bx::strLen(s_restartArgs) )
			{
				break;
			}
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

		setWindowSize(kDefaultWindowHandle, WindowSize(DARMOK_DEFAULT_WIDTH, DARMOK_DEFAULT_HEIGHT));

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

	static std::array<WindowState, DARMOK_CONFIG_MAX_WINDOWS> s_windows;

	void WindowState::clear()
	{
		handle = { 0 };
		size = WindowSize(0, 0);
		pos = WindowPosition(0, 0);
		nativeHandle = nullptr;
		dropFile = "";
	}

	WindowState& getWindowState(WindowHandle handle)
	{
		return s_windows[handle.idx];
	}

	bool processEvents()
	{
		while(true)
		{
			auto ev = pollEvent();
			if (ev == nullptr)
			{
				break;
			}
			auto ptr = ev.get();
			switch (ev->type)
			{
				case Event::Exit:
					return true;
				case Event::CharInput:
					static_cast<CharInputEvent*>(ptr)->process();
					break;
				case Event::GamepadAxisChanged:
					static_cast<GamepadAxisChangedEvent*>(ptr)->process();
					break;
				case Event::GamepadConnection:
					static_cast<GamepadConnectionEvent*>(ptr)->process();
					break;
				case Event::KeyPressed:
					static_cast<KeyPressedEvent*>(ptr)->process();
					break;
				case Event::MouseMoved:
					static_cast<MouseMovedEvent*>(ptr)->process();
					break;
				case Event::MouseButtonPressed:
					static_cast<MouseButtonPressedEvent*>(ptr)->process();
					break;
				case Event::WindowSizeChanged:
					static_cast<WindowSizeChangedEvent*>(ptr)->process();
					break;
				case Event::WindowCreated:
					static_cast<WindowCreatedEvent*>(ptr)->process();
					break;
				case Event::WindowSuspended:
					static_cast<WindowSuspendedEvent*>(ptr)->process();
					break;
				case Event::FileDropped:
					static_cast<FileDroppedEvent*>(ptr)->process();
					break;
				default:
					break;
			}
			inputProcess();
		};

		return 0;
	}

	bx::FileReaderI& getFileReader()
	{
		return *s_fileReader;
	}

	bx::FileWriterI& getFileWriter()
	{
		return *s_fileWriter;
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
		inputChar(_data);
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

	void WindowSizeChangedEvent::process()
	{
		WindowState& win = getWindowState(_window);
		win.handle = _window;
		win.size = _size;
		bgfx::reset(win.size.width, win.size.height, win.flags);
		inputSetMouseResolution(uint16_t(win.size.width), uint16_t(win.size.height));
	}

	WindowPositionChangedEvent::WindowPositionChangedEvent(WindowHandle window, const WindowPosition& pos)
		: Event(WindowSizeChanged)
		, _window(window)
		, _pos(pos)
	{
	}

	void WindowPositionChangedEvent::process()
	{
		WindowState& win = getWindowState(_window);
		win.handle = _window;
		win.pos = _pos;
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
		WindowState& win = getWindowState(_window);
		win.handle = _window;
		win.size = _options.size;
		win.pos = _options.pos;
		win.nativeHandle = _nativeHandle;
	}

	WindowDestroyedEvent::WindowDestroyedEvent(WindowHandle window)
		:Event(WindowSuspended)
		, _window(window)
	{
	}

	void WindowDestroyedEvent::process()
	{
		getWindowState(_window).clear();
	}

	WindowSuspendedEvent::WindowSuspendedEvent(WindowHandle window, SuspendPhase phase)
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
		WindowState& win = getWindowState(_window);
		win.dropFile = _filePath;
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

	void EventQueue::postWindowSuspendedEvent(WindowHandle window, SuspendPhase phase)
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

}
