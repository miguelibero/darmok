#include "platform.hpp"
#include "input.hpp"
#include "window.hpp"
#include <darmok/utils.hpp>
#include <darmok/window.hpp>
#include <bx/platform.h>

#if DARMOK_PLATFORM_GLFW

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if GLFW_VERSION_MINOR < 2
#	error "GLFW 3.2 or later is required"
#endif // GLFW_VERSION_MINOR < 2

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#	if DARMOK_PLATFORM_SUPPORT_WAYLAND
#		include <wayland-egl.h>
#		define GLFW_EXPOSE_NATIVE_WAYLAND
#	endif
#		define GLFW_EXPOSE_NATIVE_X11
#		define GLFW_EXPOSE_NATIVE_GLX
#elif BX_PLATFORM_OSX
#	define GLFW_EXPOSE_NATIVE_COCOA
#	define GLFW_EXPOSE_NATIVE_NSGL
#elif BX_PLATFORM_WINDOWS
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define GLFW_EXPOSE_NATIVE_WGL
#endif //
#include <GLFW/glfw3native.h>

#include <bgfx/platform.h>
#include <bx/handlealloc.h>
#include <bx/thread.h>
#include <bx/mutex.h>

#include "dbg.h"

namespace darmok
{
	struct MainThreadEntry
	{
		int argc = 0;
		const char* const* argv = nullptr;
		bool finished = false;

		static int32_t threadFunc(bx::Thread* thread, void* userData);
	};

#pragma region PlatformImpl definition

	class PlatformCmd;

	class PlatformImpl final
	{
	public:
		PlatformImpl() noexcept;

		int run(int argc, const char* const* argv) noexcept;
		
		void pushCmd(std::unique_ptr<PlatformCmd>&& cmd) noexcept;
		std::unique_ptr<PlatformEvent> pollEvent() noexcept;

		template<typename T, typename... A>
		T& pushCmd(A&&... args) noexcept
		{
			auto ptr = new T(std::forward<A>(args)...);
			pushCmd(std::unique_ptr<PlatformCmd>(ptr));
			return *ptr;
		}

		GLFWwindow* getGlfwWindow() const noexcept;
		PlatformEventQueue& getEvents() noexcept;
		
		[[nodiscard]] static void* getWindowHandle(GLFWwindow* window) noexcept;
		static void destroyWindow(GLFWwindow* window) noexcept;
		[[nodiscard]] static GLFWwindow* createWindow(const glm::uvec2& size, const char* title) noexcept;

	private:
		GLFWwindow* _window;
		PlatformEventQueue _events;
		MainThreadEntry _mte;
		bx::Thread _thread;
		std::queue<std::unique_ptr<PlatformCmd>> _cmds;
		glm::uvec2 _windowSize;
		glm::uvec2 _framebufferSize;

		glm::vec2 normalizeScreenPoint(double x, double y) noexcept;

		static uint8_t translateKeyModifiers(int mods) noexcept;
		static KeyboardKey translateKey(int key) noexcept;
		static MouseButton translateMouseButton(int button) noexcept;
		static GamepadAxis translateGamepadAxis(int axis) noexcept;
		static GamepadButton translateGamepadButton(int button) noexcept;

		void updateGamepads() noexcept;
		void updateGamepad(uint8_t num) noexcept;

		void joystickCallback(int jid, int action) noexcept;
		void errorCallback(int error, const char* description) noexcept;
		void keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) noexcept;
		void charCallback(GLFWwindow* window, uint32_t scancode) noexcept;
		void scrollCallback(GLFWwindow* window, double dx, double dy) noexcept;
		void cursorPosCallback(GLFWwindow* window, double x, double y) noexcept;
		void mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) noexcept;
		void windowSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept;
		void framebufferSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept;

		static void staticJoystickCallback(int jid, int action) noexcept;
		static void staticErrorCallback(int error, const char* description) noexcept;
		static void staticKeyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) noexcept;
		static void staticCharCallback(GLFWwindow* window, uint32_t scancode) noexcept;
		static void staticScrollCallback(GLFWwindow* window, double dx, double dy) noexcept;
		static void staticCursorPosCallback(GLFWwindow* window, double x, double y) noexcept;
		static void staticMouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) noexcept;
		static void staticWindowSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept;
		static void staticFramebufferSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept;
	};

