
#include "detail/platform.hpp"
#include "detail/input.hpp"
#include "detail/window.hpp"

namespace darmok
{
	PlatformEvent::PlatformEvent(Type type)
		: _type{ type }
	{
	}

	KeyboardKeyEvent::KeyboardKeyEvent(KeyboardKey key, KeyboardModifiers modifiers, bool down) noexcept
		: PlatformEvent(Type::KeyboardKey)
		, _key{ key }
		, _modifiers{ std::move(modifiers) }
		, _down{ down }
	{
	}

	expected<void, std::string> KeyboardKeyEvent::process(Input& input) noexcept
	{
		auto& keyb = input.getKeyboard().getImpl();
		return keyb.setKey(_key, _modifiers, _down);
	}

	KeyboardCharEvent::KeyboardCharEvent(char32_t chr) noexcept
		: PlatformEvent(Type::KeyboardChar)
		, _chr{ chr }
	{
	}

	expected<void, std::string> KeyboardCharEvent::process(Input& input) noexcept
	{
		return input.getKeyboard().getImpl().pushChar(_chr);
	}

	MousePositionEvent::MousePositionEvent(const glm::vec2& pos) noexcept
		: PlatformEvent(Type::MousePosition)
		, _pos{ pos }
	{
	}

	expected<void, std::string> MousePositionEvent::process(Input& input) noexcept
	{
		return input.getMouse().getImpl().setPosition(_pos);
	}

	MouseActiveEvent::MouseActiveEvent(bool active) noexcept
		: PlatformEvent(Type::MouseActive)
		, _active{ active }
	{
	}

	expected<void, std::string> MouseActiveEvent::process(Input& input) noexcept
	{
		return input.getMouse().getImpl().setActive(_active);
	}

	MouseScrollEvent::MouseScrollEvent(const glm::vec2& scroll) noexcept
		: PlatformEvent(Type::MouseScroll)
		, _scroll{ scroll }
	{
	}

	expected<void, std::string> MouseScrollEvent::process(Input& input) noexcept
	{
		return input.getMouse().getImpl().setScroll(_scroll);
	}

	MouseButtonEvent::MouseButtonEvent(MouseButton button, bool down) noexcept
		: PlatformEvent(Type::MouseButton)
		, _button{ button }
		, _down{ down }
	{
	}

	expected<void, std::string> MouseButtonEvent::process(Input& input) noexcept
	{
		return input.getMouse().getImpl().setButton(_button, _down);
	}

	GamepadStickEvent::GamepadStickEvent(uint8_t gampad, GamepadStick stick, const glm::vec3& value) noexcept
		: PlatformEvent(Type::GamepadStick)
		, _gamepad{ gampad }
		, _stick{ stick }
		, _value{ value }
	{
	}

	expected<void, std::string> GamepadStickEvent::process(Input& input) noexcept
	{
		auto gamepad = input.getGamepad(_gamepad);
		if (!gamepad)
		{
			return unexpected<std::string>{ "gamepad not found" };
		}
		return gamepad->getImpl().setStick(_stick, _value);
	}

	GamepadButtonEvent::GamepadButtonEvent(uint8_t gampad, GamepadButton button, bool down) noexcept
		: PlatformEvent(Type::GamepadButton)
		, _gamepad{ gampad }
		, _button{ button }
		, _down{ down }
	{
	}

	expected<void, std::string> GamepadButtonEvent::process(Input& input) noexcept
	{
		auto gamepad = input.getGamepad(_gamepad);
		if (!gamepad)
		{
			return unexpected<std::string>{ "gamepad not found" };
		}
		return gamepad->getImpl().setButton(_button, _down);
	}

	GamepadConnectEvent::GamepadConnectEvent(uint8_t gamepad, bool connected) noexcept
		: PlatformEvent(Type::GamepadConnect)
		, _gamepad{ gamepad }
		, _connected{ connected }
	{
	}

	expected<void, std::string> GamepadConnectEvent::process(Input& input) noexcept
	{
		auto gamepad = input.getGamepad(_gamepad);
		if (!gamepad)
		{
			return unexpected<std::string>{ "gamepad not found" };
		}
		return gamepad->getImpl().setConnected(_connected);
	}

	WindowSizeEvent::WindowSizeEvent(const glm::uvec2& size) noexcept
		: PlatformEvent(Type::WindowSize)
		, _size{ size }
	{
	}

	expected<void, std::string> WindowSizeEvent::process(Window& win) noexcept
	{
		return win.getImpl().setSize(_size);
	}

	WindowPixelSizeEvent::WindowPixelSizeEvent(const glm::uvec2& size) noexcept
		: PlatformEvent(Type::WindowPixelSize)
		, _size{ size }
	{
	}

	expected<void, std::string> WindowPixelSizeEvent::process(Window& win) noexcept
	{
		return win.getImpl().setPixelSize(_size);
	}
	
	WindowPhaseEvent::WindowPhaseEvent(WindowPhase phase) noexcept
		: PlatformEvent(Type::WindowPhase)
		, _phase{ phase }
	{
	}

	expected<void, std::string> WindowPhaseEvent::process(Window& win) noexcept
	{
		return win.getImpl().setPhase(_phase);
	}

