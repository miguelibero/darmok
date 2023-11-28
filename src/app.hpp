#pragma once

#include <darmok/window.hpp>
#include <darmok/app.hpp>
#include <darmok/input.hpp>

#include <queue>

#ifndef DARMOK_CONFIG_USE_NOOP
#	define DARMOK_CONFIG_USE_NOOP 0
#endif // DARMOK_CONFIG_USE_NOOP

#ifndef DARMOK_CONFIG_USE_SDL
#	define DARMOK_CONFIG_USE_SDL 0
#endif // DARMOK_CONFIG_USE_SDL

#ifndef DARMOK_CONFIG_USE_GLFW
#	define DARMOK_CONFIG_USE_GLFW 0
#endif // DARMOK_CONFIG_USE_GLFW

#ifndef DARMOK_CONFIG_USE_WAYLAND
#	define DARMOK_CONFIG_USE_WAYLAND 0
#endif // DARMOKY_CONFIG_USE_WAYLAND

#if !defined(DARMOK_CONFIG_USE_NATIVE) \
	&& !DARMOK_CONFIG_USE_NOOP \
	&& !DARMOK_CONFIG_USE_SDL \
	&& !DARMOK_CONFIG_USE_GLFW
#	define DARMOK_CONFIG_USE_NATIVE 1
#else
#	define DARMOK_CONFIG_USE_NATIVE 0
#endif // ...

#if !defined(DARMOK_DEFAULT_WIDTH) && !defined(DARMOK_DEFAULT_HEIGHT)
#	define DARMOK_DEFAULT_WIDTH  1280
#	define DARMOK_DEFAULT_HEIGHT 720
#elif !defined(DARMOK_DEFAULT_WIDTH) || !defined(DARMOK_DEFAULT_HEIGHT)
#	error "Both DARMOK_DEFAULT_WIDTH and DARMOK_DEFAULT_HEIGHT must be defined."
#endif // DARMOK_DEFAULT_WIDTH

#ifndef ENTRY_CONFIG_IMPLEMENT_DEFAULT_ALLOCATOR
#	define ENTRY_CONFIG_IMPLEMENT_DEFAULT_ALLOCATOR 1
#endif // ENTRY_CONFIG_IMPLEMENT_DEFAULT_ALLOCATOR

#ifndef DARMOK_CONFIG_PROFILER
#	define DARMOK_CONFIG_PROFILER 0
#endif // DARMOK_CONFIG_PROFILER

namespace darmok
{
	int main(int argc, const char* const* argv);

	class Event
	{
	public:
		enum Type
		{
			GamepadAxisChanged,
			CharInput,
			Exit,
			GamepadConnection,
			KeyPressed,
			MouseMoved,
			MouseButtonPressed,
			WindowSizeChanged,
			WindowCreated,
			WindowSuspended,
			FileDropped,
		};

		Event(Type type)
			: _type(type)
		{
		}

		enum class Result
		{
			Nothing,
			Exit,
			Reset,
		};

		static Result process(Event& ev);

	private:
		Type _type;
	};

	class GamepadAxisChangedEvent final : public Event
	{
	public:
		GamepadAxisChangedEvent(GamepadHandle handle, GamepadAxis axis, int32_t value);
		void process();
	private:
		GamepadHandle _gamepad;
		GamepadAxis _axis;
		int32_t _value;
	};

	class CharInputEvent final : public Event
	{
	public:
		CharInputEvent(const Utf8Char& data);
		void process();
	private:
		Utf8Char _data;
	};

	class GamepadConnectionEvent final : public Event
	{
	public:
		GamepadConnectionEvent(GamepadHandle gamepad, bool connected);
		void process();
	private:
		GamepadHandle _gamepad;
		bool _connected;
	};

	class KeyPressedEvent final : public Event
	{
	public:
		KeyPressedEvent(Key key, uint8_t modifiers, bool down);
		void process();
	private:
		Key _key;
		uint8_t _modifiers;
		bool _down;
	};

	class MouseMovedEvent final : public Event
	{
	public:
		MouseMovedEvent(const MousePosition& pos);
		void process();
	private:
		MousePosition _pos;
	};

	class MouseButtonPressedEvent final : public Event
	{
	public:
		MouseButtonPressedEvent(MouseButton button, bool down);
		void process();
	private:
		MouseButton _button;
		bool _down;
	};

	class WindowSizeChangedEvent final : public Event
	{
	public:
		WindowSizeChangedEvent(WindowHandle window, const WindowSize& size);
		bool process();
		WindowHandle getWindowHandle();
	private:
		WindowHandle _window;
		WindowSize _size;
	};

	class WindowPositionChangedEvent final : public Event
	{
	public:
		WindowPositionChangedEvent(WindowHandle window, const WindowPosition& pos);
		void process();
	private:
		WindowHandle _window;
		WindowPosition _pos;
	};

	class WindowCreatedEvent final : public Event
	{
	public:
		WindowCreatedEvent(WindowHandle window, void* nativeHandle, const WindowCreationOptions& options);
		void process();
	private:
		WindowHandle _window;
		void* _nativeHandle;
		WindowCreationOptions _options;

	};

	class WindowDestroyedEvent final : public Event
	{
	public:
		WindowDestroyedEvent(WindowHandle window);
		void process();
	private:
		WindowHandle _window;
	};

	class WindowSuspendedEvent final : public Event
	{
	public:
		WindowSuspendedEvent(WindowHandle window, WindowSuspendPhase phase);
		void process();
	private:
		WindowHandle _window;
		WindowSuspendPhase _phase;
	};

	class FileDroppedEvent final : public Event
	{
	public:
		FileDroppedEvent(WindowHandle window, const std::string& filePath);
		void process();
	private:
		WindowHandle _window;
		std::string _filePath;
	};

	std::unique_ptr<Event> pollEvent();

	class EventQueue final
	{
	public:
		void postGamepadAxisChangedEvent(GamepadHandle gamepad, GamepadAxis axis, int32_t value);
		void postCharInputEvent(const Utf8Char& data);
		void postExitEvent();
		void postGamepadConnectionEvent(GamepadHandle gamepad, bool connected);
		void postKeyPressedEvent(Key key, uint8_t modifiers, bool down);
		void postMouseMovedEvent(const MousePosition& pos);
		void postMouseButtonPressedEvent(MouseButton button, bool down);
		void postWindowSizeChangedEvent(WindowHandle window, const WindowSize& size);
		void postWindowPositionChangedEvent(WindowHandle window, const WindowPosition& pos);
		void postWindowCreatedEvent(WindowHandle window, void* nativeHandle, const WindowCreationOptions& options);
		void postWindowDestroyedEvent(WindowHandle window);
		void postWindowSuspendedEvent(WindowHandle window, WindowSuspendPhase phase);
		void postFileDroppedEvent(WindowHandle window, const std::string& filePath);

		std::unique_ptr<Event> poll();

	private:
		std::queue<std::unique_ptr<Event>> _queue;
	};

}