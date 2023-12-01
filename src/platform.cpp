
#include "platform.hpp"
#include "dbg.h"
#include "input.hpp"

#include <bx/bx.h>
#include <bx/file.h>
#include <bx/readerwriter.h>
#include <bgfx/bgfx.h>

namespace darmok
{
	class KeyboardKeyChangedEvent final : public PlatformEvent
	{
	public:
		KeyboardKeyChangedEvent(KeyboardKey key, uint8_t modifiers, bool down)
			: PlatformEvent(KeyboardKeyChanged)
			, _key(key)
			, _modifiers(modifiers)
			, _down(down)
		{
		}

		void process()
		{
			KeyboardImpl::get().setKey(_key, _modifiers, _down);
		}
	private:
		KeyboardKey _key;
		uint8_t _modifiers;
		bool _down;
	};

	class KeyboardCharInputEvent final : public PlatformEvent
	{
	public:
		KeyboardCharInputEvent(const Utf8Char& data)
			: PlatformEvent(KeyboardCharInput)
			, _data(data)
		{
		}

		void process()
		{
			KeyboardImpl::get().pushChar(_data);
		}
	private:
		Utf8Char _data;
	};

	class MouseMovedEvent final : public PlatformEvent
	{
	public:
		MouseMovedEvent(const MousePosition& pos)
			: PlatformEvent(MouseMoved)
			, _pos(pos)
		{
		}

		void process()
		{
			MouseImpl::get().setPosition(_pos);
		}
	private:
		MousePosition _pos;
	};

	class MouseButtonChangedEvent final : public PlatformEvent
	{
	public:
		MouseButtonChangedEvent(MouseButton button, bool down)
			: PlatformEvent(MouseButtonChanged)
			, _button(button)
			, _down(down)
		{
		}

		void process()
		{
			MouseImpl::get().setButton(_button, _down);
		}
	private:
		MouseButton _button;
		bool _down;
	};

	class GamepadAxisChangedEvent final : public PlatformEvent
	{
	public:
		GamepadAxisChangedEvent(GamepadHandle gampad, GamepadAxis axis, int32_t value)
			: PlatformEvent(GamepadAxisChanged)
			, _gamepad(gampad)
			, _axis(axis)
			, _value(value)
		{
		}

		void process()
		{
			GamepadImpl::get(_gamepad).setAxis(_axis, _value);
		}

	private:
		GamepadHandle _gamepad;
		GamepadAxis _axis;
		int32_t _value;
	};

	class GamepadButtonChangedEvent final : public PlatformEvent
	{
	public:
		GamepadButtonChangedEvent(GamepadHandle gampad, GamepadButton button, bool down)
			: PlatformEvent(GamepadButtonChanged)
			, _gamepad(gampad)
			, _button(button)
			, _down(down)
		{
		}

		void process()
		{
			GamepadImpl::get(_gamepad).setButton(_button, _down);
		}

	private:
		GamepadHandle _gamepad;
		GamepadButton _button;
		bool _down;
	};

	class GamepadConnectionEvent final : public PlatformEvent
	{
	public:
		GamepadConnectionEvent(GamepadHandle gamepad, bool connected)
			: PlatformEvent(GamepadConnection)
			, _gamepad(gamepad)
			, _connected(connected)
		{
		}

		void process()
		{
			DBG("gamepad %d, %d", _gamepad.idx, _connected);
			GamepadImpl::get(_gamepad).setConnected(_connected);
		}
	private:
		GamepadHandle _gamepad;
		bool _connected;
	};

	class WindowEvent : public PlatformEvent
	{
	public:
		WindowEvent(Type type, WindowHandle window)
			: PlatformEvent(type)
			, _window(window)
		{
		}

		Window& getWindow()
		{
			return Window::get(_window);
		}

		WindowHandle getWindowHandle()
		{
			return _window;
		}
	private:
		WindowHandle _window;
	};

