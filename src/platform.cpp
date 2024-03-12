
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

	KeyboardKeyChangedEvent::KeyboardKeyChangedEvent(KeyboardKey key, uint8_t modifiers, bool down) noexcept
		: PlatformEvent(KeyboardKeyChanged)
		, _key(key)
		, _modifiers(modifiers)
		, _down(down)
	{
	}

	void KeyboardKeyChangedEvent::process(Input& input) noexcept
	{
		input.getKeyboard().getImpl().setKey(_key, _modifiers, _down);
	}

	KeyboardCharInputEvent::KeyboardCharInputEvent(const Utf8Char& data) noexcept
		: PlatformEvent(KeyboardCharInput)
		, _data(data)
	{
	}

	void KeyboardCharInputEvent::process(Input& input) noexcept
	{
		input.getKeyboard().getImpl().pushChar(_data);
	}

	MouseMovedEvent::MouseMovedEvent(const MousePosition& pos) noexcept
		: PlatformEvent(MouseMoved)
		, _pos(pos)
	{
	}

	void MouseMovedEvent::process(Input& input) noexcept
	{
		input.getMouse().getImpl().setPosition(_pos);
	}

	MouseButtonChangedEvent::MouseButtonChangedEvent(MouseButton button, bool down) noexcept
		: PlatformEvent(MouseButtonChanged)
		, _button(button)
		, _down(down)
	{
	}

	void MouseButtonChangedEvent::process(Input& input) noexcept
	{
		input.getMouse().getImpl().setButton(_button, _down);
	}

	WindowMouseLockChangedEvent::WindowMouseLockChangedEvent(bool locked) noexcept
		: PlatformEvent(WindowMouseLockChanged)
		, _locked(locked)
	{
	}

	void WindowMouseLockChangedEvent::process(Input& input) noexcept
	{
		input.getMouse().getImpl().setLocked(_locked);
	}

	GamepadAxisChangedEvent::GamepadAxisChangedEvent(GamepadHandle gampad, GamepadAxis axis, int32_t value) noexcept
		: PlatformEvent(GamepadAxisChanged)
		, _gamepad(gampad)
		, _axis(axis)
		, _value(value)
	{
	}

	void GamepadAxisChangedEvent::process(Input& input) noexcept
	{
		auto gamepad = input.getGamepad(_gamepad);
		if (gamepad.hasValue())
		{
			gamepad->getImpl().setAxis(_axis, _value);
		}
	}

	GamepadButtonChangedEvent::GamepadButtonChangedEvent(GamepadHandle gampad, GamepadButton button, bool down) noexcept
		: PlatformEvent(GamepadButtonChanged)
		, _gamepad(gampad)
		, _button(button)
		, _down(down)
	{
	}

	void GamepadButtonChangedEvent::process(Input& input) noexcept
	{
		auto gamepad = input.getGamepad(_gamepad);
		if (gamepad.hasValue())
		{
			gamepad->getImpl().setButton(_button, _down);
		}
	}

	GamepadConnectionEvent::GamepadConnectionEvent(GamepadHandle gamepad, bool connected) noexcept
		: PlatformEvent(GamepadConnection)
		, _gamepad(gamepad)
		, _connected(connected)
	{
	}

	void GamepadConnectionEvent::process(Input& input) noexcept
	{
		DBG("gamepad %d, %d", _gamepad.idx, _connected);
		auto gamepad = input.getGamepad(_gamepad);
		if (gamepad.hasValue())
		{
			if (_connected)
			{
				gamepad->getImpl().init(_gamepad);
			}
			else
			{
				gamepad->getImpl().reset();
			}
		}
	}

	WindowSizeChangedEvent::WindowSizeChangedEvent(const WindowSize& size) noexcept
		: PlatformEvent(WindowSizeChanged)
		, _size(size)
	{
	}

	void WindowSizeChangedEvent::process(Window& win) noexcept
	{
		win.getImpl().setSize(_size);
		if (win.getPhase() == WindowPhase::Running)
		{
			bgfx::reset(_size.x, _size.y);
		}
	}

	WindowPhaseChangedEvent::WindowPhaseChangedEvent(WindowPhase phase) noexcept
		: PlatformEvent(WindowPhaseChanged)
		, _phase(phase)
	{
	}

	void WindowPhaseChangedEvent::process(Window& win) noexcept
	{
		win.getImpl().setPhase(_phase);
	}

	WindowModeChangedEvent::WindowModeChangedEvent(WindowMode mode) noexcept
		: PlatformEvent(WindowModeChanged)
		, _mode(mode)
	{
	}

	void WindowModeChangedEvent::process(Window& win) noexcept
	{
		win.getImpl().setMode(_mode);
	}


	void PlatformEvent::process(PlatformEvent& ev, Input& input, Window& window) noexcept
	{
		switch (ev._type)
		{
		case PlatformEvent::KeyboardCharInput:
			static_cast<KeyboardCharInputEvent&>(ev).process(input);
			break;
		case PlatformEvent::GamepadAxisChanged:
			static_cast<GamepadAxisChangedEvent&>(ev).process(input);
			break;
		case PlatformEvent::GamepadConnection:
			static_cast<GamepadConnectionEvent&>(ev).process(input);
			break;
		case PlatformEvent::KeyboardKeyChanged:
			static_cast<KeyboardKeyChangedEvent&>(ev).process(input);
			break;
		case PlatformEvent::MouseMoved:
			static_cast<MouseMovedEvent&>(ev).process(input);
			break;
		case PlatformEvent::MouseButtonChanged:
			static_cast<MouseButtonChangedEvent&>(ev).process(input);
			break;
		case PlatformEvent::WindowMouseLockChanged:
			static_cast<WindowMouseLockChangedEvent&>(ev).process(input);
			break;
		case PlatformEvent::WindowSizeChanged:
			static_cast<WindowSizeChangedEvent&>(ev).process(window);
			break;
		case PlatformEvent::WindowPhaseChanged:
			static_cast<WindowPhaseChangedEvent&>(ev).process(window);
			break;
		case PlatformEvent::WindowModeChanged:
			static_cast<WindowModeChangedEvent&>(ev).process(window);
			break;
		default:
			break;
		}
	}

	void PlatformEventQueue::post(std::unique_ptr<PlatformEvent>&& ev) noexcept
	{
		_events.push(std::move(ev));
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

	Platform::Platform(PlatformImpl& impl) noexcept
		: _impl(impl)
	{
	}

	const PlatformImpl& Platform::getImpl() const noexcept
	{
		return _impl;
	}

	PlatformImpl& Platform::getImpl() noexcept
	{
		return _impl;
	}
}
