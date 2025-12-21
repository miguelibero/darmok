#include "detail/platform_glfw.hpp"

#include "detail/input.hpp"
#include "detail/window.hpp"
#include "detail/app.hpp"
#include <darmok/utils.hpp>
#include <darmok/window.hpp>
#include <darmok/app.hpp>
#include <darmok/stream.hpp>
#include <stdexcept>

#include <bx/mutex.h>

#include <GLFW/glfw3native.h>

#include <tinyfiledialogs.h>

namespace darmok
{
#pragma region PlatformCmds

	PlatformCmd::PlatformCmd(Type type) noexcept
		: _type{ type }
	{
	}

	DestroyWindowCmd::DestroyWindowCmd() noexcept
		: PlatformCmd(DestroyWindow)
	{
	}

	void DestroyWindowCmd::process(GLFWwindow* glfw) noexcept
	{
		glfwSetWindowShouldClose(glfw, true);
	}

	RequestVideoModeInfoCmd::RequestVideoModeInfoCmd() noexcept
		: PlatformCmd(RequestVideoModeInfo)
	{
	}

	void RequestVideoModeInfoCmd::process(PlatformEventQueue& events, const WindowFrameSize& frame) noexcept
	{
		auto info = PlatformImpl::getVideoModeInfo(frame);
		events.post<VideoModeInfoEvent>(info);
	}

	ChangeWindowVideoModeCmd::ChangeWindowVideoModeCmd(VideoMode mode) noexcept
		: PlatformCmd(ChangeWindowVideoMode)
		, _mode{ std::move(mode) }
	{
	}

	expected<GLFWmonitor*, std::string> ChangeWindowVideoModeCmd::getMonitor() noexcept
	{
		if (_mode.monitor <= 0)
		{
			return glfwGetPrimaryMonitor();
		}
		int monCount = 0;
		auto monitors = glfwGetMonitors(&monCount);
		if (_mode.monitor >= monCount)
		{
			return unexpected<std::string>("invalid monitor");
		}
		return monitors[_mode.monitor];
	}

	void ChangeWindowVideoModeCmd::process(PlatformEventQueue& events, GLFWwindow* win, const WindowFrameSize& frame) noexcept
	{
		auto expMonitor = getMonitor();
		if (!expMonitor)
		{
			events.post<WindowErrorEvent>(expMonitor.error());
			return;
		}
		auto monitor = expMonitor.value();
		auto refreshRate = _mode.refreshRate == 0 ? GLFW_DONT_CARE : _mode.refreshRate;

		// is there a better way of doing this?
		// need to reset to normal window to get the default video mode
		PlatformImpl::resetWindowMonitor(win, frame.topLeft);
		auto defMode = glfwGetVideoMode(monitor);

		if (_mode.size.x == 0 || _mode.size.y == 0)
		{
			_mode.size.x = defMode->width;
			_mode.size.y = defMode->height;
		}

		switch (_mode.screenMode)
		{
			case WindowScreenMode::Normal:
			{
				auto workArea = PlatformImpl::getMonitorWorkarea(monitor);
				auto winSize = frame.topLeft + _mode.size + frame.botRight;
				if (workArea.size.x < winSize.x || workArea.size.y < winSize.y)
				{
					events.post<WindowErrorEvent>("window would not fit in workarea");
					_mode.size = workArea.size - frame.topLeft - frame.botRight;
				}
				glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
				glfwSetWindowAttrib(win, GLFW_FLOATING, GLFW_FALSE);
				glfwSetWindowMonitor(win, nullptr
					, workArea.origin.x + frame.topLeft.x
					, workArea.origin.y + frame.topLeft.y
					, _mode.size.x
					, _mode.size.y
					, refreshRate
				);
				break;
			}
			case WindowScreenMode::Fullscreen:
			{
				glfwSetWindowAttrib(win, GLFW_FLOATING, GLFW_FALSE);
				glfwSetWindowMonitor(win, monitor, 0, 0
					, _mode.size.x
					, _mode.size.y
					, refreshRate
				);
				break;
			}
			case WindowScreenMode::WindowedFullscreen:
			{
				// https://www.glfw.org/docs/latest/window_guide.html#window_windowed_full_screen
				if (_mode.size.x != defMode->width || _mode.size.y != defMode->height)
				{
					events.post<WindowErrorEvent>("windowed fullscreen needs to have the default video mode size");
				}
				if (refreshRate != defMode->refreshRate)
				{
					events.post<WindowErrorEvent>("windowed fullscreen needs to have the default video mode refresh rate");
				}

				glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
				glfwSetWindowAttrib(win, GLFW_FLOATING, GLFW_TRUE);
				glfwSetWindowMonitor(win, monitor, 0, 0
					, defMode->width
					, defMode->height
					, defMode->refreshRate
				);
				break;
			}
			default:
				events.post<WindowErrorEvent>("unsupported screen mode");
				return;
		}

		// glfwSetWindowAspectRatio(win, _mode.size.x, _mode.size.y);
		glfwWindowHint(GLFW_RED_BITS, _mode.depth.r);
		glfwWindowHint(GLFW_GREEN_BITS, _mode.depth.g);
		glfwWindowHint(GLFW_BLUE_BITS, _mode.depth.b);

		if (!_mode.complete())
		{
			auto fmode = PlatformImpl::getVideoMode(win, monitor);
			_mode.monitor = fmode.monitor;
			_mode.refreshRate = fmode.refreshRate;
			_mode.depth = fmode.depth;
		}

		events.post<WindowVideoModeEvent>(std::move(_mode));
	}