	class WindowSizeChangedEvent final : public WindowEvent
	{
	public:
		WindowSizeChangedEvent(WindowHandle window, const WindowSize& size)
			: WindowEvent(WindowSizeChanged, window)
			, _size(size)
		{
		}

		bool process()
		{
			auto& win = getWindow();
			if (win._size == _size)
			{
				return false;
			}
			win._size = _size;
			return true;
		}
	private:
		WindowSize _size;
	};

	class WindowPositionChangedEvent final : public WindowEvent
	{
	public:
		WindowPositionChangedEvent(WindowHandle window, const WindowPosition& pos)
			: WindowEvent(WindowSizeChanged, window)
			, _pos(pos)
		{
		}

		void process()
		{
			auto& win = getWindow();
			win._pos = _pos;
		}
	private:
		WindowPosition _pos;
	};

	class WindowTitleChangedEvent final : public WindowEvent
	{
	public:
		WindowTitleChangedEvent(WindowHandle window, const std::string& title)
			: WindowEvent(WindowSizeChanged, window)
			, _title(title)
		{
		}

		void process()
		{
			auto& win = getWindow();
			win._title = _title;
		}
	private:
		std::string _title;
	};

	class WindowCreatedEvent final : public WindowEvent
	{
	public:
		WindowCreatedEvent(WindowHandle window, void* nativeHandle, const WindowCreationOptions& options)
			: WindowEvent(WindowCreated, window)
			, _nativeHandle(nativeHandle)
			, _options(options)
		{
		}

		void process()
		{
			auto& win = getWindow();
			win._handle = getWindowHandle();
			win._size = _options.size;
			win._pos = _options.pos;
			win._title = _options.title;
			win._flags = _options.flags;
		}
	private:
		void* _nativeHandle;
		WindowCreationOptions _options;

	};

	class WindowDestroyedEvent final : public WindowEvent
	{
	public:
		WindowDestroyedEvent(WindowHandle window)
			: WindowEvent(WindowDestroyed, window)
		{
		}

		void process()
		{
			auto& win = Window::get();
			win._handle = Window::InvalidHandle;
			win._size = {};
			win._pos = {};
			win._title = {};
			win._flags = WindowFlags::None;
		}
	};

	class WindowSuspendedEvent final : public WindowEvent
	{
	public:

		WindowSuspendedEvent(WindowHandle window, WindowSuspendPhase phase)
			: WindowEvent(WindowSuspended, window)
			, _phase(phase)
		{
		}

		void process()
		{
		}
	private:
		WindowSuspendPhase _phase;
	};

	class FileDroppedEvent final : public WindowEvent
	{
	public:
		FileDroppedEvent(WindowHandle window, const std::string& filePath)
			: WindowEvent(FileDropped, window)
			, _filePath(filePath)
		{
		}

		void process()
		{
			DBG("drop file %s", _filePath.c_str());
			auto& win = getWindow();
			win._dropFilePath = _filePath;
		}
	private:
		std::string _filePath;
	};


	PlatformEvent::Result PlatformEvent::process(PlatformEvent& ev)
	{
		switch (ev._type)
		{
		case PlatformEvent::Exit:
			return Result::Exit;
		case PlatformEvent::KeyboardCharInput:
			static_cast<KeyboardCharInputEvent&>(ev).process();
			break;
		case PlatformEvent::GamepadAxisChanged:
			static_cast<GamepadAxisChangedEvent&>(ev).process();
			break;
		case PlatformEvent::GamepadConnection:
			static_cast<GamepadConnectionEvent&>(ev).process();
			break;
		case PlatformEvent::KeyboardKeyChanged:
			static_cast<KeyboardKeyChangedEvent&>(ev).process();
			break;
		case PlatformEvent::MouseMoved:
			static_cast<MouseMovedEvent&>(ev).process();
			break;
		case PlatformEvent::MouseButtonChanged:
			static_cast<MouseButtonChangedEvent&>(ev).process();
			break;
		case PlatformEvent::WindowSizeChanged:
		{
			auto reset = static_cast<WindowSizeChangedEvent&>(ev).process();
			if (reset)
			{
				return Result::Reset;
			}
			break;
		}
		case PlatformEvent::WindowCreated:
			static_cast<WindowCreatedEvent&>(ev).process();
			break;
		case PlatformEvent::WindowSuspended:
			static_cast<WindowSuspendedEvent&>(ev).process();
			break;
		case PlatformEvent::FileDropped:
			static_cast<FileDroppedEvent&>(ev).process();
			break;
		default:
			break;
		}
		return {};
	}

