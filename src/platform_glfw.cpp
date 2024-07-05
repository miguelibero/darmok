#include "platform_glfw.hpp"

#include "input.hpp"
#include "window.hpp"
#include "app.hpp"
#include <darmok/utils.hpp>
#include <darmok/window.hpp>
#include <darmok/app.hpp>
#include <stdexcept>

#include <bx/mutex.h>

#include <GLFW/glfw3native.h>

namespace darmok
{
#pragma region PlatformCmds

	PlatformCmd::PlatformCmd(Type type) noexcept
		: _type(type)
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

	ChangeWindowVideoModeCmd::ChangeWindowVideoModeCmd(const VideoMode& mode) noexcept
		: PlatformCmd(ChangeWindowVideoMode)
		, _mode(mode)
	{
	}

	std::expected<GLFWmonitor*, std::string> ChangeWindowVideoModeCmd::getMonitor() noexcept
	{
		if (_mode.monitor <= 0)
		{
			return glfwGetPrimaryMonitor();
		}
		int monCount;
		auto monitors = glfwGetMonitors(&monCount);
		if (_mode.monitor >= monCount)
		{
			return std::unexpected("invalid monitor");
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
		glm::uvec2 pixelSize(0);

		// reset window monitor
		glfwSetWindowMonitor(win, nullptr, 0, 0, 1, 1, GLFW_DONT_CARE);

		if (_mode.size.x == 0 || _mode.size.y == 0)
		{
			auto defMode = glfwGetVideoMode(monitor);
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
					return;
				}
				glfwSetWindowMonitor(win
					, nullptr
					, workArea.origin.x + frame.topLeft.x
					, workArea.origin.x + frame.topLeft.y
					, _mode.size.x
					, _mode.size.y
					, refreshRate
				);
				glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
				break;
			}
			case WindowScreenMode::Fullscreen:
			{
				glfwSetWindowMonitor(win
					, monitor
					, 0
					, 0
					, _mode.size.x
					, _mode.size.y
					, refreshRate
				);
				break;
			}
			case WindowScreenMode::WindowedFullscreen:
			{
				auto workArea = PlatformImpl::getMonitorWorkarea(monitor);
				glfwSetWindowMonitor(win
					, nullptr
					, workArea.origin.x
					, workArea.origin.y
					, workArea.size.x
					, workArea.size.y
					, refreshRate
				);
				pixelSize = _mode.size;
				glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
				break;
			}
			default:
				events.post<WindowErrorEvent>("unsupported screen mode");
				return;
		}

		glfwSetWindowAspectRatio(win, _mode.size.x, _mode.size.y);
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

		events.post<WindowSizeEvent>(pixelSize, WindowSizeType::Pixel);
		events.post<WindowVideoModeEvent>(_mode);
	}

	ChangeWindowCursorModeCmd::ChangeWindowCursorModeCmd(WindowCursorMode value) noexcept
		: PlatformCmd(ChangeWindowCursorMode)
		, _value(value)
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
		glfwSetInputMode(glfw, GLFW_CURSOR, v);
	}

	void PlatformCmd::process(PlatformCmd& cmd, PlatformImpl& plat) noexcept
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
		}
	}

#pragma endregion PlatformCmds	

	PlatformImpl::PlatformImpl(Platform& plat) noexcept
		: _plat(plat)
		, _window(nullptr)
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
# 		if DARMOK_PLATFORM_SUPPORT_WAYLAND
		if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
		{
			wl_egl_window* win_impl = (wl_egl_window*)glfwGetWindowUserPointer(window);
			if (!win_impl)
			{
				int width, height;
				glfwGetWindowSize(window, &width, &height);
				struct wl_surface* surface = (struct wl_surface*)glfwGetWaylandWindow(window);
				if (!surface) {
					return nullptr;
				}
				win_impl = wl_egl_window_create(surface, width, height);
				glfwSetWindowUserPointer(window, (void*)(uintptr_t)win_impl);
			}
			return (void*)(uintptr_t)win_impl;
		}
#		endif
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
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#		if DARMOK_CONFIG_USE_WAYLAND
		wl_egl_window* win_impl = (wl_egl_window*)glfwGetWindowUserPointer(window);
		if (win_impl)
		{
			glfwSetWindowUserPointer(window, nullptr);
			wl_egl_window_destroy(win_impl);
		}
