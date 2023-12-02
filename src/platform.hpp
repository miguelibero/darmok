#pragma once

#include <darmok/window.hpp>
#include <darmok/input.hpp>

#include <queue>
#include <memory>

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

	class PlatformEvent
	{
	public:
		enum Type
		{
			KeyboardKeyChanged,
			KeyboardCharInput,

			GamepadConnection,
			GamepadAxisChanged,
			GamepadButtonChanged,

			MouseMoved,
			MouseButtonChanged,

			WindowSizeChanged,
			WindowCreated,
			WindowDestroyed,
			WindowSuspended,
			FileDropped,

			Exit,
			Count,
		};

		PlatformEvent(Type type)
			: _type(type)
		{
		}

		enum class Result
		{
			Nothing,
			Exit,
			Reset,
		};

		static Result process(PlatformEvent& ev);

	private:
		Type _type;
	};

	class PlatformEventQueue final
	{
	public:		
		void postKeyboardCharInputEvent(const Utf8Char& data);
		void postKeyboardKeyChangedEvent(KeyboardKey key, uint8_t modifiers, bool down);

		void postMouseMovedEvent(const MousePosition& pos);
		void postMouseButtonChangedEvent(MouseButton button, bool down);

		void postGamepadConnectionEvent(GamepadHandle gamepad, bool connected);
		void postGamepadAxisChangedEvent(GamepadHandle gamepad, GamepadAxis axis, int32_t value);
		void postGamepadButtonChangedEvent(GamepadHandle gamepad, GamepadButton button, bool down);

		void postExitEvent();

		void postWindowCreatedEvent(WindowHandle window, const WindowCreationOptions& options);
		void postWindowSizeChangedEvent(WindowHandle window, const WindowSize& size);
		void postWindowPositionChangedEvent(WindowHandle window, const WindowPosition& pos);
		void postWindowTitleChangedEvent(WindowHandle window, const std::string& title);
		void postFileDroppedEvent(WindowHandle window, const std::string& filePath);
		void postWindowSuspendedEvent(WindowHandle window, WindowSuspendPhase phase);
		void postWindowDestroyedEvent(WindowHandle window);

		std::unique_ptr<PlatformEvent> poll();

	private:
		std::queue<std::unique_ptr<PlatformEvent>> _events;
	};

	class PlatformContext
	{
	public:
		WindowHandle pushCreateWindowCmd(const WindowCreationOptions& options);
		void pushDestroyWindowCmd(const WindowHandle& handle);
		void pushSetWindowPositionCmd(const WindowHandle& handle, const WindowPosition& pos);
		void pushSetWindowSizeCmd(const WindowHandle& handle, const WindowSize& size);
		void pushSetWindowTitleCmd(const WindowHandle& handle, const std::string& title);
		void pushSetWindowFlagsCmd(const WindowHandle& handle, uint32_t flags, bool enabled);
		void pushToggleWindowFullscreenCmd(const WindowHandle& handle);
		void pushSetMouseLockToWindowCmd(const WindowHandle& handle, bool lock);

		void* getNativeWindowHandle(const WindowHandle& handle) const;
		bgfx::NativeWindowHandleType::Enum getNativeWindowHandleType(const WindowHandle& handle) const;
		void* getNativeDisplayHandle() const;

		static PlatformContext& get();

		std::unique_ptr<PlatformEvent> pollEvent();
	};

	

}