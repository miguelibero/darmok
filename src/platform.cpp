
#include "platform.hpp"
#include "input.hpp"
#include "window.hpp"
#include "dbg.h"

namespace darmok
{
	PlatformEvent::PlatformEvent(Type type)
		: _type(type)
	{
	}

	class WindowEvent : public PlatformEvent
	{
	public:
		WindowEvent(Type type, WindowHandle window)
			: PlatformEvent(type)
			, _window(window)
		{
		}

		WindowImpl& getWindowImpl()
		{
			return WindowContext::get().getWindow(_window).getImpl();
		}

		WindowHandle getWindowHandle()
		{
			return _window;
		}
	private:
		WindowHandle _window;
	};

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
			auto& kb = Input::get().getKeyboard().getImpl();
			kb.setKey(_key, _modifiers, _down);
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
			auto& kb = Input::get().getKeyboard().getImpl();
			kb.pushChar(_data);
		}
	private:
		Utf8Char _data;
	};

	class MouseMovedEvent final : public WindowEvent
	{
	public:
		MouseMovedEvent(const WindowHandle& window, const MousePosition& pos)
			: WindowEvent(MouseMoved, window)
			, _pos(pos)
		{
		}

		void process()
		{
			auto& mouse = Input::get().getMouse().getImpl();
			mouse.setPosition(_pos);
			mouse.setWindow(getWindowHandle());
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
			auto& mouse = Input::get().getMouse().getImpl();
			mouse.setButton(_button, _down);
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
			auto& gamepad = Input::get().getGamepad(_gamepad).getImpl();
			gamepad.setAxis(_axis, _value);
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
			auto& gamepad = Input::get().getGamepad(_gamepad).getImpl();
			gamepad.setButton(_button, _down);
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
			auto& gamepad = Input::get().getGamepad(_gamepad).getImpl();
			if (_connected)
			{
				gamepad.init(_gamepad);
			}
			else
			{
				gamepad.reset();
			}
		}
	private:
		GamepadHandle _gamepad;
		bool _connected;
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
			auto& win = getWindowImpl();
			if (win.getSize() == _size)
			{
				return false;
			}
			win.setSize(_size);
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
			getWindowImpl().setPosition(_pos);
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
			getWindowImpl().setTitle(_title);
		}
	private:
		std::string _title;
	};

	class WindowCreatedEvent final : public WindowEvent
	{
	public:
		WindowCreatedEvent(WindowHandle window, const WindowCreationOptions& options)
			: WindowEvent(WindowCreated, window)
			, _options(options)
		{
		}

		void process()
		{
			getWindowImpl().init(getWindowHandle(), _options);
		}
	private:
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
			getWindowImpl().reset();
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
			getWindowImpl().onSuspendPhase(_phase);
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
			getWindowImpl().setDropFilePath(_filePath);
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

	void PlatformEventQueue::postKeyboardKeyChangedEvent(KeyboardKey key, uint8_t modifiers, bool down) noexcept
	{
		_events.push(std::make_unique<KeyboardKeyChangedEvent>(key, modifiers, down));
	}

	void PlatformEventQueue::postKeyboardCharInputEvent(const Utf8Char& data) noexcept
	{
		_events.push(std::make_unique<KeyboardCharInputEvent>(data));
	}

	void PlatformEventQueue::postMouseMovedEvent(const WindowHandle& window, const MousePosition& pos) noexcept
	{
		_events.push(std::make_unique<MouseMovedEvent>(window, pos));
	}

	void PlatformEventQueue::postMouseButtonChangedEvent(MouseButton button, bool down) noexcept
	{
		_events.push(std::make_unique<MouseButtonChangedEvent>(button, down));
	}

	void PlatformEventQueue::postGamepadConnectionEvent(const GamepadHandle& gamepad, bool connected) noexcept
	{
		_events.push(std::make_unique<GamepadConnectionEvent>(gamepad, connected));
	}

	void PlatformEventQueue::postGamepadAxisChangedEvent(const GamepadHandle& gamepad, GamepadAxis axis, int32_t value) noexcept
	{
		_events.push(std::make_unique<GamepadAxisChangedEvent>(gamepad, axis, value));
	}

	void PlatformEventQueue::postGamepadButtonChangedEvent(const GamepadHandle& gamepad, GamepadButton button, bool down) noexcept
	{
		_events.push(std::make_unique<GamepadButtonChangedEvent>(gamepad, button, down));
	}

	void PlatformEventQueue::postExitEvent() noexcept
	{
		_events.push(std::make_unique<PlatformEvent>(PlatformEvent::Exit));
	}

	void PlatformEventQueue::postWindowSizeChangedEvent(const WindowHandle& window, const WindowSize& size) noexcept
	{
		_events.push(std::make_unique<WindowSizeChangedEvent>(window, size));
	}

	void PlatformEventQueue::postWindowPositionChangedEvent(const WindowHandle& window, const WindowPosition& pos) noexcept
	{
		_events.push(std::make_unique<WindowPositionChangedEvent>(window, pos));
	}

	void PlatformEventQueue::postWindowTitleChangedEvent(const WindowHandle& window, const std::string& title) noexcept
	{
		_events.push(std::make_unique<WindowTitleChangedEvent>(window, title));
	}

	void PlatformEventQueue::postWindowCreatedEvent(const WindowHandle& window, const WindowCreationOptions& options) noexcept
	{
		_events.push(std::make_unique<WindowCreatedEvent>(window, options));
	}

	void PlatformEventQueue::postWindowDestroyedEvent(const WindowHandle& window) noexcept
	{
		_events.push(std::make_unique<WindowDestroyedEvent>(window));
	}

	void PlatformEventQueue::postWindowSuspendedEvent(const WindowHandle& window, WindowSuspendPhase phase) noexcept
	{
		_events.push(std::make_unique<WindowSuspendedEvent>(window, phase));
	}

	void PlatformEventQueue::postFileDroppedEvent(const WindowHandle& window, const std::string& filePath) noexcept
	{
		_events.push(std::make_unique<FileDroppedEvent>(window, filePath));
	}

	std::unique_ptr<PlatformEvent> PlatformEventQueue::poll() noexcept
	{
		if (_events.empty())
		{
			return nullptr;
		}
		auto platEv = std::move(_events.front());
		_events.pop();
		return platEv;
	}

	PlatformContext& PlatformContext::get() noexcept
	{
		static PlatformContext s_ctx;
		return s_ctx;
	}
}
