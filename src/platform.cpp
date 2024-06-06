
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

	KeyboardKeyEvent::KeyboardKeyEvent(KeyboardKey key, uint8_t modifiers, bool down) noexcept
		: PlatformEvent(Type::KeyboardKey)
		, _key(key)
		, _modifiers(modifiers)
		, _down(down)
	{
	}

	void KeyboardKeyEvent::process(Input& input) noexcept
	{
		input.getKeyboard().getImpl().setKey(_key, _modifiers, _down);
	}

	KeyboardCharEvent::KeyboardCharEvent(const Utf8Char& data) noexcept
		: PlatformEvent(Type::KeyboardChar)
		, _data(data)
	{
	}

	void KeyboardCharEvent::process(Input& input) noexcept
	{
		input.getKeyboard().getImpl().pushChar(_data);
	}

	MousePositionEvent::MousePositionEvent(const glm::vec2& pos) noexcept
		: PlatformEvent(Type::MousePosition)
		, _pos(pos)
	{
	}

	void MousePositionEvent::process(Input& input) noexcept
	{
		input.getMouse().getImpl().setPosition(_pos);
	}

	MouseActiveEvent::MouseActiveEvent(bool active) noexcept
		: PlatformEvent(Type::MouseActive)
		, _active(active)
	{
	}

	void MouseActiveEvent::process(Input& input) noexcept
	{
		input.getMouse().getImpl().setActive(_active);
	}

	MouseScrollEvent::MouseScrollEvent(const glm::vec2& scroll) noexcept
		: PlatformEvent(Type::MouseScroll)
		, _scroll(scroll)
	{
	}

	void MouseScrollEvent::process(Input& input) noexcept
	{
		input.getMouse().getImpl().setScroll(_scroll);
	}

	MouseButtonEvent::MouseButtonEvent(MouseButton button, bool down) noexcept
		: PlatformEvent(Type::MouseButton)
		, _button(button)
		, _down(down)
	{
	}

	void MouseButtonEvent::process(Input& input) noexcept
	{
		input.getMouse().getImpl().setButton(_button, _down);
	}

	GamepadStickEvent::GamepadStickEvent(uint8_t gampad, GamepadStick stick, const glm::vec3& value) noexcept
		: PlatformEvent(Type::GamepadStick)
		, _gamepad(gampad)
		, _stick(stick)
		, _value(value)
	{
	}

	void GamepadStickEvent::process(Input& input) noexcept
	{
		auto gamepad = input.getGamepad(_gamepad);
		if (gamepad)
		{
			gamepad->getImpl().setStick(_stick, _value);
		}
	}

	GamepadButtonEvent::GamepadButtonEvent(uint8_t gampad, GamepadButton button, bool down) noexcept
		: PlatformEvent(Type::GamepadButton)
		, _gamepad(gampad)
		, _button(button)
		, _down(down)
	{
	}

	void GamepadButtonEvent::process(Input& input) noexcept
	{
		auto gamepad = input.getGamepad(_gamepad);
		if (gamepad)
		{
			gamepad->getImpl().setButton(_button, _down);
		}
	}

	GamepadConnectEvent::GamepadConnectEvent(uint8_t gamepad, bool connected) noexcept
		: PlatformEvent(Type::GamepadConnect)
		, _gamepad(gamepad)
		, _connected(connected)
	{
	}

	void GamepadConnectEvent::process(Input& input) noexcept
	{
		auto gamepad = input.getGamepad(_gamepad);
		if (gamepad)
		{
			gamepad->getImpl().setConnected(_connected);
		}
	}

	WindowSizeEvent::WindowSizeEvent(const glm::uvec2& size, bool pixel) noexcept
		: PlatformEvent(Type::WindowSize)
		, _size(size)
		, _pixel(pixel)
	{
	}

	void WindowSizeEvent::process(Window& win) noexcept
	{
		if (_pixel)
		{
			win.getImpl().setPixelSize(_size);
			if (win.getPhase() == WindowPhase::Running)
			{
				bgfx::reset(_size.x, _size.y);
			}
		}
		else
		{
			win.getImpl().setSize(_size);
		}
	}

	WindowPhaseEvent::WindowPhaseEvent(WindowPhase phase) noexcept
		: PlatformEvent(Type::WindowPhase)
		, _phase(phase)
	{
	}

	void WindowPhaseEvent::process(Window& win) noexcept
	{
		win.getImpl().setPhase(_phase);
	}

	WindowModeEvent::WindowModeEvent(WindowMode mode) noexcept
		: PlatformEvent(Type::WindowMode)
		, _mode(mode)
	{
	}

	void WindowModeEvent::process(Window& win) noexcept
	{
		win.getImpl().setMode(_mode);
	}


	void PlatformEvent::process(PlatformEvent& ev, Input& input, Window& window) noexcept
	{
		switch (ev._type)
		{
		case PlatformEvent::Type::KeyboardKey:
			static_cast<KeyboardKeyEvent&>(ev).process(input);
			break;
		case PlatformEvent::Type::KeyboardChar:
			static_cast<KeyboardCharEvent&>(ev).process(input);
			break;
		case PlatformEvent::Type::GamepadConnect:
			static_cast<GamepadConnectEvent&>(ev).process(input);
			break;
		case PlatformEvent::Type::GamepadStick:
			static_cast<GamepadStickEvent&>(ev).process(input);
			break;
		case PlatformEvent::Type::GamepadButton:
			static_cast<GamepadButtonEvent&>(ev).process(input);
			break;
		case PlatformEvent::Type::MousePosition:
			static_cast<MousePositionEvent&>(ev).process(input);
			break;
		case PlatformEvent::Type::MouseActive:
			static_cast<MouseActiveEvent&>(ev).process(input);
			break;
		case PlatformEvent::Type::MouseScroll:
			static_cast<MouseScrollEvent&>(ev).process(input);
			break;
		case PlatformEvent::Type::MouseButton:
			static_cast<MouseButtonEvent&>(ev).process(input);
			break;
		case PlatformEvent::Type::WindowSize:
			static_cast<WindowSizeEvent&>(ev).process(window);
			break;
		case PlatformEvent::Type::WindowPhase:
			static_cast<WindowPhaseEvent&>(ev).process(window);
			break;
		case PlatformEvent::Type::WindowMode:
			static_cast<WindowModeEvent&>(ev).process(window);
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

	Platform& Platform::get() noexcept
	{
		static Platform platform;
		return platform;
	}

	const PlatformImpl& Platform::getImpl() const noexcept
	{
		return *_impl;
	}

	PlatformImpl& Platform::getImpl() noexcept
	{
		return *_impl;
	}
}