	ChangeWindowCursorModeCmd::ChangeWindowCursorModeCmd(WindowCursorMode value) noexcept
		: PlatformCmd(ChangeWindowCursorMode)
		, _value{ value }
	{
	}

	void ChangeWindowCursorModeCmd::process(PlatformEventQueue& events, GLFWwindow* glfw) noexcept
	{
		int v = 0;
		switch (_value)
		{
		case WindowCursorMode::Normal:
			v = GLFW_CURSOR_NORMAL;
			break;
		case WindowCursorMode::Disabled:
			v = GLFW_CURSOR_DISABLED;
			break;
		case WindowCursorMode::Hidden:
			v = GLFW_CURSOR_HIDDEN;
			break;
		default:
			return;
		}
		auto old = glfwGetInputMode(glfw, GLFW_CURSOR);
		if (old == v)
		{
			return;
		}
		glfwSetInputMode(glfw, GLFW_CURSOR, v);
		events.post<WindowCursorModeEvent>(_value);
	}

	ChangeWindowTitleCmd::ChangeWindowTitleCmd(std::string title) noexcept
		: PlatformCmd(ChangeWindowTitle)
		, _title{ std::move(title) }
	{
	}

	void ChangeWindowTitleCmd::process(PlatformEventQueue& events, GLFWwindow* glfw) noexcept
	{
		glfwSetWindowTitle(glfw, _title.c_str());
		events.post<WindowTitleEvent>(_title);
	}

	OpenFileDialogCmd::OpenFileDialogCmd(FileDialogOptions options, FileDialogCallback callback) noexcept
		: PlatformCmd(OpenFileDialog)
		, _options{ std::move(options) }
		, _callback{ std::move(callback) }
	{
	}

	OpenFileDialogCmd::~OpenFileDialogCmd() = default;

	void OpenFileDialogCmd::process(PlatformEventQueue& events, GLFWwindow* glfw) noexcept
	{
		std::vector<const char*> cfilters;
		cfilters.reserve(_options.filters.size());
		for (auto& f : _options.filters)
		{
			cfilters.push_back(f.c_str());
		}

		const char* tfdResult = nullptr;
		auto defaultPath = std::filesystem::absolute(_options.defaultPath).string();

		switch (_options.type)
		{
			case FileDialogType::Open:
			{
				tfdResult = tinyfd_openFileDialog(
					_options.title.c_str(), defaultPath.c_str(),
					static_cast<int>(cfilters.size()),
                    cfilters.data(), _options.filterDesc.c_str(),
					_options.allowMultiple ? 1 : 0
				);

				break;
			}
			case FileDialogType::Save:
			{
				tfdResult = tinyfd_saveFileDialog(
					_options.title.c_str(), defaultPath.c_str(),
                    static_cast<int>(cfilters.size()),
                    cfilters.data(), _options.filterDesc.c_str()
				);
				break;
			}
		}

		FileDialogResult result;
		if (tfdResult)
		{
			for (auto& part : StringUtils::split('|', tfdResult))
			{
				result.push_back(part);
			}
		}

		events.post<FileDialogEvent>(std::move(result), std::move(_callback));
	}

	bool PlatformCmd::process(PlatformCmd& cmd, PlatformImpl& plat) noexcept
	{
		// explicit cast to avoid virtual method for performance
		switch (cmd._type)
		{
		case PlatformCmd::RequestVideoModeInfo:
			static_cast<RequestVideoModeInfoCmd&>(cmd).process(plat.getEvents(), plat.getWindowFrameSize());
			break;
		case PlatformCmd::DestroyWindow:
			static_cast<DestroyWindowCmd&>(cmd).process(plat.getGlfwWindow());
			break;
		case PlatformCmd::ChangeWindowCursorMode:
			static_cast<ChangeWindowCursorModeCmd&>(cmd).process(plat.getEvents(), plat.getGlfwWindow());
			break;
		case PlatformCmd::ChangeWindowVideoMode:
			static_cast<ChangeWindowVideoModeCmd&>(cmd).process(plat.getEvents(), plat.getGlfwWindow(), plat.getWindowFrameSize());
			break;
		case PlatformCmd::ChangeWindowTitle:
			static_cast<ChangeWindowTitleCmd&>(cmd).process(plat.getEvents(), plat.getGlfwWindow());
			break;
		case PlatformCmd::OpenFileDialog:
			static_cast<OpenFileDialogCmd&>(cmd).process(plat.getEvents(), plat.getGlfwWindow());
		}

		return true;
	}

#pragma endregion PlatformCmds	