	void PlatformEventQueue::postKeyboardKeyChangedEvent(KeyboardKey key, uint8_t modifiers, bool down)
	{
		_events.push(std::make_unique<KeyboardKeyChangedEvent>(key, modifiers, down));
	}

	void PlatformEventQueue::postKeyboardCharInputEvent(const Utf8Char& data)
	{
		_events.push(std::make_unique<KeyboardCharInputEvent>(data));
	}

	void PlatformEventQueue::postMouseMovedEvent(const MousePosition& pos)
	{
		_events.push(std::make_unique<MouseMovedEvent>(pos));
	}

	void PlatformEventQueue::postMouseButtonChangedEvent(MouseButton button, bool down)
	{
		_events.push(std::make_unique<MouseButtonChangedEvent>(button, down));
	}

	void PlatformEventQueue::postGamepadConnectionEvent(GamepadHandle gamepad, bool connected)
	{
		_events.push(std::make_unique<GamepadConnectionEvent>(gamepad, connected));
	}

	void PlatformEventQueue::postGamepadAxisChangedEvent(GamepadHandle gamepad, GamepadAxis axis, int32_t value)
	{
		_events.push(std::make_unique<GamepadAxisChangedEvent>(gamepad, axis, value));
	}

	void PlatformEventQueue::postGamepadButtonChangedEvent(GamepadHandle gamepad, GamepadButton button, bool down)
	{
		_events.push(std::make_unique<GamepadButtonChangedEvent>(gamepad, button, down));
	}

	void PlatformEventQueue::postExitEvent()
	{
		_events.push(std::make_unique<PlatformEvent>(PlatformEvent::Exit));
	}

	void PlatformEventQueue::postWindowSizeChangedEvent(WindowHandle window, const WindowSize& size)
	{
		_events.push(std::make_unique<WindowSizeChangedEvent>(window, size));
	}

	void PlatformEventQueue::postWindowPositionChangedEvent(WindowHandle window, const WindowPosition& pos)
	{
		_events.push(std::make_unique<WindowPositionChangedEvent>(window, pos));
	}

	void PlatformEventQueue::postWindowTitleChangedEvent(WindowHandle window, const std::string& title)
	{
		_events.push(std::make_unique<WindowTitleChangedEvent>(window, title));
	}

	void PlatformEventQueue::postWindowCreatedEvent(WindowHandle window, void* nativeHandle, const WindowCreationOptions& options)
	{
		_events.push(std::make_unique<WindowCreatedEvent>(window, nativeHandle, options));
	}

	void PlatformEventQueue::postWindowDestroyedEvent(WindowHandle window)
	{
		_events.push(std::make_unique<WindowDestroyedEvent>(window));
	}

	void PlatformEventQueue::postWindowSuspendedEvent(WindowHandle window, WindowSuspendPhase phase)
	{
		_events.push(std::make_unique<WindowSuspendedEvent>(window, phase));
	}

	void PlatformEventQueue::postFileDroppedEvent(WindowHandle window, const std::string& filePath)
	{
		_events.push(std::make_unique<FileDroppedEvent>(window, filePath));
	}

	std::unique_ptr<PlatformEvent> PlatformEventQueue::poll()
	{
		if (_events.size() == 0)
		{
			return nullptr;
		}
		auto ev = std::move(_events.front());
		_events.pop();
		return std::move(ev);
	}

	PlatformContext& PlatformContext::get()
	{
		static PlatformContext s_ctx;
		return s_ctx;
	}
}
