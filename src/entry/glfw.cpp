#include "entry.hpp"
#include <darmok/utils.hpp>

#if ENTRY_CONFIG_USE_GLFW

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if GLFW_VERSION_MINOR < 2
#	error "GLFW 3.2 or later is required"
#endif // GLFW_VERSION_MINOR < 2

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#	if ENTRY_CONFIG_USE_WAYLAND
#		include <wayland-egl.h>
#		define GLFW_EXPOSE_NATIVE_WAYLAND
#	else
#		define GLFW_EXPOSE_NATIVE_X11
#		define GLFW_EXPOSE_NATIVE_GLX
#	endif
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
#include <string>

#include "dbg.h"

namespace darmok {
	namespace glfw
	{
		static void* glfwNativeWindowHandle(GLFWwindow* window)
		{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
# 		if ENTRY_CONFIG_USE_WAYLAND
			wl_egl_window* win_impl = (wl_egl_window*)glfwGetWindowUserPointer(window);
			if (!win_impl)
			{
				int width, height;
				glfwGetWindowSize(window, &width, &height);
				struct wl_surface* surface = (struct wl_surface*)glfwGetWaylandWindow(window);
				if (!surface)
					return nullptr;
				win_impl = wl_egl_window_create(surface, width, height);
				glfwSetWindowUserPointer(window, (void*)(uintptr_t)win_impl);
			}
			return (void*)(uintptr_t)win_impl;
#		else
			return (void*)(uintptr_t)glfwGetX11Window(window);
#		endif
#	elif BX_PLATFORM_OSX
			return glfwGetCocoaWindow(window);
#	elif BX_PLATFORM_WINDOWS
			return glfwGetWin32Window(window);
#	endif // BX_PLATFORM_
		}

		static void glfwDestroyWindowImpl(GLFWwindow* window)
		{
			if (!window)
				return;
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#		if ENTRY_CONFIG_USE_WAYLAND
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

		static uint8_t translateKeyModifiers(int glfw)
		{
			uint8_t modifiers = 0;

			if (glfw & GLFW_MOD_ALT)
			{
				modifiers |= to_underlying(Modifier::LeftAlt);
			}

			if (glfw & GLFW_MOD_CONTROL)
			{
				modifiers |= to_underlying(Modifier::LeftCtrl);
			}

			if (glfw & GLFW_MOD_SUPER)
			{
				modifiers |= to_underlying(Modifier::LeftMeta);
			}

			if (glfw & GLFW_MOD_SHIFT)
			{
				modifiers |= to_underlying(Modifier::LeftShift);
			}

			return modifiers;
		}

		static std::array<Key, GLFW_KEY_LAST + 1>  s_translateKey;

		static Key translateKey(int key)
		{
			return s_translateKey[key];
		}

		static MouseButton translateMouseButton(int button)
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT)
			{
				return MouseButton::Left;
			}
			else if (button == GLFW_MOUSE_BUTTON_RIGHT)
			{
				return MouseButton::Right;
			}

			return MouseButton::Middle;
		}

