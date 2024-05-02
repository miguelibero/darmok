#pragma once

#include "input.hpp"
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
		enum class Type
		{
			KeyboardKey,
			KeyboardChar,

			GamepadConnect,
			GamepadStick,
			GamepadButton,

			MousePosition,
			MouseActive,
			MouseScroll,
			MouseButton,

			WindowSize,
			WindowPhase,
			WindowMode,

			Count,
		};

		PlatformEvent(Type type);
		virtual ~PlatformEvent() = default;

		static void process(PlatformEvent& ev, Input& input, Window& win) noexcept;

	private:
		Type _type;
	};

	class KeyboardKeyEvent final : public PlatformEvent
	{
	public:
		KeyboardKeyEvent(KeyboardKey key, uint8_t modifiers, bool down) noexcept;
		void process(Input& input) noexcept;
	private:
		KeyboardKey _key;
		uint8_t _modifiers;
		bool _down;
	};

	class KeyboardCharEvent final : public PlatformEvent
	{
	public:
		KeyboardCharEvent(const Utf8Char& data) noexcept;
		void process(Input& input) noexcept;
	private:
		Utf8Char _data;
	};

	class MousePositionEvent final : public PlatformEvent
	{
	public:
		MousePositionEvent(const glm::vec2& pos) noexcept;
		void process(Input& input) noexcept;
	private:
		glm::vec2 _pos;
	};

	class MouseActiveEvent final : public PlatformEvent
	{
	public:
		MouseActiveEvent(bool active) noexcept;
		void process(Input& input) noexcept;
	private:
		bool _active;
	};

	class MouseScrollEvent final : public PlatformEvent
	{
	public:
		MouseScrollEvent(const glm::vec2& scroll) noexcept;
		void process(Input& input) noexcept;
	private:
		glm::vec2 _scroll;
	};

	class MouseButtonEvent final : public PlatformEvent
	{
	public:
		MouseButtonEvent(MouseButton button, bool down) noexcept;
		void process(Input& input) noexcept;
	private:
		MouseButton _button;
		bool _down;
	};

	class GamepadStickEvent final : public PlatformEvent
	{
	public:
		GamepadStickEvent(uint8_t gampad, GamepadStick stick, const glm::ivec3& value) noexcept;
		void process(Input& input) noexcept;

	private:
		uint8_t _gamepad;
		GamepadStick _stick;
		glm::ivec3 _value;
	};

	class GamepadButtonEvent final : public PlatformEvent
	{
	public:
		GamepadButtonEvent(uint8_t gampad, GamepadButton button, bool down) noexcept;
		void process(Input& input) noexcept;

	private:
		uint8_t _gamepad;
		GamepadButton _button;
		bool _down;
	};

	class GamepadConnectEvent final : public PlatformEvent
	{
	public:
		GamepadConnectEvent(uint8_t gamepad, bool connected) noexcept;
		void process(Input& input) noexcept;
	private:
		uint8_t _gamepad;
		bool _connected;
	};

	class WindowSizeEvent final : public PlatformEvent
	{
	public:
		WindowSizeEvent(const glm::uvec2& size, bool pixel = false) noexcept;
		void process(Window& win) noexcept;
	private:
		glm::uvec2 _size;
		bool _pixel;
	};

	class WindowPhaseEvent final : public PlatformEvent
	{
	public:
		WindowPhaseEvent(WindowPhase phase) noexcept;
		void process(Window& win) noexcept;
	private:
		WindowPhase _phase;
	};

	class WindowModeEvent final : public PlatformEvent
	{
	public:
		WindowModeEvent(WindowMode mode) noexcept;
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
		void requestCursorModeChange(WindowCursorMode mode) noexcept;

		[[nodiscard]] void* getWindowHandle() const noexcept;
		[[nodiscard]] bgfx::NativeWindowHandleType::Enum getWindowHandleType() const noexcept;
		[[nodiscard]] void* getDisplayHandle() const noexcept;

		[[nodiscard]] std::unique_ptr<PlatformEvent> pollEvent() noexcept;
	private:
		Platform(PlatformImpl& impl) noexcept;
		PlatformImpl& _impl;
	};
}