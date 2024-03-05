#pragma once

#include <darmok/window.hpp>
#include <darmok/input.hpp>
#include <bx/bx.h>
#include <queue>
#include <memory>

#ifndef DARMOK_PLATFORM_NOOP
#	define DARMOK_PLATFORM_NOOP 0
#endif // DARMOK_PLATFORM_NOOP

#ifndef DARMOK_PLATFORM_GLFW
#	define DARMOK_PLATFORM_GLFW 0
#endif // DARMOK_PLATFORM_GLFW

#ifndef DARMOK_PLATFORM_SUPPORT_WAYLAND
#	define DARMOK_PLATFORM_SUPPORT_WAYLAND 1
#endif // DARMOK_PLATFORM_SUPPORT_WAYLAND

#if !defined(DARMOK_PLATFORM_NATIVE) \
	&& !DARMOK_PLATFORM_NOOP \
	&& !DARMOK_PLATFORM_GLFW
#	define DARMOK_PLATFORM_NATIVE 1
#else
#	define DARMOK_PLATFORM_NATIVE 0
#endif // ...

#if !defined(DARMOK_DEFAULT_WIDTH) && !defined(DARMOK_DEFAULT_HEIGHT)
#	define DARMOK_DEFAULT_WIDTH  1280
#	define DARMOK_DEFAULT_HEIGHT 720
#elif !defined(DARMOK_DEFAULT_WIDTH) || !defined(DARMOK_DEFAULT_HEIGHT)
#	error "Both DARMOK_DEFAULT_WIDTH and DARMOK_DEFAULT_HEIGHT must be defined."
#endif // DARMOK_DEFAULT_WIDTH

namespace darmok
{
	int main(int argc, const char* const* argv);

	class BX_NO_VTABLE PlatformEvent
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

		PlatformEvent(Type type);

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

		void postMouseMovedEvent(const WindowHandle& window, const MousePosition& pos);
		void postMouseButtonChangedEvent(MouseButton button, bool down);

		void postGamepadConnectionEvent(const GamepadHandle& gamepad, bool connected);
		void postGamepadAxisChangedEvent(const GamepadHandle& gamepad, GamepadAxis axis, int32_t value);
		void postGamepadButtonChangedEvent(const GamepadHandle& gamepad, GamepadButton button, bool down);

		void postExitEvent();

		void postWindowCreatedEvent(const WindowHandle& window, const WindowCreationOptions& options);
		void postWindowSizeChangedEvent(const WindowHandle& window, const WindowSize& size);
		void postWindowPositionChangedEvent(const WindowHandle& window, const WindowPosition& pos);
		void postWindowTitleChangedEvent(const WindowHandle& window, const std::string& title);
		void postFileDroppedEvent(const WindowHandle& window, const std::string& filePath);
		void postWindowSuspendedEvent(const WindowHandle& window, WindowSuspendPhase phase);
		void postWindowDestroyedEvent(const WindowHandle& window);

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