#pragma endregion PlatformImpl definition

#pragma region PlatformCmds

	class BX_NO_VTABLE PlatformCmd
	{
	public:
		enum Type
		{
			CreateWindow,
			DestroyWindow,
			ChangeWindowMode,
			ChangeCursorVisibility,
		};

		PlatformCmd(Type type)
			: _type(type)
		{
		}

		static void process(PlatformCmd& cmd, PlatformImpl& plat);

	private:
		Type  _type;
	};

	class DestroyWindowCmd final : public PlatformCmd
	{
	public:
		DestroyWindowCmd()
			: PlatformCmd(DestroyWindow)
		{
		}

		void process(GLFWwindow* glfw)
		{
			glfwSetWindowShouldClose(glfw, true);
		}
	};

	class ChangeWindowModeCmd final : public PlatformCmd
	{
	public:
		ChangeWindowModeCmd(WindowMode mode)
			: PlatformCmd(ChangeWindowMode)
			, _mode(mode)
		{
		}

		void process(PlatformEventQueue& events, GLFWwindow* glfw)
		{
			switch (_mode)
			{
			case WindowMode::Normal:
			{
				glfwSetWindowMonitor(glfw
					, nullptr
					, 0
					, 0
					, 0
					, 0
					, 0
				);
				events.post<WindowModeChangedEvent>(WindowMode::Normal);
				break;
			}
			case WindowMode::Fullscreen:
			{
				GLFWmonitor* monitor = glfwGetPrimaryMonitor();
				if (monitor == nullptr)
				{
					break;
				}
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);
				glfwSetWindowMonitor(glfw
					, monitor
					, 0
					, 0
					, mode->width
					, mode->height
					, mode->refreshRate
				);
				events.post<WindowModeChangedEvent>(WindowMode::Fullscreen);
				break;
			}
			case WindowMode::WindowedFullscreen:
			{
				GLFWmonitor* monitor = glfwGetPrimaryMonitor();
				if (monitor == nullptr)
				{
					break;
				}
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);
				glfwWindowHint(GLFW_RED_BITS, mode->redBits);
				glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
				glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
				glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

				events.post<WindowModeChangedEvent>(WindowMode::WindowedFullscreen);
				break;
			}
			}
		}
	private:
		WindowMode _mode;
	};

	class ChangeCursorVisibilityCmd final : public PlatformCmd
	{
	public:

		ChangeCursorVisibilityCmd(
			bool value
		) :
			PlatformCmd(ChangeCursorVisibility)
			, _value(value)
		{
		}

		void process(PlatformEventQueue& events, GLFWwindow* glfw)
		{
			auto cursor = _value ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
			glfwSetInputMode(glfw, GLFW_CURSOR, cursor);
		}
	private:
		bool _value;
	};

	void PlatformCmd::process(PlatformCmd& cmd, PlatformImpl& plat)
	{
		// explicit cast to avoid virtual method for performance
		switch (cmd._type)
		{
		case PlatformCmd::DestroyWindow:
			static_cast<DestroyWindowCmd&>(cmd).process(plat.getGlfwWindow());
			break;
		case PlatformCmd::ChangeCursorVisibility:
			static_cast<ChangeCursorVisibilityCmd&>(cmd).process(plat.getEvents(), plat.getGlfwWindow());
			break;
		case PlatformCmd::ChangeWindowMode:
			static_cast<ChangeWindowModeCmd&>(cmd).process(plat.getEvents(), plat.getGlfwWindow());
			break;
		}
	}