		static GamepadAxis translateGamepadAxis(int axis)
		{
			// HACK: Map XInput 360 controller until GLFW gamepad API

			static GamepadAxis axes[] =
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

		static Key translateGamepadButton(int button)
		{
			// HACK: Map XInput 360 controller until GLFW gamepad API

			static Key buttons[] =
			{
				Key::GamepadA,
				Key::GamepadB,
				Key::GamepadX,
				Key::GamepadY,
				Key::GamepadShoulderL,
				Key::GamepadShoulderR,
				Key::GamepadBack,
				Key::GamepadStart,
				Key::GamepadThumbL,
				Key::GamepadThumbR,
				Key::GamepadUp,
				Key::GamepadRight,
				Key::GamepadDown,
				Key::GamepadLeft,
				Key::GamepadGuide,
			};
			return buttons[button];
		}

		class Gamepad final
		{
		public:
			Gamepad()
				: _connected(false)
			{
				bx::memSet(_axes, 0, sizeof(_axes));
				bx::memSet(_buttons, 0, sizeof(_buttons));
			}

			void update(EventQueue& eventQueue)
			{
				int numButtons, numAxes;
				const unsigned char* buttons = glfwGetJoystickButtons(_handle.idx, &numButtons);
				const float* axes = glfwGetJoystickAxes(_handle.idx, &numAxes);

				if (NULL == buttons || NULL == axes)
				{
					return;
				}
				auto axisCount = to_underlying(GamepadAxis::Count);
				if (numAxes > axisCount)
				{
					numAxes = axisCount;
				}
				auto maxButtons = to_underlying(Key::Count) - to_underlying(Key::GamepadA);
				if (numButtons > maxButtons)
				{
					numButtons = maxButtons;
				}

				WindowHandle defaultWindow = { 0 };

				for (int ii = 0; ii < numAxes; ++ii)
				{
					GamepadAxis axis = translateGamepadAxis(ii);
					int32_t value = (int32_t)(axes[ii] * 32768.f);
					if (GamepadAxis::LeftY == axis || GamepadAxis::RightY == axis)
					{
						value = -value;
					}

					if (_axes[ii] != value)
					{
						_axes[ii] = value;
						eventQueue.postAxisEvent(defaultWindow
							, _handle
							, axis
							, value);
					}
				}

				for (int ii = 0; ii < numButtons; ++ii)
				{
					Key key = translateGamepadButton(ii);
					if (_buttons[ii] != buttons[ii])
					{
						_buttons[ii] = buttons[ii];
						eventQueue.postKeyEvent(defaultWindow
							, key
							, 0
							, buttons[ii] != 0);
					}
				}
			}

			bool isConnected() const
			{
				return _connected;
			}

			void init(uint32_t i, WindowHandle window, EventQueue& eventQueue)
			{
				_handle.idx = i;
				if (glfwJoystickPresent(i))
				{
					_connected = true;
					eventQueue.postGamepadEvent(window
						, _handle
						, true);
				}
			}

			void updateConnection(int action, EventQueue& eventQueue)
			{
				WindowHandle defaultWindow = { 0 };

				if (action == GLFW_CONNECTED)
				{
					_connected = true;
					eventQueue.postGamepadEvent(defaultWindow, _handle, true);
				}
				else if (action == GLFW_DISCONNECTED)
				{
					_connected = false;
					eventQueue.postGamepadEvent(defaultWindow, _handle, false);
				}
			}


		private:
			bool _connected;
			GamepadHandle _handle;
			int32_t _axes[to_underlying(GamepadAxis::Count)];
			uint8_t _buttons[to_underlying(Key::Count) - to_underlying(Key::GamepadA)];
		};

		struct MainThreadEntry
		{
			int argc;
			const char* const* argv;

			static int32_t threadFunc(bx::Thread* thread, void* userData);
		};


		class WindowMap final
		{

		public:
			WindowHandle findHandle(GLFWwindow* window)
			{
				bx::MutexScope scope(_lock);
				for (uint32_t ii = 0, num = _windowAlloc.getNumHandles(); ii < num; ++ii)
				{
					uint16_t idx = _windowAlloc.getHandleAt(ii);
					if (window == _windows[idx])
					{
						WindowHandle handle = { idx };
						return handle;
					}
				}

				WindowHandle invalid = { UINT16_MAX };
				return invalid;
			}

			WindowHandle createHandle()
			{
				return WindowHandle(_windowAlloc.alloc());
			}

			GLFWwindow* getWindow(WindowHandle handle) const
			{
				return _windows[handle.idx];
			}

			void setWindow(WindowHandle handle, GLFWwindow* win)
			{
				_windows[handle.idx] = win;
			}

		private:
			bx::Mutex _lock;
			std::array<GLFWwindow*, ENTRY_CONFIG_MAX_WINDOWS> _windows;
			bx::HandleAllocT<ENTRY_CONFIG_MAX_WINDOWS> _windowAlloc;
		};


		class Cmd
		{
		public:
			enum Type
			{
				CreateWindow,
				DestroyWindow,
				SetWindowTitle,
				SetWindowPosition,
				SetWindowSize,
				ToggleWindowFrame,
				ToggleWindowFullScreen,
				LockMouseToWindow,
			};


			Cmd(Type type)
				: _type(type)
			{
			}

			static void process(Cmd& cmd, EventQueue& eventQueue, WindowMap& windows);

		private:
			Type  _type;
		};

		class WindowCmd : public Cmd
		{
		public:
			WindowCmd(Type type, WindowHandle handle)
				: Cmd(type)
				, _handle(handle)
			{
			}
		protected:

		protected:
			WindowHandle _handle;

		};

		static void keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
		static void charCallback(GLFWwindow* window, uint32_t scancode);
		static void scrollCallback(GLFWwindow* window, double dx, double dy);
		static void cursorPosCallback(GLFWwindow* window, double mx, double my);
		static void mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods);
		static void windowSizeCallback(GLFWwindow* window, int32_t width, int32_t height);
		static void dropFileCallback(GLFWwindow* window, int32_t count, const char** filePaths);
		static void joystickCallback(int jid, int action);

