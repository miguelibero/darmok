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

extern "C" int32_t _main_(int32_t _argc, char** _argv);

namespace darmok
{
	static bx::FileReaderI* s_fileReader = NULL;
	static bx::FileWriterI* s_fileWriter = NULL;

	extern bx::AllocatorI* getDefaultAllocator();
	bx::AllocatorI* g_allocator = getDefaultAllocator();

	static std::string s_currentDir;

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

	static const char* keyNames[] =
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

	const std::string& getName(Key key)
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
			enum { ShiftMask = to_underlying(Modifier::LeftShift) | to_underlying(Modifier::RightShift) };

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

	static const InputBinding s_bindings[] =
	{
		{ Key::KeyQ,         to_underlying(Modifier::LeftCtrl),  1, NULL, "exit"                              },
		{ Key::KeyQ,         to_underlying(Modifier::RightCtrl), 1, NULL, "exit"                              },
		{ Key::KeyF,         to_underlying(Modifier::LeftCtrl),  1, NULL, "graphics fullscreen"               },
		{ Key::KeyF,         to_underlying(Modifier::RightCtrl), 1, NULL, "graphics fullscreen"               },
		{ Key::Return,       to_underlying(Modifier::RightAlt),  1, NULL, "graphics fullscreen"               },
		{ Key::F1,           to_underlying(Modifier::None),      1, NULL, "graphics stats"                    },
		{ Key::F1,           to_underlying(Modifier::LeftCtrl),  1, NULL, "graphics ifh"                      },
		{ Key::GamepadStart, to_underlying(Modifier::None),      1, NULL, "graphics stats"                    },
		{ Key::F1,           to_underlying(Modifier::LeftShift), 1, NULL, "graphics stats 0\ngraphics text 0" },
		{ Key::F3,           to_underlying(Modifier::None),      1, NULL, "graphics wireframe"                },
		{ Key::F6,           to_underlying(Modifier::None),      1, NULL, "graphics profiler"                 },
		{ Key::F7,           to_underlying(Modifier::None),      1, NULL, "graphics vsync"                    },
		{ Key::F8,           to_underlying(Modifier::None),      1, NULL, "graphics msaa"                     },
		{ Key::F9,           to_underlying(Modifier::None),      1, NULL, "graphics flush"                    },
		{ Key::F10,          to_underlying(Modifier::None),      1, NULL, "graphics hidpi"                    },
		{ Key::Print,        to_underlying(Modifier::None),      1, NULL, "graphics screenshot"               },
		{ Key::KeyP,         to_underlying(Modifier::LeftCtrl),  1, NULL, "graphics screenshot"               },

		INPUT_BINDING_END
	};

#if BX_PLATFORM_EMSCRIPTEN
	static AppI* s_app;
	static void updateApp()
	{
		s_app->update();
	}
#endif // BX_PLATFORM_EMSCRIPTEN

	static IApp*    s_currentApp = NULL;

	static char s_restartArgs[1024] = { '\0' };

	static ptrdiff_t s_offset = 0;

	IApp::~IApp()
	{
	}

	int runApp(std::unique_ptr<IApp>&& app, int argc, const char* const* argv)
	{
		auto width = ENTRY_DEFAULT_WIDTH;
		auto height = ENTRY_DEFAULT_HEIGHT;
		setWindowSize(kDefaultWindowHandle, width, height);

		app->init(argc, argv, width, height);
		bgfx::frame();

#if BX_PLATFORM_EMSCRIPTEN
		s_app = _app;
		emscripten_set_main_loop(&updateApp, -1, 1);
#else
		while (app->update() )
		{
			if (0 != bx::strLen(s_restartArgs) )
			{
				break;
			}
		}
#endif // BX_PLATFORM_EMSCRIPTEN

		return app->shutdown();
	}