	PlatformImpl::PlatformImpl(Platform& plat) noexcept
		: _plat{ plat }
		, _window{ nullptr }
		, _winFrameSize{ glm::uvec2(0), glm::uvec2(0) }
	{
	}

	GLFWwindow* PlatformImpl::getGlfwWindow() const noexcept
	{
		return _window;
	}

	const WindowFrameSize& PlatformImpl::getWindowFrameSize() const noexcept
	{
		return _winFrameSize;
	}

	PlatformEventQueue& PlatformImpl::getEvents() noexcept
	{
		return _events;
	}

	void* PlatformImpl::getWindowHandle(GLFWwindow* window) noexcept
	{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
		if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
		{
			return glfwGetWaylandWindow(window);
		}
		return (void*)(uintptr_t)glfwGetX11Window(window);
#	elif BX_PLATFORM_OSX
		return glfwGetCocoaWindow(window);
#	elif BX_PLATFORM_WINDOWS
		return glfwGetWin32Window(window);
#	endif // BX_PLATFORM_
	}

	void PlatformImpl::destroyWindow(GLFWwindow* window) noexcept
	{
		if (window == nullptr)
		{
			return;
		}
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if DARMOK_CONFIG_USE_WAYLAND
		wl_egl_window* win_impl = (wl_egl_window*)glfwGetWindowUserPointer(window);
		if (win_impl)
		{
			glfwSetWindowUserPointer(window, nullptr);
			wl_egl_window_destroy(win_impl);
		}
#endif
#endif
		glfwDestroyWindow(window);
	}


	GLFWwindow* PlatformImpl::createWindow(const glm::uvec2& size, const char* title) noexcept
	{
		// glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glm::uvec2 fsize(size);
		GLFWmonitor* monitor = nullptr;
		if (fsize.x == 0 || fsize.y == 0)
		{
			monitor = glfwGetPrimaryMonitor();
			auto mode = glfwGetVideoMode(monitor);
			fsize.x = mode->width;
			fsize.y = mode->height;
		}

		auto window = glfwCreateWindow(
			fsize.x
			, fsize.y
			, title
			, monitor
			, nullptr);
		if (!window)
		{
			return window;
		}

		// glfwSetWindowAspectRatio(window, size.x, size.y);
		
		glfwSetKeyCallback(window, staticKeyCallback);
		glfwSetCharCallback(window, staticCharCallback);
		glfwSetScrollCallback(window, staticScrollCallback);
		glfwSetCursorPosCallback(window, staticCursorPosCallback);
		glfwSetCursorEnterCallback(window, staticCursorEnterCallback);
		glfwSetMouseButtonCallback(window, staticMouseButtonCallback);
		glfwSetJoystickCallback(staticJoystickCallback);
		glfwSetWindowSizeCallback(window, staticWindowSizeCallback);
		glfwSetFramebufferSizeCallback(window, staticFramebufferSizeCallback);

		return window;
	}

	KeyboardModifiers PlatformImpl::translateKeyModifiers(int glfw) noexcept
	{
		KeyboardModifiers modifiers;

		if (static_cast<bool>(glfw & GLFW_MOD_ALT))
		{
			modifiers.insert(Keyboard::Definition::ModifierAlt);
		}

		if (static_cast<bool>(glfw & GLFW_MOD_CONTROL))
		{
			modifiers.insert(Keyboard::Definition::ModifierCtrl);
		}

		if (static_cast<bool>(glfw & GLFW_MOD_SUPER))
		{
			modifiers.insert(Keyboard::Definition::ModifierMeta);
		}

		if (static_cast<bool>(glfw & GLFW_MOD_SHIFT))
		{
			modifiers.insert(Keyboard::Definition::ModifierShift);
		}

		return modifiers;
	}

