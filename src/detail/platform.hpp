#pragma once

#include "input.hpp"
#include <darmok/window.hpp>
#include <darmok/utils.hpp>
#include <darmok/app_fwd.hpp>
#include <bx/bx.h>
#include <queue>
#include <memory>
#include <mutex>
#include <string>

#ifndef DARMOK_PLATFORM_NOOP
#	define DARMOK_PLATFORM_NOOP 0
#endif // DARMOK_PLATFORM_NOOP

#ifndef DARMOK_PLATFORM_GLFW
#	define DARMOK_PLATFORM_GLFW 0
#endif // DARMOK_PLATFORM_GLFW

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
			WindowPixelSize,
			WindowPhase,
			WindowVideoMode,
			WindowCursorMode,
			WindowTitle,
			WindowError,

			VideoModeInfo,

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
		KeyboardKeyEvent(KeyboardKey key, KeyboardModifiers modifiers, bool down) noexcept;
		void process(Input& input) noexcept;
	private:
		KeyboardKey _key;
		KeyboardModifiers _modifiers;
		bool _down;
	};

	class KeyboardCharEvent final : public PlatformEvent
	{
	public:
		KeyboardCharEvent(char32_t chr) noexcept;
		void process(Input& input) noexcept;
	private:
		char32_t _chr;
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
		GamepadStickEvent(uint8_t gampad, GamepadStick stick, const glm::vec3& value) noexcept;
		void process(Input& input) noexcept;

	private:
		uint8_t _gamepad;
		GamepadStick _stick;
		glm::vec3 _value;
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
		WindowSizeEvent(const glm::uvec2& size) noexcept;
		void process(Window& win) noexcept;
	private:
		glm::uvec2 _size;
	};

	class WindowPixelSizeEvent final : public PlatformEvent
	{
	public:
		WindowPixelSizeEvent(const glm::uvec2& size) noexcept;
		void process(Window& win) noexcept;
	private:
		glm::uvec2 _size;
	};

	class WindowPhaseEvent final : public PlatformEvent
	{
	public:
		WindowPhaseEvent(WindowPhase phase) noexcept;
		void process(Window& win) noexcept;
	private:
		WindowPhase _phase;
	};

	class WindowErrorEvent final : public PlatformEvent
	{
	public:
		WindowErrorEvent(std::string err) noexcept;
		void process(Window& win) noexcept;
	private:
		std::string _error;
	};

	class WindowVideoModeEvent final : public PlatformEvent
	{
	public:
		WindowVideoModeEvent(const VideoMode& mode) noexcept;
		void process(Window& win) noexcept;
	private:
		VideoMode _mode;
	};

	class VideoModeInfoEvent final : public PlatformEvent
	{
	public:
		VideoModeInfoEvent(VideoModeInfo info) noexcept;
		void process(Window& win) noexcept;
	private:
		VideoModeInfo _info;
	};

	class WindowCursorModeEvent final : public PlatformEvent
	{
	public:
		WindowCursorModeEvent(WindowCursorMode mode) noexcept;
		void process(Window& win) noexcept;
	private:
		WindowCursorMode _mode;
	};

	class WindowTitleEvent final : public PlatformEvent
	{
	public:
		WindowTitleEvent(std::string title) noexcept;
		void process(Window& win) noexcept;
	private:
		std::string _title;
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
		std::mutex _mutex;
		std::queue<std::unique_ptr<PlatformEvent>> _events;
	};

#pragma endregion PlatformEvents

	class PlatformImpl;

	class BX_NO_VTABLE IPlatformRunnable
	{
	public:
		virtual ~IPlatformRunnable() = default;
		virtual int32_t operator()() noexcept = 0;
	};

	class Platform final
	{
	public:
		static Platform& get() noexcept;

		Platform(const Platform& other) = delete;
		Platform(Platform&& other) = delete;

		const PlatformImpl& getImpl() const noexcept;
		PlatformImpl& getImpl() noexcept;

		void requestVideoModeInfo() noexcept;
		void requestWindowDestruction() noexcept;
		void requestWindowVideoModeChange(const VideoMode& mode) noexcept;
		void requestWindowCursorModeChange(WindowCursorMode mode) noexcept;
		void requestWindowTitle(const std::string& title) noexcept;

		[[nodiscard]] void* getWindowHandle() const noexcept;
		[[nodiscard]] bgfx::NativeWindowHandleType::Enum getWindowHandleType() const noexcept;
		[[nodiscard]] void* getDisplayHandle() const noexcept;

		[[nodiscard]] std::unique_ptr<PlatformEvent> pollEvent() noexcept;

		int32_t run(std::unique_ptr<IPlatformRunnable>&& runnable);

	private:
		Platform() noexcept;
		~Platform() noexcept;
		std::unique_ptr<PlatformImpl> _impl;
	};
}