	int main(int _argc, const char* const* _argv)
	{
		//DBG(BX_COMPILER_NAME " / " BX_CPU_NAME " / " BX_ARCH_NAME " / " BX_PLATFORM_NAME);

		s_fileReader = BX_NEW(g_allocator, FileReader);
		s_fileWriter = BX_NEW(g_allocator, FileWriter);

		inputInit();
		inputAddBindings("bindings", s_bindings);

		bx::FilePath fp(_argv[0]);
		char title[bx::kMaxFilePath];
		bx::strCopy(title, BX_COUNTOF(title), fp.getBaseName() );

		setWindowTitle(kDefaultWindowHandle, title);
		setWindowSize(kDefaultWindowHandle, ENTRY_DEFAULT_WIDTH, ENTRY_DEFAULT_HEIGHT);

		// TODO: run app

		setCurrentDir("");

		inputRemoveBindings("bindings");
		inputShutdown();

		bx::deleteObject(g_allocator, s_fileReader);
		s_fileReader = NULL;

		bx::deleteObject(g_allocator, s_fileWriter);
		s_fileWriter = NULL;

		return 0;
	}

	WindowState s_window[ENTRY_CONFIG_MAX_WINDOWS];

	bool processEvents(uint32_t& width, uint32_t& height, uint32_t& debug, uint32_t& reset, MouseState* mouse)
	{
		while(true)
		{
			auto ev = pollEvent();
			if (ev == nullptr)
			{
				break;
			}
			switch (ev->type)
			{
				case Event::Exit:
					return true;
				default:
					ev->process();
					break;
			}
			inputProcess();
		};

		bgfx::reset(width, height, reset);
		inputSetMouseResolution(uint16_t(width), uint16_t(height) );

		return 0;
	}

	bool processWindowEvents(WindowState& state, uint32_t& debug, uint32_t& reset)
	{
		WindowHandle handle = { UINT16_MAX };

		bool needsReset = false;

		while (true)
		{
			auto ev = pollEvent();
			if (ev == nullptr)
			{
				break;
			}
			if (isValid(ev->handle))
			{
				handle = ev->handle;
			}
			switch (ev->type)
			{
				case Event::Exit:
					return true;
				case Event::Size:
					needsReset = true;
					ev->process();
					break;
				default:
					ev->process();
					break;
			}
			inputProcess();
		};


		if (needsReset)
		{
			WindowState& win = s_window[0];
			bgfx::reset(win.width, win.height, reset);
			inputSetMouseResolution(uint16_t(win.width), uint16_t(win.height));
		}

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

	void AxisEvent::process()
	{
		inputSetGamepadAxis(gamepad, axis, value);
	}

	void CharEvent::process()
	{
		inputChar(len, std::move(data));
	}

	void GamepadEvent::process()
	{
		DBG("gamepad %d, %d", gamepad.idx, connected);
	}

	void KeyEvent::process()
	{
		inputSetKeyState(key, modifiers, down);
	}

	void MouseEvent::process()
	{
		bool mouseLock = inputIsMouseLocked();

		if (move)
		{
			inputSetMousePos(x, y, z);
		}
		else
		{
			inputSetMouseButtonState(button, down);
		}

		if (!mouseLock)
		{
			WindowState& win = s_window[handle.idx];
			if (move)
			{
				win.mouse.x = x;
				win.mouse.y = y;
				win.mouse.z = z;
			}
			else
			{
				win.mouse.buttons[to_underlying(button)] = down;
			}
		}
	}

	void SizeEvent::process()
	{
		if (isValid(handle))
		{
			WindowState& win = s_window[handle.idx];
			win.handle = handle;
			win.width = width;
			win.height = height;
		}
	}

	void DropFileEvent::process()
	{
		DBG("drop file %s", filePath.getCPtr());
		if (isValid(handle))
		{
			WindowState& win = s_window[handle.idx];
			win.dropFile = filePath;
		}
	}

	void Event::process()
	{

	}
}
