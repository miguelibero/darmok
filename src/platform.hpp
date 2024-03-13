#pragma once

#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/utils.hpp>
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

#pragma region PlatformEvents

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
			WindowMouseLockChanged,

			WindowSizeChanged,
			WindowPhaseChanged,
			WindowModeChanged,

			Count,
		};

		PlatformEvent(Type type);
		virtual ~PlatformEvent() = default;

		static void process(PlatformEvent& ev, Input& input, Window& win) noexcept;

	private:
		Type _type;
	};

	class KeyboardKeyChangedEvent final : public PlatformEvent
	{
	public:
		KeyboardKeyChangedEvent(KeyboardKey key, uint8_t modifiers, bool down) noexcept;
		void process(Input& input) noexcept;
	private:
		KeyboardKey _key;
		uint8_t _modifiers;
		bool _down;
	};

	class KeyboardCharInputEvent final : public PlatformEvent
	{
	public:
		KeyboardCharInputEvent(const Utf8Char& data) noexcept;
		void process(Input& input) noexcept;
	private:
		Utf8Char _data;
	};

	class MouseMovedEvent final : public PlatformEvent
	{
	public:
		MouseMovedEvent(const MousePosition& pos) noexcept;
		void process(Input& input) noexcept;
	private:
		MousePosition _pos;
	};

	class MouseButtonChangedEvent final : public PlatformEvent
	{
	public:
		MouseButtonChangedEvent(MouseButton button, bool down) noexcept;
		void process(Input& input) noexcept;
	private:
		MouseButton _button;
		bool _down;
	};

	class WindowMouseLockChangedEvent final : public PlatformEvent
	{
	public:
		WindowMouseLockChangedEvent(bool locked) noexcept;
		void process(Input& input) noexcept;
	private:
		bool _locked;
	};

	class GamepadAxisChangedEvent final : public PlatformEvent
	{
	public:
		GamepadAxisChangedEvent(uint8_t gampad, GamepadAxis axis, int32_t value) noexcept;
		void process(Input& input) noexcept;

	private:
		uint8_t _gamepad;
		GamepadAxis _axis;
		int32_t _value;
	};

	class GamepadButtonChangedEvent final : public PlatformEvent
	{
	public:
		GamepadButtonChangedEvent(uint8_t gampad, GamepadButton button, bool down) noexcept;
		void process(Input& input) noexcept;

	private:
		uint8_t _gamepad;
		GamepadButton _button;
		bool _down;
	};

	class GamepadConnectionEvent final : public PlatformEvent
	{
	public:
		GamepadConnectionEvent(uint8_t gamepad, bool connected) noexcept;
		void process(Input& input) noexcept;
	private:
		uint8_t _gamepad;
		bool _connected;
	};

	class WindowSizeChangedEvent final : public PlatformEvent
	{
	public:
		WindowSizeChangedEvent(const WindowSize& size) noexcept;
		void process(Window& win) noexcept;
	private:
		WindowSize _size;
	};

	class WindowPhaseChangedEvent final : public PlatformEvent
	{
	public:
		WindowPhaseChangedEvent(WindowPhase phase) noexcept;
		void process(Window& win) noexcept;
	private:
		WindowPhase _phase;
	};

	class WindowModeChangedEvent final : public PlatformEvent
	{
	public:
		WindowModeChangedEvent(WindowMode mode) noexcept;
		void process(Window& win) noexcept;
	private:
		WindowMode _mode;
	};

	class PlatformEventQueue final
	{
	public:
		void post(std::unique_ptr<PlatformEvent>&& ev) noexcept;

		template<typename T, typename... A>
		T& post(A&&... args) noexcept
		{
			auto ptr = new T(std::forward<A>(args)...);
			post(std::unique_ptr<PlatformEvent>(ptr));
			return *ptr;
		}

		std::unique_ptr<PlatformEvent> poll() noexcept;
	private:
		std::queue<std::unique_ptr<PlatformEvent>> _events;
	};

#pragma endregion PlatformEvents

	class PlatformImpl;

	class Platform final
	{
	public:
		static Platform& get() noexcept;

		Platform(const Platform& other) = delete;
		Platform(Platform&& other) = delete;

		const PlatformImpl& getImpl() const noexcept;
		PlatformImpl& getImpl() noexcept;

		void requestWindowDestruction() noexcept;
		void requestWindowModeChange(WindowMode mode) noexcept;
		void requestWindowMouseLock(bool enabled) noexcept;

		[[nodiscard]] void* getWindowHandle() const noexcept;
		[[nodiscard]] bgfx::NativeWindowHandleType::Enum getWindowHandleType() const noexcept;
		[[nodiscard]] void* getDisplayHandle() const noexcept;

		[[nodiscard]] std::unique_ptr<PlatformEvent> pollEvent() noexcept;
	private:
		Platform(PlatformImpl& impl) noexcept;
		PlatformImpl& _impl;
	};
}