#pragma endregion PlatformCmds

	PlatformImpl::PlatformImpl() noexcept
		: _window(nullptr)
		, _windowSize(0)
		, _framebufferSize(0)
	{
	}

	GLFWwindow* PlatformImpl::getGlfwWindow() const noexcept
	{
		return _window;
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
		GLFWwindow* window = glfwCreateWindow(size.x
			, size.y
			, title
			, nullptr
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

	static constexpr std::array<KeyboardKey, GLFW_KEY_LAST + 1> createTranslateKeys()
	{
		std::array<KeyboardKey, GLFW_KEY_LAST + 1> v;
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

	static const std::array<KeyboardKey, GLFW_KEY_LAST + 1>  _glfwKeys = createTranslateKeys();

	KeyboardKey PlatformImpl::translateKey(int key) noexcept
	{
		if (key < 0 || key >= _glfwKeys.size())
		{
			return KeyboardKey::Count;
		}
		return _glfwKeys[key];
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

	GamepadAxis PlatformImpl::translateGamepadAxis(int axis) noexcept
	{
		// HACK: Map XInput 360 controller until GLFW gamepad API
		static std::array<GamepadAxis, 6> axes =
		{
			GamepadAxis::LeftX,
			GamepadAxis::LeftY,
			GamepadAxis::RightX,
			GamepadAxis::RightY,
			GamepadAxis::LeftZ,
			GamepadAxis::RightZ,
		};
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

	void PlatformImpl::errorCallback(int error, const char* description) noexcept
	{
		DBG("GLFW error %d: %s", error, description);
	}

	void PlatformImpl::updateGamepad(uint8_t num) noexcept
	{
		int numButtons, numAxes;
		const unsigned char* buttons = glfwGetJoystickButtons(num, &numButtons);
		const float* axes = glfwGetJoystickAxes(num, &numAxes);

		if (nullptr == buttons || nullptr == axes)
		{
			return;
		}

		for (int i = 0; i < numAxes; ++i)
		{
			auto axis = translateGamepadAxis(i);
			int32_t value = (int32_t)(axes[i] * 32768.f);
			if (GamepadAxis::LeftY == axis || GamepadAxis::RightY == axis)
			{
				value = -value;
			}
			_events.post<GamepadAxisChangedEvent>(num, axis, value);
		}

		for (int i = 0; i < numButtons; ++i)
		{
			auto button = translateGamepadButton(i);
			bool down = buttons[i];
			_events.post<GamepadButtonChangedEvent>(num, button, down);
		}
	}

	void PlatformImpl::updateGamepads() noexcept
	{
		for(uint8_t num = 0; num < Gamepad::MaxAmount; num++)
		{
			updateGamepad(num);
		}
	}

	int PlatformImpl::run(int argc, const char* const* argv) noexcept
	{
		_mte.argc = argc;
		_mte.argv = argv;

		glfwSetErrorCallback(staticErrorCallback);

		if (!glfwInit())
		{
			DBG("glfwInit failed!");
			return bx::kExitFailure;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		_windowSize = glm::uvec2{ DARMOK_DEFAULT_WIDTH, DARMOK_DEFAULT_HEIGHT };
		_window = createWindow(_windowSize, "darmok");

		if (!_window)
		{
			DBG("glfwCreateWindow failed!");
			glfwTerminate();
			return bx::kExitFailure;
		}

		_events.post<WindowSizeChangedEvent>(_windowSize);
		int w, h;
		glfwGetFramebufferSize(_window, &w, &h);
		_framebufferSize = glm::uvec2(w, h);
		_events.post<WindowSizeChangedEvent>(_framebufferSize, true);
		_events.post<WindowPhaseChangedEvent>(WindowPhase::Running);

		_thread.init(MainThreadEntry::threadFunc, &_mte);

		while (!glfwWindowShouldClose(_window) && !_mte.finished)
		{
			glfwWaitEventsTimeout(0.016);

			updateGamepads();

			while (!_cmds.empty())
			{
				auto cmd = std::move(_cmds.front());
				_cmds.pop();
				PlatformCmd::process(*cmd, *this);
			}
		}

		_events.post<WindowPhaseChangedEvent>(WindowPhase::Destroyed);
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
		_events.post<KeyboardKeyChangedEvent>(key2, mods2, down);
	}

	void PlatformImpl::charCallback(GLFWwindow* window, uint32_t scancode) noexcept
	{
		BX_UNUSED(window);
		auto data = Utf8Char::encode(scancode);
		if (!data.len)
		{
			return;
		}
		_events.post<KeyboardCharInputEvent>(data);
	}

	glm::vec2 PlatformImpl::normalizeScreenPoint(double x, double y) noexcept
	{
		y = _windowSize.y - y;
		auto f = glm::vec2(_framebufferSize) / glm::vec2(_windowSize);
		return (glm::vec2(x, y) * f) + glm::vec2(0.5F);
	}

	void PlatformImpl::scrollCallback(GLFWwindow* window, double dx, double dy) noexcept
	{
		_events.post<MouseScrolledEvent>(glm::vec2{ dx, dy });
	}

	void PlatformImpl::cursorPosCallback(GLFWwindow* window, double x, double y) noexcept
	{
		_events.post<MouseMovedEvent>(normalizeScreenPoint(x, y));
	}

	void PlatformImpl::mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) noexcept
	{
		BX_UNUSED(window, mods);
		bool down = action == GLFW_PRESS;
		_events.post<MouseButtonChangedEvent>(
			translateMouseButton(button)
			, down
		);
	}

	void PlatformImpl::windowSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept
	{
		_windowSize = glm::uvec2(width, height);
		_events.post<WindowSizeChangedEvent>(_windowSize);
	}

	void PlatformImpl::framebufferSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept
	{
		_framebufferSize = glm::uvec2(width, height);
		_events.post<WindowSizeChangedEvent>(_framebufferSize, true);
	}

	void PlatformImpl::joystickCallback(int jid, int action) noexcept
	{
		if (action == GLFW_CONNECTED)
		{
			_events.post<GamepadConnectionEvent>(jid, true);
		}
		else if (action == GLFW_DISCONNECTED)
		{
			_events.post<GamepadConnectionEvent>(jid, false);
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
		_cmds.push(std::move(cmd));
	}

	std::unique_ptr<PlatformEvent> PlatformImpl::pollEvent() noexcept
	{
		return _events.poll();
	}

	int32_t MainThreadEntry::threadFunc(bx::Thread* thread, void* userData)
	{
		BX_UNUSED(thread);

		auto self = (MainThreadEntry*)userData;
		int32_t result = main(self->argc, self->argv);
		self->finished = true;
		return result;
	}

	Platform& Platform::get() noexcept
	{
		static PlatformImpl impl;
		static Platform plat(impl);
		return plat;
	}

	std::unique_ptr<PlatformEvent> Platform::pollEvent() noexcept
	{
		return _impl.pollEvent();
	}

	void Platform::requestWindowDestruction() noexcept
	{
		_impl.pushCmd<DestroyWindowCmd>();
	}

	void Platform::requestWindowModeChange(WindowMode mode) noexcept
	{
		_impl.pushCmd<ChangeWindowModeCmd>(mode);
	}

	void Platform::requestCursorVisibilityChange(bool visible) noexcept
	{
		_impl.pushCmd<ChangeCursorVisibilityCmd>(visible);
	}

	void* Platform::getWindowHandle() const noexcept
	{
		return PlatformImpl::getWindowHandle(_impl.getGlfwWindow());
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

int main(int argc, const char* const* argv) noexcept
{
	return darmok::Platform::get().getImpl().run(argc, argv);
}

#endif // DARMOK_CONFIG_USE_GLFW