		class CreateWindowCmd final : public WindowCmd
		{
		public:

			CreateWindowCmd(
				WindowHandle handle
				, int32_t  x
				, int32_t  y
				, uint32_t width
				, uint32_t height
				, uint32_t flags
				, const std::string& title
			) :
				WindowCmd(CreateWindow, handle)
				, _x(x)
				, _y(y)
				, _width(width)
				, _height(height)
				, _flags(flags)
				, _title(title)
			{
			}

			void process(EventQueue& eventQueue, WindowMap& windows)
			{
				GLFWwindow* window = glfwCreateWindow(_width
					, _height
					, _title.c_str()
					, NULL
					, NULL);
				if (!window)
				{
					return;
				}

				glfwSetWindowPos(window, _x, _y);
				if (_flags & ENTRY_WINDOW_FLAG_ASPECT_RATIO)
				{
					glfwSetWindowAspectRatio(window, _width, _height);
				}

				glfwSetKeyCallback(window, keyCallback);
				glfwSetCharCallback(window, charCallback);
				glfwSetScrollCallback(window, scrollCallback);
				glfwSetCursorPosCallback(window, cursorPosCallback);
				glfwSetMouseButtonCallback(window, mouseButtonCallback);
				glfwSetWindowSizeCallback(window, windowSizeCallback);
				glfwSetDropCallback(window, dropFileCallback);


				windows.setWindow(_handle, window);
				eventQueue.postSizeEvent(_handle, _width, _height);
				eventQueue.postWindowEvent(_handle, window);
			}

		private:
			int32_t  _x;
			int32_t  _y;
			uint32_t _width;
			uint32_t _height;
			uint32_t _flags;
			std::string _title;
		};

		class DestroyWindowCmd final : public WindowCmd
		{
		public:

			DestroyWindowCmd(
				WindowHandle handle
			) :
				WindowCmd(DestroyWindow, handle)
			{
			}

			void process(EventQueue& eventQueue, WindowMap& windows)
			{
				if (isValid(_handle))
				{
					auto window = windows.getWindow(_handle);
					eventQueue.postWindowEvent(_handle, window);
					glfwDestroyWindowImpl(window);
					windows.setWindow(_handle, nullptr);
				}
			}

		};

		class SetWindowTitleCmd final : public WindowCmd
		{
		public:

			SetWindowTitleCmd(WindowHandle handle, const std::string& title
			) :
				WindowCmd(SetWindowTitle, handle)
				, _title(title)
			{
			}

			void process(WindowMap& windows)
			{
				auto window = windows.getWindow(_handle);
				glfwSetWindowTitle(window, _title.c_str());
			}

		private:
			std::string _title;
		};

		class SetWindowPositionCmd final : public WindowCmd
		{
		public:

			SetWindowPositionCmd(
				WindowHandle handle
				, int32_t  x
				, int32_t  y
			) :
				WindowCmd(SetWindowPosition, handle)
				, _x(x)
				, _y(y)
			{
			}

			void process(WindowMap& windows)
			{
				auto window = windows.getWindow(_handle);
				glfwSetWindowPos(window, _x, _y);
			}

		private:
			int32_t  _x;
			int32_t  _y;
		};