	WindowErrorEvent::WindowErrorEvent(std::string err) noexcept
		: PlatformEvent(Type::WindowError)
		, _error{ std::move(err) }
	{
	}

	expected<void, std::string> WindowErrorEvent::process(Window& win) noexcept
	{
		return win.getImpl().onError(_error);
	}

	WindowVideoModeEvent::WindowVideoModeEvent(VideoMode mode) noexcept
		: PlatformEvent(Type::WindowVideoMode)
		, _mode{ std::move(mode) }
	{
	}

	expected<void, std::string> WindowVideoModeEvent::process(Window& win) noexcept
	{
		return win.getImpl().setVideoMode(_mode);
	}

	VideoModeInfoEvent::VideoModeInfoEvent(VideoModeInfo info) noexcept
		: PlatformEvent(Type::VideoModeInfo)
		, _info{ std::move(info) }
	{
	}

	expected<void, std::string> VideoModeInfoEvent::process(Window& win) noexcept
	{
		return win.getImpl().setVideoModeInfo(_info);
	}

	WindowCursorModeEvent::WindowCursorModeEvent(WindowCursorMode mode) noexcept
		: PlatformEvent(Type::WindowCursorMode)
		, _mode{ mode }
	{
	}

	expected<void, std::string> WindowCursorModeEvent::process(Window& win) noexcept
	{
		return win.getImpl().setCursorMode(_mode);
	}

	WindowTitleEvent::WindowTitleEvent(std::string title) noexcept
		: PlatformEvent(Type::WindowCursorMode)
		, _title{ std::move(title) }
	{
	}

	expected<void, std::string> WindowTitleEvent::process(Window& win) noexcept
	{
		return win.getImpl().setTitle(_title);
	}

	FileDialogEvent::FileDialogEvent(FileDialogResult result, FileDialogCallback callback) noexcept
		: PlatformEvent(Type::FileDialog)
		, _result{ std::move(result) }
		, _callback{ std::move(callback) }
	{
	}

	expected<void, std::string> FileDialogEvent::process(Window& win) noexcept
	{
		if (_callback)
		{
			return _callback(_result);
		}
		return {};
	}

	expected<void, std::string> PlatformEvent::process(PlatformEvent& ev, Input& input, Window& window) noexcept
	{
		switch (ev._type)
		{
		case PlatformEvent::Type::KeyboardKey:
			return static_cast<KeyboardKeyEvent&>(ev).process(input);
		case PlatformEvent::Type::KeyboardChar:
			return static_cast<KeyboardCharEvent&>(ev).process(input);
		case PlatformEvent::Type::GamepadConnect:
			return static_cast<GamepadConnectEvent&>(ev).process(input);
		case PlatformEvent::Type::GamepadStick:
			return static_cast<GamepadStickEvent&>(ev).process(input);
		case PlatformEvent::Type::GamepadButton:
			return static_cast<GamepadButtonEvent&>(ev).process(input);
		case PlatformEvent::Type::MousePosition:
			return static_cast<MousePositionEvent&>(ev).process(input);
		case PlatformEvent::Type::MouseActive:
			return static_cast<MouseActiveEvent&>(ev).process(input);
		case PlatformEvent::Type::MouseScroll:
			return static_cast<MouseScrollEvent&>(ev).process(input);
		case PlatformEvent::Type::MouseButton:
			return static_cast<MouseButtonEvent&>(ev).process(input);
		case PlatformEvent::Type::WindowSize:
			return static_cast<WindowSizeEvent&>(ev).process(window);
		case PlatformEvent::Type::WindowPixelSize:
			return static_cast<WindowPixelSizeEvent&>(ev).process(window);
		case PlatformEvent::Type::WindowPhase:
			return static_cast<WindowPhaseEvent&>(ev).process(window);
		case PlatformEvent::Type::WindowVideoMode:
			return static_cast<WindowVideoModeEvent&>(ev).process(window);
		case PlatformEvent::Type::WindowCursorMode:
			return static_cast<WindowCursorModeEvent&>(ev).process(window);
		case PlatformEvent::Type::WindowTitle:
			return static_cast<WindowTitleEvent&>(ev).process(window);
		case PlatformEvent::Type::WindowError:
			return static_cast<WindowErrorEvent&>(ev).process(window);
		case PlatformEvent::Type::VideoModeInfo:
			return static_cast<VideoModeInfoEvent&>(ev).process(window);
		case PlatformEvent::Type::FileDialog:
			return static_cast<FileDialogEvent&>(ev).process(window);
		default:
			return unexpected<std::string>("unknown platform event type");
		}
	}

	void PlatformEventQueue::post(std::unique_ptr<PlatformEvent>&& ev) noexcept
	{
		const std::lock_guard lock(_mutex);
		_events.push(std::move(ev));
	}

	std::unique_ptr<PlatformEvent> PlatformEventQueue::poll() noexcept
	{
		const std::lock_guard lock(_mutex);
		if (_events.empty())
		{
			return nullptr;
		}
		std::unique_ptr<PlatformEvent> platEv = std::move(_events.front());
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
