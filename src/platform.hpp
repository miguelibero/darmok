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
		void postKeyboardCharInputEvent(const Utf8Char& data) noexcept;
		void postKeyboardKeyChangedEvent(KeyboardKey key, uint8_t modifiers, bool down) noexcept;

		void postMouseMovedEvent(const WindowHandle& window, const MousePosition& pos) noexcept;
		void postMouseButtonChangedEvent(MouseButton button, bool down) noexcept;

		void postGamepadConnectionEvent(const GamepadHandle& gamepad, bool connected) noexcept;
		void postGamepadAxisChangedEvent(const GamepadHandle& gamepad, GamepadAxis axis, int32_t value) noexcept;
		void postGamepadButtonChangedEvent(const GamepadHandle& gamepad, GamepadButton button, bool down) noexcept;

		void postExitEvent() noexcept;

		void postWindowCreatedEvent(const WindowHandle& window, const WindowCreationOptions& options) noexcept;
		void postWindowSizeChangedEvent(const WindowHandle& window, const WindowSize& size) noexcept;
		void postWindowPositionChangedEvent(const WindowHandle& window, const WindowPosition& pos) noexcept;
		void postWindowTitleChangedEvent(const WindowHandle& window, const std::string& title) noexcept;
		void postFileDroppedEvent(const WindowHandle& window, const std::string& filePath) noexcept;
		void postWindowSuspendedEvent(const WindowHandle& window, WindowSuspendPhase phase) noexcept;
		void postWindowDestroyedEvent(const WindowHandle& window)noexcept;

		std::unique_ptr<PlatformEvent> poll()noexcept;

	private:
		std::queue<std::unique_ptr<PlatformEvent>> _events;
	};

	class PlatformContext
	{
	public:
		WindowHandle pushCreateWindowCmd(const WindowCreationOptions& options) noexcept;
		void pushDestroyWindowCmd(const WindowHandle& handle) noexcept;
		void pushSetWindowPositionCmd(const WindowHandle& handle, const WindowPosition& pos) noexcept;
		void pushSetWindowSizeCmd(const WindowHandle& handle, const WindowSize& size) noexcept;
		void pushSetWindowTitleCmd(const WindowHandle& handle, const std::string& title) noexcept;
		void pushSetWindowFlagsCmd(const WindowHandle& handle, uint32_t flags, bool enabled) noexcept;
		void pushToggleWindowFullscreenCmd(const WindowHandle& handle) noexcept;
		void pushSetMouseLockToWindowCmd(const WindowHandle& handle, bool lock) noexcept;

		[[nodiscard]] void* getNativeWindowHandle(const WindowHandle& handle) const noexcept;
		[[nodiscard]] bgfx::NativeWindowHandleType::Enum getNativeWindowHandleType(const WindowHandle& handle) const noexcept;
		[[nodiscard]] void* getNativeDisplayHandle() const noexcept;

		static PlatformContext& get() noexcept;

		std::unique_ptr<PlatformEvent> pollEvent() noexcept;
	};

	

}