		class SetWindowSizeCmd final : public WindowCmd
		{
		public:

			SetWindowSizeCmd(
				WindowHandle handle
				, uint32_t width
				, uint32_t height
			) :
				WindowCmd(SetWindowSize, handle)
				, _width(width)
				, _height(height)
			{
			}

			void process(WindowMap& windows)
			{
				auto window = windows.getWindow(_handle);
				glfwSetWindowSize(window, _width, _height);
			}

		private:
			uint32_t _width;
			uint32_t _height;
		};

		class ToggleWindowFullScreenCmd final : public WindowCmd
		{
		public:

			ToggleWindowFullScreenCmd(
				WindowHandle handle
			) :
				WindowCmd(ToggleWindowFullScreen, handle)
			{
			}

			void process(WindowMap& windows)
			{
				auto window = windows.getWindow(_handle);
				if (glfwGetWindowMonitor(window))
				{
					glfwSetWindowMonitor(window
						, NULL
						, _oldX
						, _oldY
						, _oldWidth
						, _oldHeight
						, 0
					);
				}
				else
				{
					GLFWmonitor* monitor = glfwGetPrimaryMonitor();
					if (NULL != monitor)
					{
						glfwGetWindowPos(window, &_oldX, &_oldY);
						glfwGetWindowSize(window, &_oldWidth, &_oldHeight);

						const GLFWvidmode* mode = glfwGetVideoMode(monitor);
						glfwSetWindowMonitor(window
							, monitor
							, 0
							, 0
							, mode->width
							, mode->height
							, mode->refreshRate
						);
					}

				}
			}
		private:
			static int32_t _oldX;
			static int32_t _oldY;
			static int32_t _oldWidth;
			static int32_t _oldHeight;
		};

		class ToggleWindowFrameCmd final : public WindowCmd
		{
		public:

			ToggleWindowFrameCmd(
				WindowHandle handle
			) :
				WindowCmd(ToggleWindowFrame, handle)
			{
			}

			void process(EventQueue& eventQueue)
			{
				// Wait for glfwSetWindowDecorated to exist
			}
		};

		class LockMouseToWindowCmd final : public WindowCmd
		{
		public:

			LockMouseToWindowCmd(
				WindowHandle handle,
				bool value
			) :
				WindowCmd(LockMouseToWindow, handle)
				, _value(value)
			{
			}