	PlatformImpl::KeyMap PlatformImpl::createKeyMap() noexcept
	{
		KeyMap v{};
		v[GLFW_KEY_ESCAPE] = Keyboard::Definition::KeyEsc;
		v[GLFW_KEY_ENTER] = Keyboard::Definition::KeyReturn;
		v[GLFW_KEY_TAB] = Keyboard::Definition::KeyTab;
		v[GLFW_KEY_BACKSPACE] = Keyboard::Definition::KeyBackspace;
		v[GLFW_KEY_SPACE] = Keyboard::Definition::KeySpace;
		v[GLFW_KEY_UP] = Keyboard::Definition::KeyUp;
		v[GLFW_KEY_DOWN] = Keyboard::Definition::KeyDown;
		v[GLFW_KEY_LEFT] = Keyboard::Definition::KeyLeft;
		v[GLFW_KEY_RIGHT] = Keyboard::Definition::KeyRight;
		v[GLFW_KEY_INSERT] = Keyboard::Definition::KeyInsert;
		v[GLFW_KEY_DELETE] = Keyboard::Definition::KeyDelete;
		v[GLFW_KEY_PAGE_UP] = Keyboard::Definition::KeyPageUp;
		v[GLFW_KEY_PAGE_DOWN] = Keyboard::Definition::KeyPageDown;
		v[GLFW_KEY_HOME] = Keyboard::Definition::KeyHome;
		v[GLFW_KEY_END] = Keyboard::Definition::KeyEnd;
		v[GLFW_KEY_PRINT_SCREEN] = Keyboard::Definition::KeyPrint;
		v[GLFW_KEY_PAUSE] = Keyboard::Definition::KeyPause;
		v[GLFW_KEY_KP_ADD] = Keyboard::Definition::KeyPlus;
		v[GLFW_KEY_EQUAL] = Keyboard::Definition::KeyPlus;
		v[GLFW_KEY_KP_SUBTRACT] = Keyboard::Definition::KeyMinus;
		v[GLFW_KEY_MINUS] = Keyboard::Definition::KeyMinus;
		v[GLFW_KEY_COMMA] = Keyboard::Definition::KeyComma;
		v[GLFW_KEY_PERIOD] = Keyboard::Definition::KeyPeriod;
		v[GLFW_KEY_SLASH] = Keyboard::Definition::KeySlash;
		v[GLFW_KEY_BACKSLASH] = Keyboard::Definition::KeyBackslash;
		v[GLFW_KEY_GRAVE_ACCENT] = Keyboard::Definition::KeyGraveAccent;
		v[GLFW_KEY_CAPS_LOCK] = Keyboard::Definition::KeyCapsLock;
		v[GLFW_KEY_NUM_LOCK] = Keyboard::Definition::KeyNumLock;
		v[GLFW_KEY_SCROLL_LOCK] = Keyboard::Definition::KeyScrollLock;
		v[GLFW_KEY_F1] = Keyboard::Definition::KeyF1;
		v[GLFW_KEY_F2] = Keyboard::Definition::KeyF2;
		v[GLFW_KEY_F3] = Keyboard::Definition::KeyF3;
		v[GLFW_KEY_F4] = Keyboard::Definition::KeyF4;
		v[GLFW_KEY_F5] = Keyboard::Definition::KeyF5;
		v[GLFW_KEY_F6] = Keyboard::Definition::KeyF6;
		v[GLFW_KEY_F7] = Keyboard::Definition::KeyF7;
		v[GLFW_KEY_F8] = Keyboard::Definition::KeyF8;
		v[GLFW_KEY_F9] = Keyboard::Definition::KeyF9;
		v[GLFW_KEY_F10] = Keyboard::Definition::KeyF10;
		v[GLFW_KEY_F11] = Keyboard::Definition::KeyF11;
		v[GLFW_KEY_F12] = Keyboard::Definition::KeyF12;
		v[GLFW_KEY_KP_0] = Keyboard::Definition::KeyNumPad0;
		v[GLFW_KEY_KP_1] = Keyboard::Definition::KeyNumPad1;
		v[GLFW_KEY_KP_2] = Keyboard::Definition::KeyNumPad2;
		v[GLFW_KEY_KP_3] = Keyboard::Definition::KeyNumPad3;
		v[GLFW_KEY_KP_4] = Keyboard::Definition::KeyNumPad4;
		v[GLFW_KEY_KP_5] = Keyboard::Definition::KeyNumPad5;
		v[GLFW_KEY_KP_6] = Keyboard::Definition::KeyNumPad6;
		v[GLFW_KEY_KP_7] = Keyboard::Definition::KeyNumPad7;
		v[GLFW_KEY_KP_8] = Keyboard::Definition::KeyNumPad8;
		v[GLFW_KEY_KP_9] = Keyboard::Definition::KeyNumPad9;
		v[GLFW_KEY_0] = Keyboard::Definition::Key0;
		v[GLFW_KEY_1] = Keyboard::Definition::Key1;
		v[GLFW_KEY_2] = Keyboard::Definition::Key2;
		v[GLFW_KEY_3] = Keyboard::Definition::Key3;
		v[GLFW_KEY_4] = Keyboard::Definition::Key4;
		v[GLFW_KEY_5] = Keyboard::Definition::Key5;
		v[GLFW_KEY_6] = Keyboard::Definition::Key6;
		v[GLFW_KEY_7] = Keyboard::Definition::Key7;
		v[GLFW_KEY_8] = Keyboard::Definition::Key8;
		v[GLFW_KEY_9] = Keyboard::Definition::Key9;
		v[GLFW_KEY_A] = Keyboard::Definition::KeyA;
		v[GLFW_KEY_B] = Keyboard::Definition::KeyB;
		v[GLFW_KEY_C] = Keyboard::Definition::KeyC;
		v[GLFW_KEY_D] = Keyboard::Definition::KeyD;
		v[GLFW_KEY_E] = Keyboard::Definition::KeyE;
		v[GLFW_KEY_F] = Keyboard::Definition::KeyF;
		v[GLFW_KEY_G] = Keyboard::Definition::KeyG;
		v[GLFW_KEY_H] = Keyboard::Definition::KeyH;
		v[GLFW_KEY_I] = Keyboard::Definition::KeyI;
		v[GLFW_KEY_J] = Keyboard::Definition::KeyJ;
		v[GLFW_KEY_K] = Keyboard::Definition::KeyK;
		v[GLFW_KEY_L] = Keyboard::Definition::KeyL;
		v[GLFW_KEY_M] = Keyboard::Definition::KeyM;
		v[GLFW_KEY_N] = Keyboard::Definition::KeyN;
		v[GLFW_KEY_O] = Keyboard::Definition::KeyO;
		v[GLFW_KEY_P] = Keyboard::Definition::KeyP;
		v[GLFW_KEY_Q] = Keyboard::Definition::KeyQ;
		v[GLFW_KEY_R] = Keyboard::Definition::KeyR;
		v[GLFW_KEY_S] = Keyboard::Definition::KeyS;
		v[GLFW_KEY_T] = Keyboard::Definition::KeyT;
		v[GLFW_KEY_U] = Keyboard::Definition::KeyU;
		v[GLFW_KEY_V] = Keyboard::Definition::KeyV;
		v[GLFW_KEY_W] = Keyboard::Definition::KeyW;
		v[GLFW_KEY_X] = Keyboard::Definition::KeyX;
		v[GLFW_KEY_Y] = Keyboard::Definition::KeyY;
		v[GLFW_KEY_Z] = Keyboard::Definition::KeyZ;
		return v;
	}

