#include "platform.hpp"
#include "input.hpp"
#include "window.hpp"
#include <darmok/utils.hpp>
#include <bx/platform.h>

#if DARMOK_CONFIG_USE_GLFW

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if GLFW_VERSION_MINOR < 2
#	error "GLFW 3.2 or later is required"
#endif // GLFW_VERSION_MINOR < 2

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#	if DARMOK_CONFIG_USE_WAYLAND
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

#include "dbg.h"

namespace darmok {

	namespace glfw {

		static void* getNativeWindowHandle(GLFWwindow* window)
		{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
# 		if DARMOK_CONFIG_USE_WAYLAND
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

		static void destroyWindowImpl(GLFWwindow* window)
		{
			if (!window)
				return;
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

		static GLFWwindow* createWindowImpl(const WindowCreationOptions& options);
		
		void joystickCallback(int jid, int action);

		static uint8_t translateKeyModifiers(int glfw)
		{
			uint8_t modifiers = 0;

			if (glfw & GLFW_MOD_ALT)
			{
				modifiers |= to_underlying(KeyboardModifier::LeftAlt);
			}

			if (glfw & GLFW_MOD_CONTROL)
			{
				modifiers |= to_underlying(KeyboardModifier::LeftCtrl);
			}

			if (glfw & GLFW_MOD_SUPER)
			{
				modifiers |= to_underlying(KeyboardModifier::LeftMeta);
			}

			if (glfw & GLFW_MOD_SHIFT)
			{
				modifiers |= to_underlying(KeyboardModifier::LeftShift);
			}

			return modifiers;
		}

		static std::array<KeyboardKey, GLFW_KEY_LAST + 1>  s_translateKey;

		static KeyboardKey translateKey(int key)
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

		static GamepadButton translateGamepadButton(int button)
		{
			// HACK: Map XInput 360 controller until GLFW gamepad API

			static GamepadButton buttons[] =
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

		struct MainThreadEntry
		{
			int argc;
			const char* const* argv;
			bool finished;

			MainThreadEntry()
				: argc(0)
				, argv(nullptr)
				, finished(false)
			{
			}

			static int32_t threadFunc(bx::Thread* thread, void* userData);
		};


		class WindowMap final
		{

		public:
			WindowHandle findHandle(GLFWwindow* window)
			{
				bx::MutexScope scope(_lock);
				for (uint32_t i = 0, num = _windowAlloc.getNumHandles(); i < num; ++i)
				{
					uint16_t idx = _windowAlloc.getHandleAt(i);
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

			GLFWwindow* getWindow(const WindowHandle& handle) const
			{
				return _windows[handle.idx];
			}

			void setWindow(const WindowHandle& handle, GLFWwindow* win)
			{
				_windows[handle.idx] = win;
			}

		private:
			bx::Mutex _lock;
			std::array<GLFWwindow*, Window::MaxAmount> _windows;
			bx::HandleAllocT<Window::MaxAmount> _windowAlloc;
		};


		class BX_NO_VTABLE Cmd
		{
		public:
			enum Type
			{
				CreateWindow,
				DestroyWindow,
				SetWindowTitle,
				SetWindowFlags,
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

			static void process(Cmd& cmd, PlatformEventQueue& events, WindowMap& windows);

		private:
			Type  _type;
		};

		class BX_NO_VTABLE WindowCmd : public Cmd
		{
		public:
			WindowCmd(Type type, WindowHandle handle)
				: Cmd(type)
				, _handle(handle)
			{
			}
		protected:
			WindowHandle _handle;
		};

		class CreateWindowCmd final : public WindowCmd
		{
		public:

			CreateWindowCmd(
				WindowHandle handle
				, const WindowCreationOptions& options
			) :
				WindowCmd(CreateWindow, handle)
				, _options(options)
			{
			}

			void process(PlatformEventQueue& events, WindowMap& windows)
			{
				GLFWwindow* window = createWindowImpl(_options);
				windows.setWindow(_handle, window);
				events.postWindowCreatedEvent(_handle, _options);
			}

		private:
			WindowCreationOptions _options;
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

			void process(PlatformEventQueue& events, WindowMap& windows)
			{
				auto window = windows.getWindow(_handle);
				events.postWindowDestroyedEvent(_handle);
				destroyWindowImpl(window);
				windows.setWindow(_handle, nullptr);
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

			void process(PlatformEventQueue& events, WindowMap& windows)
			{
				auto window = windows.getWindow(_handle);
				glfwSetWindowTitle(window, _title.c_str());
				events.postWindowTitleChangedEvent(_handle, _title);
			}

		private:
			std::string _title;
		};

		class SetWindowFlagsCmd final : public WindowCmd
		{
		public:

			SetWindowFlagsCmd(WindowHandle handle, uint32_t flags, bool enabled
			) :
				WindowCmd(SetWindowTitle, handle)
				, _flags(flags)
				, _enabled(enabled)
			{
			}

			void process(PlatformEventQueue& events, WindowMap& windows)
			{
				// TODO
			}

		private:
			uint32_t _flags;
			bool _enabled;
		};

		class SetWindowPositionCmd final : public WindowCmd
		{
		public:

			SetWindowPositionCmd(
				WindowHandle handle
				, const WindowPosition& pos
			) :
				WindowCmd(SetWindowPosition, handle)
				, _pos(pos)
			{
			}

			void process(WindowMap& windows)
			{
				auto window = windows.getWindow(_handle);
				glfwSetWindowPos(window, _pos.x, _pos.y);
			}

		private:
			WindowPosition _pos;
		};

		class SetWindowSizeCmd final : public WindowCmd
		{
		public:

			SetWindowSizeCmd(
				WindowHandle handle
				, const WindowSize& size
			) :
				WindowCmd(SetWindowSize, handle)
				, _size(size)
			{
			}

			void process(WindowMap& windows)
			{
				auto window = windows.getWindow(_handle);
				glfwSetWindowSize(window, _size.width, _size.height);
			}

		private:
			WindowSize _size;
		};

		class ToggleWindowFullScreenCmd final : public WindowCmd
		{
		private:
			struct NormalWindowState
			{
				WindowSize size;
				WindowPosition pos;
			};
			typedef std::array<NormalWindowState, Window::MaxAmount> NormalWindowStates;
			static NormalWindowStates _normals;

			NormalWindowState& getNormalWindowState()
			{
				return _normals[_handle.idx];
			}

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
					NormalWindowState& state = getNormalWindowState();
					glfwSetWindowMonitor(window
						, nullptr
						, state.pos.x
						, state.pos.y
						, state.size.width
						, state.size.height
						, 0
					);
				}
				else
				{
					GLFWmonitor* monitor = glfwGetPrimaryMonitor();
					if (monitor != nullptr)
					{
						NormalWindowState& state = getNormalWindowState();
						glfwGetWindowPos(window, &state.pos.x, &state.pos.y);
						int w, h;
						glfwGetWindowSize(window, &w, &h);
						state.size.width = w;
						state.size.height = h;
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
		};

		ToggleWindowFullScreenCmd::NormalWindowStates ToggleWindowFullScreenCmd::_normals;

		class ToggleWindowFrameCmd final : public WindowCmd
		{
		public:

			ToggleWindowFrameCmd(
				WindowHandle handle
			) :
				WindowCmd(ToggleWindowFrame, handle)
			{
			}

			void process(PlatformEventQueue& events)
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
				auto cursor = _value ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
				glfwSetInputMode(window, GLFW_CURSOR, cursor);
				
				Input::get().getMouse().getImpl().setLocked(_value);
			}
		private:
			bool _value;
		};

		void Cmd::process(Cmd& cmd, PlatformEventQueue& events, WindowMap& windows)
		{
			// explicit cast to avoid virtual method for performance
			switch (cmd._type)
			{
			case Cmd::CreateWindow:
				static_cast<CreateWindowCmd&>(cmd).process(events, windows);
				break;
			case Cmd::DestroyWindow:
				static_cast<DestroyWindowCmd&>(cmd).process(events, windows);
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
				static_cast<SetWindowTitleCmd&>(cmd).process(events, windows);
				break;
			case Cmd::ToggleWindowFrame:
				static_cast<ToggleWindowFrameCmd&>(cmd).process(events);
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
		static Utf8Char encodeUTF8(uint32_t scancode)
		{
			Utf8Char utf8 = { 0, 0 };

			if (scancode < 0x80)
			{
				utf8.data = scancode;
				utf8.len = 1;
			}
			else if (scancode < 0x800)
			{
				utf8.data = (scancode >> 6) | 0xc0;
				utf8.data += ((scancode & 0x3f) | 0x80) >> 8;
				utf8.len = 2;
			}
			else if (scancode < 0x10000)
			{
				utf8.data = (scancode >> 12) | 0xe0;
				utf8.data += (((scancode >> 6) & 0x3f) | 0x80) >> 8;
				utf8.data += ((scancode & 0x3f) | 0x80) >> 16;
				utf8.len = 3;
			}
			else if (scancode < 0x110000)
			{
				utf8.data = (scancode >> 18) | 0xf0;
				utf8.data += (((scancode >> 12) & 0x3f) | 0x80) >> 8;
				utf8.data += (((scancode >> 6) & 0x3f) | 0x80) >> 16;
				utf8.data += ((scancode & 0x3f) | 0x80) >> 24;
				utf8.len = 4;
			}

			return utf8;
		}

		class Context final
		{
		public:
			Context()
				: _scrollPos(0.0f)
			{
				s_translateKey[GLFW_KEY_ESCAPE] = KeyboardKey::Esc;
				s_translateKey[GLFW_KEY_ENTER] = KeyboardKey::Return;
				s_translateKey[GLFW_KEY_TAB] = KeyboardKey::Tab;
				s_translateKey[GLFW_KEY_BACKSPACE] = KeyboardKey::Backspace;
				s_translateKey[GLFW_KEY_SPACE] = KeyboardKey::Space;
				s_translateKey[GLFW_KEY_UP] = KeyboardKey::Up;
				s_translateKey[GLFW_KEY_DOWN] = KeyboardKey::Down;
				s_translateKey[GLFW_KEY_LEFT] = KeyboardKey::Left;
				s_translateKey[GLFW_KEY_RIGHT] = KeyboardKey::Right;
				s_translateKey[GLFW_KEY_PAGE_UP] = KeyboardKey::PageUp;
				s_translateKey[GLFW_KEY_PAGE_DOWN] = KeyboardKey::PageDown;
				s_translateKey[GLFW_KEY_HOME] = KeyboardKey::Home;
				s_translateKey[GLFW_KEY_END] = KeyboardKey::End;
				s_translateKey[GLFW_KEY_PRINT_SCREEN] = KeyboardKey::Print;
				s_translateKey[GLFW_KEY_KP_ADD] = KeyboardKey::Plus;
				s_translateKey[GLFW_KEY_EQUAL] = KeyboardKey::Plus;
				s_translateKey[GLFW_KEY_KP_SUBTRACT] = KeyboardKey::Minus;
				s_translateKey[GLFW_KEY_MINUS] = KeyboardKey::Minus;
				s_translateKey[GLFW_KEY_COMMA] = KeyboardKey::Comma;
				s_translateKey[GLFW_KEY_PERIOD] = KeyboardKey::Period;
				s_translateKey[GLFW_KEY_SLASH] = KeyboardKey::Slash;
				s_translateKey[GLFW_KEY_F1] = KeyboardKey::F1;
				s_translateKey[GLFW_KEY_F2] = KeyboardKey::F2;
				s_translateKey[GLFW_KEY_F3] = KeyboardKey::F3;
				s_translateKey[GLFW_KEY_F4] = KeyboardKey::F4;
				s_translateKey[GLFW_KEY_F5] = KeyboardKey::F5;
				s_translateKey[GLFW_KEY_F6] = KeyboardKey::F6;
				s_translateKey[GLFW_KEY_F7] = KeyboardKey::F7;
				s_translateKey[GLFW_KEY_F8] = KeyboardKey::F8;
				s_translateKey[GLFW_KEY_F9] = KeyboardKey::F9;
				s_translateKey[GLFW_KEY_F10] = KeyboardKey::F10;
				s_translateKey[GLFW_KEY_F11] = KeyboardKey::F11;
				s_translateKey[GLFW_KEY_F12] = KeyboardKey::F12;
				s_translateKey[GLFW_KEY_KP_0] = KeyboardKey::NumPad0;
				s_translateKey[GLFW_KEY_KP_1] = KeyboardKey::NumPad1;
				s_translateKey[GLFW_KEY_KP_2] = KeyboardKey::NumPad2;
				s_translateKey[GLFW_KEY_KP_3] = KeyboardKey::NumPad3;
				s_translateKey[GLFW_KEY_KP_4] = KeyboardKey::NumPad4;
				s_translateKey[GLFW_KEY_KP_5] = KeyboardKey::NumPad5;
				s_translateKey[GLFW_KEY_KP_6] = KeyboardKey::NumPad6;
				s_translateKey[GLFW_KEY_KP_7] = KeyboardKey::NumPad7;
				s_translateKey[GLFW_KEY_KP_8] = KeyboardKey::NumPad8;
				s_translateKey[GLFW_KEY_KP_9] = KeyboardKey::NumPad9;
				s_translateKey[GLFW_KEY_0] = KeyboardKey::Key0;
				s_translateKey[GLFW_KEY_1] = KeyboardKey::Key1;
				s_translateKey[GLFW_KEY_2] = KeyboardKey::Key2;
				s_translateKey[GLFW_KEY_3] = KeyboardKey::Key3;
				s_translateKey[GLFW_KEY_4] = KeyboardKey::Key4;
				s_translateKey[GLFW_KEY_5] = KeyboardKey::Key5;
				s_translateKey[GLFW_KEY_6] = KeyboardKey::Key6;
				s_translateKey[GLFW_KEY_7] = KeyboardKey::Key7;
				s_translateKey[GLFW_KEY_8] = KeyboardKey::Key8;
				s_translateKey[GLFW_KEY_9] = KeyboardKey::Key9;
				s_translateKey[GLFW_KEY_A] = KeyboardKey::KeyA;
				s_translateKey[GLFW_KEY_B] = KeyboardKey::KeyB;
				s_translateKey[GLFW_KEY_C] = KeyboardKey::KeyC;
				s_translateKey[GLFW_KEY_D] = KeyboardKey::KeyD;
				s_translateKey[GLFW_KEY_E] = KeyboardKey::KeyE;
				s_translateKey[GLFW_KEY_F] = KeyboardKey::KeyF;
				s_translateKey[GLFW_KEY_G] = KeyboardKey::KeyG;
				s_translateKey[GLFW_KEY_H] = KeyboardKey::KeyH;
				s_translateKey[GLFW_KEY_I] = KeyboardKey::KeyI;
				s_translateKey[GLFW_KEY_J] = KeyboardKey::KeyJ;
				s_translateKey[GLFW_KEY_K] = KeyboardKey::KeyK;
				s_translateKey[GLFW_KEY_L] = KeyboardKey::KeyL;
				s_translateKey[GLFW_KEY_M] = KeyboardKey::KeyM;
				s_translateKey[GLFW_KEY_N] = KeyboardKey::KeyN;
				s_translateKey[GLFW_KEY_O] = KeyboardKey::KeyO;
				s_translateKey[GLFW_KEY_P] = KeyboardKey::KeyP;
				s_translateKey[GLFW_KEY_Q] = KeyboardKey::KeyQ;
				s_translateKey[GLFW_KEY_R] = KeyboardKey::KeyR;
				s_translateKey[GLFW_KEY_S] = KeyboardKey::KeyS;
				s_translateKey[GLFW_KEY_T] = KeyboardKey::KeyT;
				s_translateKey[GLFW_KEY_U] = KeyboardKey::KeyU;
				s_translateKey[GLFW_KEY_V] = KeyboardKey::KeyV;
				s_translateKey[GLFW_KEY_W] = KeyboardKey::KeyW;
				s_translateKey[GLFW_KEY_X] = KeyboardKey::KeyX;
				s_translateKey[GLFW_KEY_Y] = KeyboardKey::KeyY;
				s_translateKey[GLFW_KEY_Z] = KeyboardKey::KeyZ;
			}

			void updateGamepad(const GamepadHandle& handle)
			{
				int numButtons, numAxes;
				const unsigned char* buttons = glfwGetJoystickButtons(handle.idx, &numButtons);
				const float* axes = glfwGetJoystickAxes(handle.idx, &numAxes);

				if (nullptr == buttons || nullptr == axes)
				{
					return;
				}
				auto& gamepad = Input::get().getGamepad(handle);

				if (numAxes > gamepad.getAxes().size())
				{
					numAxes = (int)gamepad.getAxes().size();
				}
				if (numButtons > gamepad.getButtons().size())
				{
					numButtons = (int)gamepad.getButtons().size();
				}

				for (int i = 0; i < numAxes; ++i)
				{
					GamepadAxis axis = translateGamepadAxis(i);
					int32_t value = (int32_t)(axes[i] * 32768.f);
					if (GamepadAxis::LeftY == axis || GamepadAxis::RightY == axis)
					{
						value = -value;
					}

					if (gamepad.getAxis(axis) != value)
					{
						_eventQueue.postGamepadAxisChangedEvent(handle
							, axis
							, value);
					}
				}

				for (int i = 0; i < numButtons; ++i)
				{
					GamepadButton button = translateGamepadButton(i);
					bool down = buttons[i];
					if (gamepad.getButton(button) != down)
					{
						_eventQueue.postGamepadButtonChangedEvent(handle, button, down);
					}
				}
			}

			void updateGamepads()
			{
				for(GamepadHandle::idx_t i = 0; i < Gamepad::MaxAmount; i++)
				{
					updateGamepad({ i });
				}
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

				WindowCreationOptions options
				{
					WindowSize{ DARMOK_DEFAULT_WIDTH, DARMOK_DEFAULT_HEIGHT },
					"darmok",
				};

				GLFWwindow* window = createWindowImpl(options);

				if (!window)
				{
					DBG("glfwCreateWindow failed!");
					glfwTerminate();
					return bx::kExitFailure;
				}

				_windows.setWindow(handle, window);

				WindowPosition pos;
				glfwGetWindowPos(window, &pos.x, &pos.y);
				options.pos = pos;

				auto& win = darmok::Context::get().getWindow(handle).getImpl();
				win.init(handle, options);

				_thread.init(MainThreadEntry::threadFunc, &_mte);

				while (!glfwWindowShouldClose(window) && !_mte.finished)
				{
					glfwWaitEventsTimeout(0.016);

					updateGamepads();

					while (!_cmds.empty())
					{
						auto cmd = std::move(_cmds.front());
						_cmds.pop();
						Cmd::process(*cmd, _eventQueue, _windows);
					}
				}

				_eventQueue.postExitEvent();
				_thread.shutdown();

				destroyWindowImpl(window);
				glfwTerminate();

				return _thread.getExitCode();
			}

			void keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
			{
				BX_UNUSED(window, scancode);
				if (key == GLFW_KEY_UNKNOWN)
				{
					return;
				}
				int mods2 = translateKeyModifiers(mods);
				KeyboardKey key2 = translateKey(key);
				bool down = (action == GLFW_PRESS || action == GLFW_REPEAT);
				_eventQueue.postKeyboardKeyChangedEvent(key2, mods2, down);
			}

			void charCallback(GLFWwindow* window, uint32_t scancode)
			{
				BX_UNUSED(window);
				Utf8Char data = encodeUTF8(scancode);
				if (!data.len)
				{
					return;
				}
				_eventQueue.postKeyboardCharInputEvent(data);
			}

			void scrollCallback(GLFWwindow* window, double dx, double dy)
			{
				BX_UNUSED(window, dx);
				double mx, my;
				glfwGetCursorPos(window, &mx, &my);
				_scrollPos += dy;
				_eventQueue.postMouseMovedEvent(
					MousePosition((int32_t)mx
					, (int32_t)my
					, (int32_t)_scrollPos));
			}

			void cursorPosCallback(GLFWwindow* window, double x, double y)
			{
				BX_UNUSED(window);
				_eventQueue.postMouseMovedEvent(
					MousePosition((int32_t)x
					, (int32_t)y
					, (int32_t)_scrollPos)
				);
			}

			void mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
			{
				BX_UNUSED(window, mods);
				bool down = action == GLFW_PRESS;
				_eventQueue.postMouseButtonChangedEvent(
					translateMouseButton(button)
					, down
				);
			}

			void windowSizeCallback(GLFWwindow* window, int32_t width, int32_t height)
			{
				WindowHandle handle = _windows.findHandle(window);
				_eventQueue.postWindowSizeChangedEvent(handle, WindowSize(width, height));
			}

			void windowPosCallback(GLFWwindow* window, int32_t x, int32_t y)
			{
				WindowHandle handle = _windows.findHandle(window);
				_eventQueue.postWindowPositionChangedEvent(handle, WindowPosition(x, y));
			}

			void dropFileCallback(GLFWwindow* window, int32_t count, const char** filePaths)
			{
				WindowHandle handle = _windows.findHandle(window);
				for (int32_t i = 0; i < count; ++i)
				{
					_eventQueue.postFileDroppedEvent(handle, filePaths[i]);
				}
			}

			void joystickCallback(int jid, int action)
			{
				GamepadHandle handle{ (uint16_t)jid };

				if (action == GLFW_CONNECTED)
				{
					_eventQueue.postGamepadConnectionEvent(handle, true);
				}
				else if (action == GLFW_DISCONNECTED)
				{
					_eventQueue.postGamepadConnectionEvent(handle, false);
				}
			}

		private:

			MainThreadEntry _mte;
			bx::Thread _thread;

			PlatformEventQueue _eventQueue;
			WindowMap _windows;

			typedef std::queue<std::unique_ptr<Cmd>> CmdQueue;
			CmdQueue _cmds;

			double _scrollPos;

			friend PlatformContext;
		};

		int32_t MainThreadEntry::threadFunc(bx::Thread* thread, void* userData)
		{
			BX_UNUSED(thread);

			MainThreadEntry* self = (MainThreadEntry*)userData;
			int32_t result = main(self->argc, self->argv);

			self->finished = true;

			return result;
		}

		static Context s_ctx;
		
		void keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
		{
			s_ctx.keyCallback(window, key, scancode, action, mods);
		}

		void charCallback(GLFWwindow* window, uint32_t scancode)
		{
			s_ctx.charCallback(window, scancode);
		}

		void scrollCallback(GLFWwindow* window, double dx, double dy)
		{
			s_ctx.scrollCallback(window, dx, dy);
		}

		void cursorPosCallback(GLFWwindow* window, double mx, double my)
		{
			s_ctx.cursorPosCallback(window, mx, my);
		}

		void mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
		{
			s_ctx.mouseButtonCallback(window, button, action, mods);
		}

		void windowSizeCallback(GLFWwindow* window, int32_t width, int32_t height)
		{
			s_ctx.windowSizeCallback(window, width, height);
		}

		void windowPosCallback(GLFWwindow* window, int32_t xpos, int32_t ypos)
		{
			s_ctx.windowPosCallback(window, xpos, ypos);
		}

		void dropFileCallback(GLFWwindow* window, int32_t count, const char** filePaths)
		{
			s_ctx.dropFileCallback(window, count, filePaths);
		}

		void joystickCallback(int jid, int action)
		{
			s_ctx.joystickCallback(jid, action);
		}

		GLFWwindow* createWindowImpl(const WindowCreationOptions& options)
		{
			GLFWwindow* window = glfwCreateWindow(options.size.width
				, options.size.height
				, options.title.c_str()
				, NULL
				, NULL);
			if (!window)
			{
				return window;
			}

			if (options.pos.has_value())
			{
				auto pos = options.pos.value();
				glfwSetWindowPos(window, pos.x, pos.y);
			}
			if (options.flags & WindowFlags::AspectRatio)
			{
				glfwSetWindowAspectRatio(window, options.size.width, options.size.height);
			}

			glfwSetKeyCallback(window, keyCallback);
			glfwSetCharCallback(window, charCallback);
			glfwSetScrollCallback(window, scrollCallback);
			glfwSetCursorPosCallback(window, cursorPosCallback);
			glfwSetMouseButtonCallback(window, mouseButtonCallback);
			glfwSetWindowSizeCallback(window, windowSizeCallback);
			glfwSetDropCallback(window, dropFileCallback);

			return window;
		}
	}

	WindowHandle PlatformContext::pushCreateWindowCmd(const WindowCreationOptions& options)
	{
		auto handle = glfw::s_ctx._windows.createHandle();
		glfw::s_ctx._cmds.push(std::make_unique<glfw::CreateWindowCmd>(handle, options));
		return handle;
	}

	void PlatformContext::pushDestroyWindowCmd(const WindowHandle& handle)
	{
		glfw::s_ctx._cmds.push(std::make_unique<glfw::DestroyWindowCmd>(handle));
	}

	void PlatformContext::pushSetWindowPositionCmd(const WindowHandle& handle, const WindowPosition& pos)
	{
		glfw::s_ctx._cmds.push(std::make_unique<glfw::SetWindowPositionCmd>(handle, pos));
	}

	void PlatformContext::pushSetWindowSizeCmd(const WindowHandle& handle, const WindowSize& size)
	{
		glfw::s_ctx._cmds.push(std::make_unique<glfw::SetWindowSizeCmd>(handle, size));
	}

	void PlatformContext::pushSetWindowTitleCmd(const WindowHandle& handle, const std::string& title)
	{
		glfw::s_ctx._cmds.push(std::make_unique<glfw::SetWindowTitleCmd>(handle, title));
	}

	void PlatformContext::pushSetWindowFlagsCmd(const WindowHandle& handle, uint32_t flags, bool enabled)
	{
		glfw::s_ctx._cmds.push(std::make_unique<glfw::SetWindowFlagsCmd>(handle, flags, enabled));
	}

	void PlatformContext::pushToggleWindowFullscreenCmd(const WindowHandle& handle)
	{
		glfw::s_ctx._cmds.push(std::make_unique<glfw::ToggleWindowFullScreenCmd>(handle));
	}

	void PlatformContext::pushSetMouseLockToWindowCmd(const WindowHandle& handle, bool lock)
	{
		glfw::s_ctx._cmds.push(std::make_unique<glfw::LockMouseToWindowCmd>(handle, lock));
	}

	void* PlatformContext::getNativeWindowHandle(const WindowHandle& handle) const
	{
		return glfw::getNativeWindowHandle(glfw::s_ctx._windows.getWindow(handle));
	}

	void* PlatformContext::getNativeDisplayHandle() const
	{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#		if DARMOK_CONFIG_USE_WAYLAND
		return glfwGetWaylandDisplay();
#		else
		return glfwGetX11Display();
#		endif // DARMOK_CONFIG_USE_WAYLAND
#	else
		return NULL;
#	endif // BX_PLATFORM_*
	}

	bgfx::NativeWindowHandleType::Enum PlatformContext::getNativeWindowHandleType(const WindowHandle& window) const
	{
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#		if DARMOK_CONFIG_USE_WAYLAND
		return bgfx::NativeWindowHandleType::Wayland;
#		else
		return bgfx::NativeWindowHandleType::Default;
#		endif // DARMOK_CONFIG_USE_WAYLAND
#	else
		return bgfx::NativeWindowHandleType::Default;
#	endif // BX_PLATFORM_*
	}

	std::unique_ptr<PlatformEvent> PlatformContext::pollEvent()
	{
		return glfw::s_ctx._eventQueue.poll();
	}
}

int main(int argc, const char* const* argv)
{
	return darmok::glfw::s_ctx.run(argc, argv);
}

#endif // DARMOK_CONFIG_USE_GLFW