			void process(WindowMap& windows)
			{
				auto window = windows.getWindow(_handle);
				if (_value)
				{
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
				else
				{
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
			}
		private:
			bool _value;
		};

		void Cmd::process(Cmd& cmd, EventQueue& eventQueue, WindowMap& windows)
		{
			// explicit cast to avoid virtual method for performance
			switch (cmd._type)
			{
			case Cmd::CreateWindow:
				static_cast<CreateWindowCmd&>(cmd).process(eventQueue, windows);
				break;
			case Cmd::DestroyWindow:
				static_cast<DestroyWindowCmd&>(cmd).process(eventQueue, windows);
				break;
			case Cmd::LockMouseToWindow:
				static_cast<LockMouseToWindowCmd&>(cmd).process(windows);
				break;
			case Cmd::SetWindowPosition:
				static_cast<SetWindowPositionCmd&>(cmd).process(windows);
				break;
			case Cmd::SetWindowSize:
				static_cast<SetWindowSizeCmd&>(cmd).process(windows);
				break;
			case Cmd::SetWindowTitle:
				static_cast<SetWindowTitleCmd&>(cmd).process(windows);
				break;
			case Cmd::ToggleWindowFrame:
				static_cast<ToggleWindowFrameCmd&>(cmd).process(eventQueue);
				break;
			case Cmd::ToggleWindowFullScreen:
				static_cast<ToggleWindowFullScreenCmd&>(cmd).process(windows);
				break;
			}
		}

		static void errorCallback(int error, const char* description)
		{
			DBG("GLFW error %d: %s", error, description);
		}

		// Based on cutef8 by Jeff Bezanson (Public Domain)
		static uint8_t encodeUTF8(std::array<uint8_t, 4>& chars, uint32_t scancode)
		{
			uint8_t length = 0;

			if (scancode < 0x80)
			{
				chars[length++] = (char)scancode;
			}
			else if (scancode < 0x800)
			{
				chars[length++] = (scancode >> 6) | 0xc0;
				chars[length++] = (scancode & 0x3f) | 0x80;
			}
			else if (scancode < 0x10000)
			{
				chars[length++] = (scancode >> 12) | 0xe0;
				chars[length++] = ((scancode >> 6) & 0x3f) | 0x80;
				chars[length++] = (scancode & 0x3f) | 0x80;
			}
			else if (scancode < 0x110000)
			{
				chars[length++] = (scancode >> 18) | 0xf0;
				chars[length++] = ((scancode >> 12) & 0x3f) | 0x80;
				chars[length++] = ((scancode >> 6) & 0x3f) | 0x80;
				chars[length++] = (scancode & 0x3f) | 0x80;
			}

			return length;
		}

		class Context final
		{
		public:
			Context()
				: _scrollPos(0.0f)
			{
				s_translateKey[GLFW_KEY_ESCAPE] = Key::Esc;
				s_translateKey[GLFW_KEY_ENTER] = Key::Return;
				s_translateKey[GLFW_KEY_TAB] = Key::Tab;
				s_translateKey[GLFW_KEY_BACKSPACE] = Key::Backspace;
				s_translateKey[GLFW_KEY_SPACE] = Key::Space;
				s_translateKey[GLFW_KEY_UP] = Key::Up;
				s_translateKey[GLFW_KEY_DOWN] = Key::Down;
				s_translateKey[GLFW_KEY_LEFT] = Key::Left;
				s_translateKey[GLFW_KEY_RIGHT] = Key::Right;
				s_translateKey[GLFW_KEY_PAGE_UP] = Key::PageUp;
				s_translateKey[GLFW_KEY_PAGE_DOWN] = Key::PageDown;
				s_translateKey[GLFW_KEY_HOME] = Key::Home;
				s_translateKey[GLFW_KEY_END] = Key::End;
				s_translateKey[GLFW_KEY_PRINT_SCREEN] = Key::Print;
				s_translateKey[GLFW_KEY_KP_ADD] = Key::Plus;
				s_translateKey[GLFW_KEY_EQUAL] = Key::Plus;
				s_translateKey[GLFW_KEY_KP_SUBTRACT] = Key::Minus;
				s_translateKey[GLFW_KEY_MINUS] = Key::Minus;
				s_translateKey[GLFW_KEY_COMMA] = Key::Comma;
				s_translateKey[GLFW_KEY_PERIOD] = Key::Period;
				s_translateKey[GLFW_KEY_SLASH] = Key::Slash;
				s_translateKey[GLFW_KEY_F1] = Key::F1;
				s_translateKey[GLFW_KEY_F2] = Key::F2;
				s_translateKey[GLFW_KEY_F3] = Key::F3;
				s_translateKey[GLFW_KEY_F4] = Key::F4;
				s_translateKey[GLFW_KEY_F5] = Key::F5;
				s_translateKey[GLFW_KEY_F6] = Key::F6;
				s_translateKey[GLFW_KEY_F7] = Key::F7;
				s_translateKey[GLFW_KEY_F8] = Key::F8;
				s_translateKey[GLFW_KEY_F9] = Key::F9;
				s_translateKey[GLFW_KEY_F10] = Key::F10;
				s_translateKey[GLFW_KEY_F11] = Key::F11;
				s_translateKey[GLFW_KEY_F12] = Key::F12;
				s_translateKey[GLFW_KEY_KP_0] = Key::NumPad0;
				s_translateKey[GLFW_KEY_KP_1] = Key::NumPad1;
				s_translateKey[GLFW_KEY_KP_2] = Key::NumPad2;
				s_translateKey[GLFW_KEY_KP_3] = Key::NumPad3;
				s_translateKey[GLFW_KEY_KP_4] = Key::NumPad4;
				s_translateKey[GLFW_KEY_KP_5] = Key::NumPad5;
				s_translateKey[GLFW_KEY_KP_6] = Key::NumPad6;
				s_translateKey[GLFW_KEY_KP_7] = Key::NumPad7;
				s_translateKey[GLFW_KEY_KP_8] = Key::NumPad8;
				s_translateKey[GLFW_KEY_KP_9] = Key::NumPad9;
				s_translateKey[GLFW_KEY_0] = Key::Key0;
				s_translateKey[GLFW_KEY_1] = Key::Key1;
				s_translateKey[GLFW_KEY_2] = Key::Key2;
				s_translateKey[GLFW_KEY_3] = Key::Key3;
				s_translateKey[GLFW_KEY_4] = Key::Key4;
				s_translateKey[GLFW_KEY_5] = Key::Key5;
				s_translateKey[GLFW_KEY_6] = Key::Key6;
				s_translateKey[GLFW_KEY_7] = Key::Key7;
				s_translateKey[GLFW_KEY_8] = Key::Key8;
				s_translateKey[GLFW_KEY_9] = Key::Key9;
				s_translateKey[GLFW_KEY_A] = Key::KeyA;
				s_translateKey[GLFW_KEY_B] = Key::KeyB;
				s_translateKey[GLFW_KEY_C] = Key::KeyC;
				s_translateKey[GLFW_KEY_D] = Key::KeyD;
				s_translateKey[GLFW_KEY_E] = Key::KeyE;
				s_translateKey[GLFW_KEY_F] = Key::KeyF;
				s_translateKey[GLFW_KEY_G] = Key::KeyG;
				s_translateKey[GLFW_KEY_H] = Key::KeyH;
				s_translateKey[GLFW_KEY_I] = Key::KeyI;
				s_translateKey[GLFW_KEY_J] = Key::KeyJ;
				s_translateKey[GLFW_KEY_K] = Key::KeyK;
				s_translateKey[GLFW_KEY_L] = Key::KeyL;
				s_translateKey[GLFW_KEY_M] = Key::KeyM;
				s_translateKey[GLFW_KEY_N] = Key::KeyN;
				s_translateKey[GLFW_KEY_O] = Key::KeyO;
				s_translateKey[GLFW_KEY_P] = Key::KeyP;
				s_translateKey[GLFW_KEY_Q] = Key::KeyQ;
				s_translateKey[GLFW_KEY_R] = Key::KeyR;
				s_translateKey[GLFW_KEY_S] = Key::KeyS;
				s_translateKey[GLFW_KEY_T] = Key::KeyT;
				s_translateKey[GLFW_KEY_U] = Key::KeyU;
				s_translateKey[GLFW_KEY_V] = Key::KeyV;
				s_translateKey[GLFW_KEY_W] = Key::KeyW;
				s_translateKey[GLFW_KEY_X] = Key::KeyX;
				s_translateKey[GLFW_KEY_Y] = Key::KeyY;
				s_translateKey[GLFW_KEY_Z] = Key::KeyZ;
			}

			int run(int argc, const char* const* argv)
			{
				_mte.argc = argc;
				_mte.argv = argv;

				glfwSetErrorCallback(errorCallback);

				if (!glfwInit())
				{
					DBG("glfwInit failed!");
					return bx::kExitFailure;
				}

				glfwSetJoystickCallback(glfw::joystickCallback);

				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

				WindowHandle handle = _windows.createHandle();
				GLFWwindow* window = glfwCreateWindow(ENTRY_DEFAULT_WIDTH
					, ENTRY_DEFAULT_HEIGHT
					, "darmok"
					, NULL
					, NULL
				);

				if (!window)
				{
					DBG("glfwCreateWindow failed!");
					glfwTerminate();
					return bx::kExitFailure;
				}

				_windows.setWindow(handle, window);
				
				glfwSetKeyCallback(window, glfw::keyCallback);
				glfwSetCharCallback(window, glfw::charCallback);
				glfwSetScrollCallback(window, glfw::scrollCallback);
				glfwSetCursorPosCallback(window, glfw::cursorPosCallback);
				glfwSetMouseButtonCallback(window, glfw::mouseButtonCallback);
				glfwSetWindowSizeCallback(window, glfw::windowSizeCallback);
				glfwSetDropCallback(window, glfw::dropFileCallback);

				_eventQueue.postSizeEvent(handle, ENTRY_DEFAULT_WIDTH, ENTRY_DEFAULT_HEIGHT);

				for (uint32_t ii = 0; ii < _gamepads.size(); ++ii)
				{
					_gamepads[ii].init(ii, handle, _eventQueue);
				}

				_thread.init(MainThreadEntry::threadFunc, &_mte);

				while (!glfwWindowShouldClose(window))
				{
					glfwWaitEventsTimeout(0.016);

					for (auto& gamepad : _gamepads)
					{
						if (gamepad.isConnected())
						{
							gamepad.update(_eventQueue);
						}
					}

					while (!_cmds.empty())
					{
						auto cmd = std::move(_cmds.front());
						_cmds.pop();
						Cmd::process(*cmd, _eventQueue, _windows);
					}
				}

				_eventQueue.postExitEvent();
				_thread.shutdown();

				glfwDestroyWindowImpl(window);
				glfwTerminate();

				return _thread.getExitCode();
			}

			void keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
			{
				BX_UNUSED(scancode);
				if (key == GLFW_KEY_UNKNOWN)
				{
					return;
				}
				WindowHandle handle = _windows.findHandle(window);
				int mods2 = translateKeyModifiers(mods);
				Key key2 = translateKey(key);
				bool down = (action == GLFW_PRESS || action == GLFW_REPEAT);
				_eventQueue.postKeyEvent(handle, key2, mods2, down);
			}

			void charCallback(GLFWwindow* window, uint32_t scancode)
			{
				WindowHandle handle = _windows.findHandle(window);
				std::array<uint8_t, 4> chars;
				uint8_t length = encodeUTF8(chars, scancode);
				if (!length)
				{
					return;
				}

				_eventQueue.postCharEvent(handle, length, std::move(chars));
			}

			void scrollCallback(GLFWwindow* window, double dx, double dy)
			{
				BX_UNUSED(dx);
				WindowHandle handle = _windows.findHandle(window);
				double mx, my;
				glfwGetCursorPos(window, &mx, &my);
				_scrollPos += dy;
				_eventQueue.postMouseEvent(handle
					, (int32_t)mx
					, (int32_t)my
					, (int32_t)_scrollPos
				);
			}

			void cursorPosCallback(GLFWwindow* window, double x, double y)
			{
				WindowHandle handle = _windows.findHandle(window);
				_eventQueue.postMouseEvent(handle
					, (int32_t)x
					, (int32_t)y
					, (int32_t)_scrollPos
				);
			}

			void mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
			{
				BX_UNUSED(mods);
				WindowHandle handle = _windows.findHandle(window);
				bool down = action == GLFW_PRESS;
				double mx, my;
				glfwGetCursorPos(window, &mx, &my);
				_eventQueue.postMouseEvent(handle
					, (int32_t)mx
					, (int32_t)my
					, (int32_t)_scrollPos
					, translateMouseButton(button)
					, down
				);
			}

			void windowSizeCallback(GLFWwindow* window, int32_t width, int32_t height)
			{
				WindowHandle handle = _windows.findHandle(window);
				_eventQueue.postSizeEvent(handle, width, height);
			}

			void dropFileCallback(GLFWwindow* window, int32_t count, const char** filePaths)
			{
				WindowHandle handle = _windows.findHandle(window);
				for (int32_t ii = 0; ii < count; ++ii)
				{
					_eventQueue.postDropFileEvent(handle, filePaths[ii]);
				}
			}

			void joystickCallback(int jid, int action)
			{
				if (jid >= ENTRY_CONFIG_MAX_GAMEPADS)
				{
					return;
				}

				_gamepads[jid].updateConnection(action, _eventQueue);
			}

			std::unique_ptr<Event>&& pollEvent()
			{
				return _eventQueue.poll();
			}

			std::unique_ptr<Event>&& pollEvent(WindowHandle handle)
			{
				return _eventQueue.poll(handle);
			}

			WindowHandle pushCreateWindowCmd(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t flags, const std::string& title)
			{
				auto handle = _windows.createHandle();
				_cmds.push(std::make_unique<CreateWindowCmd>(handle, x, y, width, height, flags, title));
				return handle;
			}

			void pushDestroyWindowCmd(WindowHandle handle)
			{
				_cmds.push(std::make_unique<DestroyWindowCmd>(handle));
			}

			void pushSetWindowPositionCmd(WindowHandle handle, int32_t x, int32_t y)
			{
				_cmds.push(std::make_unique<SetWindowPositionCmd>(handle, x, y));
			}

			void pushSetWindowSizeCmd(WindowHandle handle, uint32_t width, uint32_t height)
			{
				_cmds.push(std::make_unique<SetWindowSizeCmd>(handle, width, height));
			}

			void pushSetWindowTitleCmd(WindowHandle handle, const std::string& title)
			{
				_cmds.push(std::make_unique<SetWindowTitleCmd>(handle, title));
			}

			void pushSetWindowFlagsCmd(WindowHandle handle, uint32_t flags, bool enabled)
			{
				BX_UNUSED(handle, flags, enabled);
			}

			void pushToggleFullscreenCmd(WindowHandle handle)
			{
				_cmds.push(std::make_unique<ToggleWindowFullScreenCmd>(handle));
			}

			void pushSetMouseLockCmd(WindowHandle handle, bool lock)
			{
				_cmds.push(std::make_unique<LockMouseToWindowCmd>(handle, lock));
			}

			void* getNativeWindowHandle(WindowHandle handle)
			{
				return glfwNativeWindowHandle(_windows.getWindow(handle));
			}


		private:

			MainThreadEntry _mte;
			bx::Thread _thread;

			EventQueue _eventQueue;
			WindowMap _windows;

			std::array<Gamepad, ENTRY_CONFIG_MAX_GAMEPADS>  _gamepads;

			typedef std::queue<std::unique_ptr<Cmd>> CmdQueue;
			CmdQueue _cmds;

			double _scrollPos;
		};

		static Context _ctx;

		void keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
		{
			_ctx.keyCallback(window, key, scancode, action, mods);
		}

		void charCallback(GLFWwindow* window, uint32_t scancode)
		{
			_ctx.charCallback(window, scancode);
		}

		void scrollCallback(GLFWwindow* window, double dx, double dy)
		{
			_ctx.scrollCallback(window, dx, dy);
		}

		void cursorPosCallback(GLFWwindow* window, double mx, double my)
		{
			_ctx.cursorPosCallback(window, mx, my);
		}

		void mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
		{
			_ctx.mouseButtonCallback(window, button, action, mods);
		}

		void windowSizeCallback(GLFWwindow* window, int32_t width, int32_t height)
		{
			_ctx.windowSizeCallback(window, width, height);
		}

		void dropFileCallback(GLFWwindow* window, int32_t count, const char** filePaths)
		{
			_ctx.dropFileCallback(window, count, filePaths);
		}

		void joystickCallback(int jid, int action)
		{
			_ctx.joystickCallback(jid, action);
		}

		void* getNativeDisplayHandle()
		{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#		if ENTRY_CONFIG_USE_WAYLAND
			return glfwGetWaylandDisplay();
#		else
			return glfwGetX11Display();
#		endif // ENTRY_CONFIG_USE_WAYLAND
#	else
			return NULL;
#	endif // BX_PLATFORM_*
		}

		bgfx::NativeWindowHandleType::Enum getNativeWindowHandleType(WindowHandle _handle)
		{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#		if ENTRY_CONFIG_USE_WAYLAND
			return bgfx::NativeWindowHandleType::Wayland;
#		else
			return bgfx::NativeWindowHandleType::Default;
#		endif // ENTRY_CONFIG_USE_WAYLAND
#	else
			return bgfx::NativeWindowHandleType::Default;
#	endif // BX_PLATFORM_*
		}

		int32_t MainThreadEntry::threadFunc(bx::Thread* thread, void* userData)
		{
			BX_UNUSED(thread);

			MainThreadEntry* self = (MainThreadEntry*)userData;
			int32_t result = main(self->argc, self->argv);

			// RUN app?

			return result;
		}
	}
}

#endif // ENTRY_CONFIG_USE_GLFW