	KeyboardKey PlatformImpl::translateKey(int key) noexcept
	{
		static auto keyMap = createKeyMap();
		if (key < 0 || key >= keyMap.size())
		{
			return Keyboard::Definition::KeyNone;
		}
		return keyMap[key];
	}

	MouseButton PlatformImpl::translateMouseButton(int button) noexcept
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			return Mouse::Definition::ButtonLeft;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			return Mouse::Definition::ButtonRight;
		}

		return Mouse::Definition::ButtonMiddle;
	}

	GamepadButton PlatformImpl::translateGamepadButton(int button) noexcept
	{
		switch (button)
		{
		case GLFW_GAMEPAD_BUTTON_A:
			return Gamepad::Definition::ButtonA;
		case GLFW_GAMEPAD_BUTTON_B:
			return Gamepad::Definition::ButtonB;
		case GLFW_GAMEPAD_BUTTON_X:
			return Gamepad::Definition::ButtonX;
		case GLFW_GAMEPAD_BUTTON_Y:
			return Gamepad::Definition::ButtonY;
		case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER:
			return Gamepad::Definition::ButtonLeftBumper;
		case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER:
			return Gamepad::Definition::ButtonRightBumper;
		case GLFW_GAMEPAD_BUTTON_BACK:
			return Gamepad::Definition::ButtonSelect;
		case GLFW_GAMEPAD_BUTTON_START:
			return Gamepad::Definition::ButtonStart;
		case GLFW_GAMEPAD_BUTTON_GUIDE:
			return Gamepad::Definition::ButtonGuide;
		case GLFW_GAMEPAD_BUTTON_LEFT_THUMB:
			return Gamepad::Definition::ButtonLeftThumb;
		case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB:
			return Gamepad::Definition::ButtonRightThumb;
		case GLFW_GAMEPAD_BUTTON_DPAD_UP:
			return Gamepad::Definition::ButtonPadUp;
		case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
			return Gamepad::Definition::ButtonPadRight;
		case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
			return Gamepad::Definition::ButtonPadDown;
		case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
			return Gamepad::Definition::ButtonPadLeft;
		default:
			return Gamepad::Definition::ButtonNone;
		};
	}

	std::optional<PlatformImpl::GamepadAxisConfig> PlatformImpl::translateGamepadAxis(int axis) noexcept
	{
		switch (axis)
		{
		case GLFW_GAMEPAD_AXIS_LEFT_X:
			return GamepadAxisConfig{ Gamepad::Definition::StickLeft, 0 };
		case GLFW_GAMEPAD_AXIS_LEFT_Y:
			return GamepadAxisConfig{ Gamepad::Definition::StickLeft, 1, true };
		case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER:
			return GamepadAxisConfig{ Gamepad::Definition::StickLeft, 2 };
		case GLFW_GAMEPAD_AXIS_RIGHT_X:
			return GamepadAxisConfig{ Gamepad::Definition::StickRight, 0 };
		case GLFW_GAMEPAD_AXIS_RIGHT_Y:
			return GamepadAxisConfig{ Gamepad::Definition::StickRight, 1, true };
		case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER:
			return GamepadAxisConfig{ Gamepad::Definition::StickRight, 2 };
		default:
			return std::nullopt;
		}
	}

	void PlatformImpl::errorCallback(int error, const char* description)
	{
		_events.post<WindowErrorEvent>(std::string("GLFW error ") + std::to_string(error) + ": " + std::string(description));
	}

	void PlatformImpl::updateGamepad(uint8_t num) noexcept
	{
		int id = GLFW_JOYSTICK_1 + num;

		int numAxes, numButtons = 0;
		const float* axes = nullptr;
		const unsigned char* buttons = nullptr;
		GLFWgamepadstate state;
		if (glfwJoystickIsGamepad(id))
		{
			if (glfwGetGamepadState(id, &state))
			{
				axes = state.axes;
				numAxes = GLFW_GAMEPAD_AXIS_LAST;
				buttons = state.buttons;
				numButtons = GLFW_GAMEPAD_BUTTON_LAST;
			}
		}
		else
		{
			// TODO: check if this works with non-xbox controllers
			axes = glfwGetJoystickAxes(id, &numAxes);
			buttons = glfwGetJoystickButtons(id, &numButtons);
		}
		if (axes)
		{
			std::unordered_map<GamepadStick, glm::vec3> stickValues;
			for (int i = 0; i <= numAxes; ++i)
			{
				auto config = translateGamepadAxis(i);
				if (!config)
				{
					continue;
				}
				auto value = axes[i];
				if (config->reverse)
				{
					value = -value;
				}
				auto idx = glm::vec3::length_type(config->index);
				stickValues[config->stick][idx] = value;
			}
			for (auto& elm : stickValues)
			{
				_events.post<GamepadStickEvent>(num, elm.first, elm.second);
			}
		}
		if (buttons)
		{
			for (int i = 0; i <= numButtons; ++i)
			{
				auto button = translateGamepadButton(i);
				auto down = buttons[i] == GLFW_PRESS;
				if (_gamepadButtons[num][button] != down)
				{
					_gamepadButtons[num][button] = down;
					_events.post<GamepadButtonEvent>(num, button, down);
				}
			}
		}
	}

	void PlatformImpl::updateGamepads() noexcept
	{
		for(uint8_t num = 0; num <= GLFW_JOYSTICK_LAST; ++num)
		{
			updateGamepad(num);
		}
	}

	expected<int, std::string> PlatformImpl::run(std::unique_ptr<IPlatformRunnable> runnable) noexcept
	{
		static const std::string winTitle{ "darmok" };
		
		_mte.runnable = std::move(runnable);

		glfwSetErrorCallback(staticErrorCallback);

		if (!glfwInit())
		{
			return unexpected<std::string>{ "glfwInit failed!" };
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		auto winSize = glm::uvec2{ DARMOK_DEFAULT_WIDTH, DARMOK_DEFAULT_HEIGHT };
		_window = createWindow(winSize, winTitle.c_str());

		if (!_window)
		{
			glfwTerminate();
			return unexpected<std::string>{ "glfwCreateWindow failed!" };
		}

		int left, right, top, bottom = 0;
		glfwGetWindowFrameSize(_window, &left, &top, &right, &bottom);
		_winFrameSize = { {left, top}, {right, bottom} };

		int w, h = 0;
		glfwGetFramebufferSize(_window, &w, &h);
		const glm::uvec2 pixelSize(w, h);

		{
			// send events for initial window state
			_events.post<VideoModeInfoEvent>(getVideoModeInfo(_winFrameSize));
			_events.post<WindowTitleEvent>(winTitle);
			_events.post<WindowSizeEvent>(winSize);
			_events.post<WindowPixelSizeEvent>(pixelSize);
			_events.post<WindowVideoModeEvent>(getVideoMode(_window));
			_events.post<WindowPhaseEvent>(WindowPhase::Running);
		}

		{
			// send event for initial mouse state
			double x, y = 0;
			glfwGetCursorPos(_window, &x, &y);
			_events.post<MousePositionEvent>(glm::vec2{ x, y });
		}

		_thread.init(MainThreadEntry::threadFunc, &_mte);

		while (!glfwWindowShouldClose(_window) && !_mte.finished)
		{
			glfwWaitEventsTimeout(0.016);
			updateGamepads();

			std::lock_guard cmdsLock{ _cmdsMutex };
			while (!_cmds.empty())
			{
				std::unique_ptr<PlatformCmd> cmd = std::move(_cmds.front());
				_cmds.pop();
				if (!PlatformCmd::process(*cmd, *this))
				{
					_cmds.push(std::move(cmd));
				}
			}
		}

		_events.post<WindowPhaseEvent>(WindowPhase::Destroyed);
		_thread.shutdown();

		destroyWindow(_window);
		glfwTerminate();

		return _thread.getExitCode();
	}

	void PlatformImpl::keyCallback(GLFWwindow* window, int32_t key, int32_t chr, int32_t action, int32_t mods) noexcept
	{
		BX_UNUSED(window, chr);
		if (key == GLFW_KEY_UNKNOWN)
		{
			return;
		}
		auto mods2 = translateKeyModifiers(mods);
		auto key2 = translateKey(key);
		if (key2 == Keyboard::Definition::KeyNone)
		{
			return;
		}
		bool down = (action == GLFW_PRESS || action == GLFW_REPEAT);
		_events.post<KeyboardKeyEvent>(key2, mods2, down);
	}

	void PlatformImpl::charCallback(GLFWwindow* window, uint32_t chr) noexcept
	{
		BX_UNUSED(window);
		_events.post<KeyboardCharEvent>(chr);
	}

	void PlatformImpl::scrollCallback(GLFWwindow* window, double scrollx, double scrolly) noexcept
	{
		_events.post<MouseScrollEvent>(glm::vec2{ scrollx, scrolly });
	}

	void PlatformImpl::cursorPosCallback(GLFWwindow* window, double posx, double posy) noexcept
	{
		_events.post<MousePositionEvent>(glm::vec2{ posx, posy });
	}

	void PlatformImpl::cursorEnterCallback(GLFWwindow* window, int entered) noexcept
	{
		_events.post<MouseActiveEvent>((bool)entered);
	}

	void PlatformImpl::mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) noexcept
	{
		BX_UNUSED(window, mods);
		bool down = action == GLFW_PRESS;
		_events.post<MouseButtonEvent>(
			translateMouseButton(button)
			, down
		);
	}

	void PlatformImpl::windowSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept
	{
		if (width == 0 || height == 0)
		{
			return;
		}
		const glm::uvec2 size(width, height);
		_events.post<WindowSizeEvent>(size);
	}

	void PlatformImpl::framebufferSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept
	{
		if (width == 0 || height == 0)
		{
			return;
		}
		glm::uvec2 size(width, height);
		_events.post<WindowPixelSizeEvent>(size);
	}

	void PlatformImpl::joystickCallback(int jid, int action) noexcept
	{
		if (action == GLFW_CONNECTED)
		{
			_events.post<GamepadConnectEvent>(jid, true);
		}
		else if (action == GLFW_DISCONNECTED)
		{
			_events.post<GamepadConnectEvent>(jid, false);
		}
	}

	void PlatformImpl::staticJoystickCallback(int jid, int action) noexcept
	{
		Platform::get().getImpl().joystickCallback(jid, action);
	}

	void PlatformImpl::staticErrorCallback(int error, const char* description) noexcept
	{
		Platform::get().getImpl().errorCallback(error, description);
	}

	void PlatformImpl::staticKeyCallback(GLFWwindow* window, int32_t key, int32_t chr, int32_t action, int32_t mods) noexcept
	{
		Platform::get().getImpl().keyCallback(window, key, chr, action, mods);
	}

	void PlatformImpl::staticCharCallback(GLFWwindow* window, uint32_t chr) noexcept
	{
		Platform::get().getImpl().charCallback(window, chr);
	}

	void PlatformImpl::staticScrollCallback(GLFWwindow* window, double dx, double dy) noexcept
	{
		Platform::get().getImpl().scrollCallback(window, dx, dy);
	}

	void PlatformImpl::staticCursorPosCallback(GLFWwindow* window, double x, double y) noexcept
	{
		Platform::get().getImpl().cursorPosCallback(window, x, y);
	}

	void PlatformImpl::staticCursorEnterCallback(GLFWwindow* window, int entered) noexcept
	{
		Platform::get().getImpl().cursorEnterCallback(window, entered);
	}

	void PlatformImpl::staticMouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) noexcept
	{
		Platform::get().getImpl().mouseButtonCallback(window, button, action, mods);
	}

	void PlatformImpl::staticWindowSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept
	{
		Platform::get().getImpl().windowSizeCallback(window, width, height);
	}

	void PlatformImpl::staticFramebufferSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept
	{
		Platform::get().getImpl().framebufferSizeCallback(window, width, height);
	};

	void PlatformImpl::pushCmd(std::unique_ptr<PlatformCmd>&& cmd) noexcept
	{
		std::lock_guard cmdsLock{ _cmdsMutex };
		_cmds.push(std::move(cmd));
	}

	Viewport PlatformImpl::getMonitorWorkarea(GLFWmonitor* mon) noexcept
	{
		int x, y, w, h;
		glfwGetMonitorWorkarea(mon, &x, &y, &w, &h);
		return Viewport(x, y, w, h);
	}

	void PlatformImpl::resetWindowMonitor(GLFWwindow* win, const glm::uvec2& pos, const glm::uvec2& size) noexcept
	{
		glfwSetWindowSizeCallback(win, nullptr);
		glfwSetFramebufferSizeCallback(win, nullptr);
		glfwSetWindowMonitor(win, nullptr,
			pos.x, pos.y, size.x, size.y, GLFW_DONT_CARE);
		glfwSetWindowSizeCallback(win, staticWindowSizeCallback);
		glfwSetFramebufferSizeCallback(win, staticFramebufferSizeCallback);
	}

	VideoMode PlatformImpl::getVideoMode(GLFWwindow* win, GLFWmonitor* mon) noexcept
	{
		int monitorIndex = 0;
		if (mon == 0)
		{
			mon = glfwGetPrimaryMonitor();
		}
		else
		{
			int monitorCount = 0;
			auto monitors = glfwGetMonitors(&monitorCount);
			for (int i = 0; i < monitorCount; i++)
			{
				if (monitors[i] == mon)
				{
					monitorIndex = i;
					break;
				}
			}
		}
		auto glfwMode = glfwGetVideoMode(mon);
		auto size = glm::uvec2(glfwMode->width, glfwMode->height);
		auto screenMode = WindowScreenMode::Fullscreen;
		if (win)
		{
			if (glfwGetWindowMonitor(win) == mon)
			{
				screenMode = WindowScreenMode::Fullscreen;
			}
			else if (!glfwGetWindowAttrib(win, GLFW_DECORATED))
			{
				screenMode = WindowScreenMode::WindowedFullscreen;
			}
			else
			{
				screenMode = WindowScreenMode::Normal;
			}
			int w, h;
			glfwGetWindowSize(win, &w, &h);
			size.x = w;
			size.y = h;
		}

		return {
			.screenMode = screenMode,
			.size = size,
			.depth = Color3(glfwMode->redBits, glfwMode->greenBits, glfwMode->blueBits),
			.refreshRate = (uint16_t)glfwMode->refreshRate,
			.monitor = monitorIndex
		};
	}

	VideoModeInfo PlatformImpl::getVideoModeInfo(const WindowFrameSize& frame) noexcept
	{
		int monitorCount;
		VideoModeInfo info;
		auto monitors = glfwGetMonitors(&monitorCount);

		for (int i = 0; i < monitorCount; i++)
		{
			auto glfwMon = monitors[i];
			auto& mon = info.monitors.emplace_back();
			mon.name = glfwGetMonitorName(glfwMon);
			mon.workarea = getMonitorWorkarea(glfwMon);
			int modeCount = 0;
			auto glfwModes = glfwGetVideoModes(glfwMon, &modeCount);
			auto monSize = mon.workarea.size - mon.workarea.origin;
			auto defMode = getVideoMode(nullptr, glfwMon);

			for (int j = 0; j < modeCount; j++)
			{
				auto glfwMode = glfwModes[j];
				auto& mode = info.modes.emplace_back(WindowScreenMode::Fullscreen);
				mode.size.x = glfwMode.width;
				mode.size.y = glfwMode.height;
				mode.depth.r = glfwMode.redBits;
				mode.depth.g = glfwMode.greenBits;
				mode.depth.b = glfwMode.blueBits;
				mode.refreshRate = glfwMode.refreshRate;
				mode.monitor = i;

				auto winSize = frame.topLeft + mode.size + frame.botRight;
				if (winSize.x <= monSize.x && winSize.y <= monSize.y)
				{
					auto winMode = defMode;
					winMode.size = mode.size;
					winMode.screenMode = WindowScreenMode::Normal;
					auto itr = std::find(info.modes.begin(), info.modes.end(), winMode);
					if (itr == info.modes.end())
					{
						info.modes.push_back(winMode);
					}
				}
			}

			auto& wfMode = info.modes.emplace_back(defMode);
			wfMode.monitor = i;
			wfMode.screenMode = WindowScreenMode::WindowedFullscreen;
		}

		return info;
	}

	std::unique_ptr<PlatformEvent> PlatformImpl::pollEvent() noexcept
	{
		return _events.poll();
	}

	int32_t MainThreadEntry::threadFunc(bx::Thread* thread, void* userData)
	{
		BX_UNUSED(thread);
		auto self = (MainThreadEntry*)userData;
		auto result = (*self->runnable)();
		self->finished = true;
		return result;
	}

	Platform::Platform() noexcept
		: _impl(std::make_unique<PlatformImpl>(*this))
	{
	}

	Platform::~Platform() noexcept
	{
		// empty on purpose
	}

	std::unique_ptr<PlatformEvent> Platform::pollEvent() noexcept
	{
		return _impl->pollEvent();
	}

	expected<int32_t, std::string> Platform::run(std::unique_ptr<IPlatformRunnable> runnable) noexcept
	{
		return _impl->run(std::move(runnable));
	}

	void Platform::requestWindowDestruction() noexcept
	{
		_impl->pushCmd<DestroyWindowCmd>();
	}

	void Platform::requestVideoModeInfo() noexcept
	{
		_impl->pushCmd<RequestVideoModeInfoCmd>();
	}

	void Platform::requestWindowVideoModeChange(VideoMode mode) noexcept
	{
		_impl->pushCmd<ChangeWindowVideoModeCmd>(std::move(mode));
	}

	void Platform::requestWindowCursorModeChange(WindowCursorMode mode) noexcept
	{
		_impl->pushCmd<ChangeWindowCursorModeCmd>(mode);
	}

	void Platform::requestWindowTitle(std::string title) noexcept
	{
		_impl->pushCmd<ChangeWindowTitleCmd>(std::move(title));
	}

	void Platform::openFileDialog(FileDialogOptions options, FileDialogCallback callback) noexcept
	{
		_impl->pushCmd<OpenFileDialogCmd>(std::move(options), std::move(callback));
	}

	void* Platform::getWindowHandle() const noexcept
	{
		return PlatformImpl::getWindowHandle(_impl->getGlfwWindow());
	}

	void* Platform::getDisplayHandle() const noexcept
	{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
        if(glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
        {
            return glfwGetWaylandDisplay();
        }
		return glfwGetX11Display();
#	else
		return nullptr;
#	endif // BX_PLATFORM_*
	}

	bgfx::NativeWindowHandleType::Enum Platform::getWindowHandleType() const noexcept
	{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
        if(glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
        {
            return bgfx::NativeWindowHandleType::Wayland;
        }
		return bgfx::NativeWindowHandleType::Default;
#	else
		return bgfx::NativeWindowHandleType::Default;
#	endif // BX_PLATFORM_*
	}
}