#		endif
#	endif
		glfwDestroyWindow(window);
	}


	GLFWwindow* PlatformImpl::createWindow(const glm::uvec2& size, const char* title) noexcept
	{
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
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

		glfwSetWindowAspectRatio(window, size.x, size.y);
		
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

	uint8_t PlatformImpl::translateKeyModifiers(int glfw) noexcept
	{
		uint8_t modifiers = 0;

		if (static_cast<bool>(glfw & GLFW_MOD_ALT))
		{
			modifiers |= to_underlying(KeyboardModifier::LeftAlt);
		}

		if (static_cast<bool>(glfw & GLFW_MOD_CONTROL))
		{
			modifiers |= to_underlying(KeyboardModifier::LeftCtrl);
		}

		if (static_cast<bool>(glfw & GLFW_MOD_SUPER))
		{
			modifiers |= to_underlying(KeyboardModifier::LeftMeta);
		}

		if (static_cast<bool>(glfw & GLFW_MOD_SHIFT))
		{
			modifiers |= to_underlying(KeyboardModifier::LeftShift);
		}

		return modifiers;
	}

	PlatformImpl::KeyMap PlatformImpl::createKeyMap() noexcept
	{
		KeyMap v{};
		v[GLFW_KEY_ESCAPE] = KeyboardKey::Esc;
		v[GLFW_KEY_ENTER] = KeyboardKey::Return;
		v[GLFW_KEY_TAB] = KeyboardKey::Tab;
		v[GLFW_KEY_BACKSPACE] = KeyboardKey::Backspace;
		v[GLFW_KEY_SPACE] = KeyboardKey::Space;
		v[GLFW_KEY_UP] = KeyboardKey::Up;
		v[GLFW_KEY_DOWN] = KeyboardKey::Down;
		v[GLFW_KEY_LEFT] = KeyboardKey::Left;
		v[GLFW_KEY_RIGHT] = KeyboardKey::Right;
		v[GLFW_KEY_PAGE_UP] = KeyboardKey::PageUp;
		v[GLFW_KEY_PAGE_DOWN] = KeyboardKey::PageDown;
		v[GLFW_KEY_HOME] = KeyboardKey::Home;
		v[GLFW_KEY_END] = KeyboardKey::End;
		v[GLFW_KEY_PRINT_SCREEN] = KeyboardKey::Print;
		v[GLFW_KEY_KP_ADD] = KeyboardKey::Plus;
		v[GLFW_KEY_EQUAL] = KeyboardKey::Plus;
		v[GLFW_KEY_KP_SUBTRACT] = KeyboardKey::Minus;
		v[GLFW_KEY_MINUS] = KeyboardKey::Minus;
		v[GLFW_KEY_COMMA] = KeyboardKey::Comma;
		v[GLFW_KEY_PERIOD] = KeyboardKey::Period;
		v[GLFW_KEY_SLASH] = KeyboardKey::Slash;
		v[GLFW_KEY_BACKSLASH] = KeyboardKey::Backslash;
		v[GLFW_KEY_GRAVE_ACCENT] = KeyboardKey::GraveAccent;
		v[GLFW_KEY_CAPS_LOCK] = KeyboardKey::CapsLock;
		v[GLFW_KEY_NUM_LOCK] = KeyboardKey::NumLock;
		v[GLFW_KEY_SCROLL_LOCK] = KeyboardKey::ScrollLock;
		v[GLFW_KEY_F1] = KeyboardKey::F1;
		v[GLFW_KEY_F2] = KeyboardKey::F2;
		v[GLFW_KEY_F3] = KeyboardKey::F3;
		v[GLFW_KEY_F4] = KeyboardKey::F4;
		v[GLFW_KEY_F5] = KeyboardKey::F5;
		v[GLFW_KEY_F6] = KeyboardKey::F6;
		v[GLFW_KEY_F7] = KeyboardKey::F7;
		v[GLFW_KEY_F8] = KeyboardKey::F8;
		v[GLFW_KEY_F9] = KeyboardKey::F9;
		v[GLFW_KEY_F10] = KeyboardKey::F10;
		v[GLFW_KEY_F11] = KeyboardKey::F11;
		v[GLFW_KEY_F12] = KeyboardKey::F12;
		v[GLFW_KEY_KP_0] = KeyboardKey::NumPad0;
		v[GLFW_KEY_KP_1] = KeyboardKey::NumPad1;
		v[GLFW_KEY_KP_2] = KeyboardKey::NumPad2;
		v[GLFW_KEY_KP_3] = KeyboardKey::NumPad3;
		v[GLFW_KEY_KP_4] = KeyboardKey::NumPad4;
		v[GLFW_KEY_KP_5] = KeyboardKey::NumPad5;
		v[GLFW_KEY_KP_6] = KeyboardKey::NumPad6;
		v[GLFW_KEY_KP_7] = KeyboardKey::NumPad7;
		v[GLFW_KEY_KP_8] = KeyboardKey::NumPad8;
		v[GLFW_KEY_KP_9] = KeyboardKey::NumPad9;
		v[GLFW_KEY_0] = KeyboardKey::Key0;
		v[GLFW_KEY_1] = KeyboardKey::Key1;
		v[GLFW_KEY_2] = KeyboardKey::Key2;
		v[GLFW_KEY_3] = KeyboardKey::Key3;
		v[GLFW_KEY_4] = KeyboardKey::Key4;
		v[GLFW_KEY_5] = KeyboardKey::Key5;
		v[GLFW_KEY_6] = KeyboardKey::Key6;
		v[GLFW_KEY_7] = KeyboardKey::Key7;
		v[GLFW_KEY_8] = KeyboardKey::Key8;
		v[GLFW_KEY_9] = KeyboardKey::Key9;
		v[GLFW_KEY_A] = KeyboardKey::KeyA;
		v[GLFW_KEY_B] = KeyboardKey::KeyB;
		v[GLFW_KEY_C] = KeyboardKey::KeyC;
		v[GLFW_KEY_D] = KeyboardKey::KeyD;
		v[GLFW_KEY_E] = KeyboardKey::KeyE;
		v[GLFW_KEY_F] = KeyboardKey::KeyF;
		v[GLFW_KEY_G] = KeyboardKey::KeyG;
		v[GLFW_KEY_H] = KeyboardKey::KeyH;
		v[GLFW_KEY_I] = KeyboardKey::KeyI;
		v[GLFW_KEY_J] = KeyboardKey::KeyJ;
		v[GLFW_KEY_K] = KeyboardKey::KeyK;
		v[GLFW_KEY_L] = KeyboardKey::KeyL;
		v[GLFW_KEY_M] = KeyboardKey::KeyM;
		v[GLFW_KEY_N] = KeyboardKey::KeyN;
		v[GLFW_KEY_O] = KeyboardKey::KeyO;
		v[GLFW_KEY_P] = KeyboardKey::KeyP;
		v[GLFW_KEY_Q] = KeyboardKey::KeyQ;
		v[GLFW_KEY_R] = KeyboardKey::KeyR;
		v[GLFW_KEY_S] = KeyboardKey::KeyS;
		v[GLFW_KEY_T] = KeyboardKey::KeyT;
		v[GLFW_KEY_U] = KeyboardKey::KeyU;
		v[GLFW_KEY_V] = KeyboardKey::KeyV;
		v[GLFW_KEY_W] = KeyboardKey::KeyW;
		v[GLFW_KEY_X] = KeyboardKey::KeyX;
		v[GLFW_KEY_Y] = KeyboardKey::KeyY;
		v[GLFW_KEY_Z] = KeyboardKey::KeyZ;
		return v;
	}

	KeyboardKey PlatformImpl::translateKey(int key) noexcept
	{
		static auto keyMap = createKeyMap();
		if (key < 0 || key >= keyMap.size())
		{
			return KeyboardKey::Count;
		}
		return keyMap[key];
	}

	MouseButton PlatformImpl::translateMouseButton(int button) noexcept
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			return MouseButton::Left;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			return MouseButton::Right;
		}

		return MouseButton::Middle;
	}

	std::optional<PlatformImpl::GamepadAxisConfig> PlatformImpl::translateGamepadAxis(int axis) noexcept
	{
		// Map XInput 360 controller until GLFW gamepad API
		const static std::array<GamepadAxisConfig, 6> axes =
		{
			GamepadAxisConfig{ GamepadStick::Left, 0 },
			GamepadAxisConfig{ GamepadStick::Left, 1, true },
			GamepadAxisConfig{ GamepadStick::Right, 0 },
			GamepadAxisConfig{ GamepadStick::Right, 1, true },
			GamepadAxisConfig{ GamepadStick::Left, 2 },
			GamepadAxisConfig{ GamepadStick::Right, 2 },
		};
		if (axis < 0 || axis >= axes.size())
		{
			return std::nullopt;
		}
		return axes[axis];
	}

	GamepadButton PlatformImpl::translateGamepadButton(int button) noexcept
	{
		// HACK: Map XInput 360 controller until GLFW gamepad API

		static std::array<GamepadButton, 15> buttons =
		{
			GamepadButton::A,
			GamepadButton::B,
			GamepadButton::X,
			GamepadButton::Y,
			GamepadButton::ShoulderL,
			GamepadButton::ShoulderR,
			GamepadButton::Back,
			GamepadButton::Start,
			GamepadButton::ThumbL,
			GamepadButton::ThumbR,
			GamepadButton::Up,
			GamepadButton::Right,
			GamepadButton::Down,
			GamepadButton::Left,
			GamepadButton::Guide,
		};
		return buttons[button];
	}

	void PlatformImpl::errorCallback(int error, const char* description)
	{
		_events.post<WindowErrorEvent>(std::string("GLFW error ") + std::to_string(error) + ": " + std::string(description));
	}

	void PlatformImpl::updateGamepad(uint8_t num) noexcept
	{
		int numButtons, numAxes;
		const unsigned char* buttons = glfwGetJoystickButtons(num, &numButtons);
		const float* axes = glfwGetJoystickAxes(num, &numAxes);

		if (axes != nullptr)
		{
			std::unordered_map<GamepadStick, glm::vec3> stickValues;
			for (int i = 0; i < numAxes; ++i)
			{
				auto value = axes[i];
				auto config = translateGamepadAxis(i);
				if (!config)
				{
					continue;
				}
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

		if (buttons != nullptr)
		{
			for (int i = 0; i < numButtons; ++i)
			{
				auto button = translateGamepadButton(i);
				bool down = buttons[i];
				_events.post<GamepadButtonEvent>(num, button, down);
			}
		}
	}

	void PlatformImpl::updateGamepads() noexcept
	{
		for(uint8_t num = 0; num < Gamepad::MaxAmount; num++)
		{
			updateGamepad(num);
		}
	}

	int PlatformImpl::run(std::unique_ptr<IPlatformRunnable>&& runnable)
	{
		_mte.runnable = std::move(runnable);

		glfwSetErrorCallback(staticErrorCallback);

		if (!glfwInit())
		{
			throw std::runtime_error("glfwInit failed!");
			return bx::kExitFailure;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		auto winSize = glm::uvec2{ DARMOK_DEFAULT_WIDTH, DARMOK_DEFAULT_HEIGHT };
		_window = createWindow(winSize, "darmok");

		if (!_window)
		{
			glfwTerminate();
			throw std::runtime_error("glfwCreateWindow failed!");
			return bx::kExitFailure;
		}

		int left, right, top, bottom;
		glfwGetWindowFrameSize(_window, &left, &top, &right, &bottom);
		_winFrameSize = { {left, top}, {right, bottom} };

		int w, h;
		glfwGetFramebufferSize(_window, &w, &h);
		glm::uvec2 fbSize(w, h);


		{
			// send events for initial window state
			_events.post<VideoModeInfoEvent>(getVideoModeInfo(_winFrameSize));
			_events.post<WindowSizeEvent>(winSize, WindowSizeType::Size);
			_events.post<WindowSizeEvent>(fbSize, WindowSizeType::Framebuffer);
			_events.post<WindowVideoModeEvent>(getVideoMode(_window));
			_events.post<WindowPhaseEvent>(WindowPhase::Running);
		}

		{
			// send event for initial mouse state
			double x, y;
			glfwGetCursorPos(_window, &x, &y);
			_events.post<MousePositionEvent>(glm::vec2{ x, y });
		}

		_thread.init(MainThreadEntry::threadFunc, &_mte);

		while (!glfwWindowShouldClose(_window) && !_mte.finished)
		{
			glfwWaitEventsTimeout(0.016);
			updateGamepads();

			std::lock_guard cmdsLock(_cmdsMutex);
			while (!_cmds.empty())
			{
				std::unique_ptr<PlatformCmd> cmd = std::move(_cmds.front());
				_cmds.pop();
				PlatformCmd::process(*cmd, *this);
			}
		}

		_events.post<WindowPhaseEvent>(WindowPhase::Destroyed);
		_thread.shutdown();

		destroyWindow(_window);
		glfwTerminate();

		return _thread.getExitCode();
	}

	void PlatformImpl::keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) noexcept
	{
		BX_UNUSED(window, scancode);
		if (key == GLFW_KEY_UNKNOWN)
		{
			return;
		}
		int mods2 = translateKeyModifiers(mods);
		KeyboardKey key2 = translateKey(key);
		if (key2 == KeyboardKey::Count)
		{
			return;
		}
		bool down = (action == GLFW_PRESS || action == GLFW_REPEAT);
		_events.post<KeyboardKeyEvent>(key2, mods2, down);
	}

	void PlatformImpl::charCallback(GLFWwindow* window, uint32_t scancode) noexcept
	{
		BX_UNUSED(window);
		auto data = Utf8Char::encode(scancode);
		if (!data.len)
		{
			return;
		}
		_events.post<KeyboardCharEvent>(data);
	}

	void PlatformImpl::scrollCallback(GLFWwindow* window, double dx, double dy) noexcept
	{
		_events.post<MouseScrollEvent>(glm::vec2{ dx, dy });
	}

	void PlatformImpl::cursorPosCallback(GLFWwindow* window, double x, double y) noexcept
	{
		_events.post<MousePositionEvent>(glm::vec2{ x, y });
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
		glm::uvec2 size(width, height);
		_events.post<WindowSizeEvent>(size, WindowSizeType::Size);
	}

	void PlatformImpl::framebufferSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept
	{
		glm::uvec2 size(width, height);
		_events.post<WindowSizeEvent>(size, WindowSizeType::Framebuffer);
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

	void PlatformImpl::staticKeyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) noexcept
	{
		Platform::get().getImpl().keyCallback(window, key, scancode, action, mods);
	}

	void PlatformImpl::staticCharCallback(GLFWwindow* window, uint32_t scancode) noexcept
	{
		Platform::get().getImpl().charCallback(window, scancode);
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
		std::lock_guard cmdsLock(_cmdsMutex);
		_cmds.push(std::move(cmd));
	}

	Viewport PlatformImpl::getMonitorWorkarea(GLFWmonitor* mon) noexcept
	{
		int x, y, w, h;
		glfwGetMonitorWorkarea(mon, &x, &y, &w, &h);
		return Viewport(x, y, w, h);
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
			int monitorCount;
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

		auto defMode = getVideoMode();
		std::vector<glm::uvec2> resolutions;

		for (int i = 0; i < monitorCount; i++)
		{
			auto glfwMon = monitors[i];
			auto& mon = info.monitors.emplace_back();
			mon.name = glfwGetMonitorName(glfwMon);
			mon.workarea = getMonitorWorkarea(glfwMon);
			int modeCount;
			auto glfwModes = glfwGetVideoModes(glfwMon, &modeCount);
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

				auto itr = std::find(resolutions.begin(), resolutions.end(), mode.size);
				if (itr == resolutions.end())
				{
					resolutions.push_back(mode.size);
				}
			}
		}
		auto& mon = info.monitors[0];
		auto monSize = mon.workarea.size - mon.workarea.origin;
		auto mode = getVideoMode(nullptr, monitors[0]);
		mode.monitor = -1;
		for (auto& size : resolutions)
		{
			for (int i = 0; i < monitorCount; i++)
			{
				// TODO: is there a way of checking if GLFW_DECORATED=false is supported in the platform?
				auto& wfMode = info.modes.emplace_back(mode);
				wfMode.screenMode = WindowScreenMode::WindowedFullscreen;
				wfMode.size = size;
				wfMode.monitor = i;
			}

			auto winSize = frame.topLeft + size + frame.botRight;
			if (winSize.x <= monSize.x && winSize.y <= monSize.y)
			{
				auto& winMode = info.modes.emplace_back(mode);
				winMode.screenMode = WindowScreenMode::Normal;
				winMode.size = size;
			}
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
	}

	std::unique_ptr<PlatformEvent> Platform::pollEvent() noexcept
	{
		return _impl->pollEvent();
	}

	int32_t Platform::run(std::unique_ptr<IPlatformRunnable>&& runnable)
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

	void Platform::requestWindowVideoModeChange(const VideoMode& mode) noexcept
	{
		_impl->pushCmd<ChangeWindowVideoModeCmd>(mode);
	}

	void Platform::requestWindowCursorModeChange(WindowCursorMode mode) noexcept
	{
		_impl->pushCmd<ChangeWindowCursorModeCmd>(mode);
	}

	void* Platform::getWindowHandle() const noexcept
	{
		return PlatformImpl::getWindowHandle(_impl->getGlfwWindow());
	}

	void* Platform::getDisplayHandle() const noexcept
	{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#       if DARMOK_PLATFORM_SUPPORT_WAYLAND
        if(glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
        {
            return glfwGetWaylandDisplay();
        }
#       endif
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