#pragma once

#include <darmok/entry.hpp>
#include <queue>
#include <bx/filepath.h>

#ifndef ENTRY_CONFIG_USE_NOOP
#	define ENTRY_CONFIG_USE_NOOP 0
#endif // ENTRY_CONFIG_USE_NOOP

#ifndef ENTRY_CONFIG_USE_SDL
#	define ENTRY_CONFIG_USE_SDL 0
#endif // ENTRY_CONFIG_USE_SDL

#ifndef ENTRY_CONFIG_USE_GLFW
#	define ENTRY_CONFIG_USE_GLFW 0
#endif // ENTRY_CONFIG_USE_GLFW

#ifndef ENTRY_CONFIG_USE_WAYLAND
#	define ENTRY_CONFIG_USE_WAYLAND 0
#endif // ENTRY_CONFIG_USE_WAYLAND

#if !defined(ENTRY_CONFIG_USE_NATIVE) \
	&& !ENTRY_CONFIG_USE_NOOP \
	&& !ENTRY_CONFIG_USE_SDL \
	&& !ENTRY_CONFIG_USE_GLFW
#	define ENTRY_CONFIG_USE_NATIVE 1
#else
#	define ENTRY_CONFIG_USE_NATIVE 0
#endif // ...

#ifndef ENTRY_CONFIG_MAX_WINDOWS
#	define ENTRY_CONFIG_MAX_WINDOWS 8
#endif // ENTRY_CONFIG_MAX_WINDOWS

#ifndef ENTRY_CONFIG_MAX_GAMEPADS
#	define ENTRY_CONFIG_MAX_GAMEPADS 4
#endif // ENTRY_CONFIG_MAX_GAMEPADS

#if !defined(ENTRY_DEFAULT_WIDTH) && !defined(ENTRY_DEFAULT_HEIGHT)
#	define ENTRY_DEFAULT_WIDTH  1280
#	define ENTRY_DEFAULT_HEIGHT 720
#elif !defined(ENTRY_DEFAULT_WIDTH) || !defined(ENTRY_DEFAULT_HEIGHT)
#	error "Both ENTRY_DEFAULT_WIDTH and ENTRY_DEFAULT_HEIGHT must be defined."
#endif // ENTRY_DEFAULT_WIDTH

#ifndef ENTRY_CONFIG_IMPLEMENT_DEFAULT_ALLOCATOR
#	define ENTRY_CONFIG_IMPLEMENT_DEFAULT_ALLOCATOR 1
#endif // ENTRY_CONFIG_IMPLEMENT_DEFAULT_ALLOCATOR

#ifndef ENTRY_CONFIG_PROFILER
#	define ENTRY_CONFIG_PROFILER 0
#endif // ENTRY_CONFIG_PROFILER

#define ENTRY_IMPLEMENT_EVENT(cls, type) \
			cls(WindowHandle handle) : Event(type, handle) {} \
			void process() override;
namespace darmok
{
	int main(int argc, const char* const* argv);

	char keyToAscii(Key key, uint8_t modifiers);

	class Event
	{
	public:
		enum Type
		{
			Axis,
			Char,
			Exit,
			Gamepad,
			Key,
			Mouse,
			Size,
			Window,
			Suspend,
			DropFile,
		};

		Event(Type type)
			: type(type)
		{
			handle.idx = UINT16_MAX;
		}

		Event(Type type, WindowHandle handle)
			: type(type)
			, handle(handle)
		{
		}

		Event::Type type;
		WindowHandle handle;

		virtual void process();
	};

	struct AxisEvent final : public Event
	{
		ENTRY_IMPLEMENT_EVENT(AxisEvent, Event::Axis);

		GamepadAxis axis;
		int32_t value;
		GamepadHandle gamepad;

	};

	struct CharEvent final : public Event
	{
		ENTRY_IMPLEMENT_EVENT(CharEvent, Event::Char);

		uint8_t len;
		std::array<uint8_t, 4> data;
	};

	struct GamepadEvent final : public Event
	{
		ENTRY_IMPLEMENT_EVENT(GamepadEvent, Event::Gamepad);

		GamepadHandle gamepad;
		bool connected;
	};

	struct KeyEvent final : public Event
	{
		ENTRY_IMPLEMENT_EVENT(KeyEvent, Event::Key);

		darmok::Key key;
		uint8_t modifiers;
		bool down;
	};

	struct MouseEvent final : public Event
	{
		ENTRY_IMPLEMENT_EVENT(MouseEvent, Event::Mouse);

		int32_t x;
		int32_t y;
		int32_t z;
		MouseButton button;
		bool down;
		bool move;
	};

	struct SizeEvent final : public Event
	{
		ENTRY_IMPLEMENT_EVENT(SizeEvent, Event::Size);

		uint32_t width;
		uint32_t height;
	};

	struct WindowEvent final : public Event
	{
		ENTRY_IMPLEMENT_EVENT(WindowEvent, Event::Window);

		void* nwh;
	};

	struct SuspendEvent final : public Event
	{
		ENTRY_IMPLEMENT_EVENT(SuspendEvent, Event::Suspend);

		darmok::Suspend state;
	};

	struct DropFileEvent final : public Event
	{
		ENTRY_IMPLEMENT_EVENT(DropFileEvent, Event::DropFile);

		bx::FilePath filePath;
	};

	std::unique_ptr<Event>&& pollEvent();
	std::unique_ptr<Event>&& pollEvent(WindowHandle handle);

	class EventQueue final
	{
	public:
		void postAxisEvent(WindowHandle handle, GamepadHandle gamepad, GamepadAxis axis, int32_t value)
		{
			auto ev = std::make_unique<AxisEvent>(handle);
			ev->gamepad = gamepad;
			ev->axis    = axis;
			ev->value   = value;
			_queue.push(std::move(ev));
		}

		void postCharEvent(WindowHandle handle, uint8_t len, std::array<uint8_t, 4>&& data)
		{
			auto ev = std::make_unique<CharEvent>(handle);
			ev->len = len;
			ev->data = std::move(data);
			_queue.push(std::move(ev));
		}

		void postExitEvent()
		{
			auto ev = std::make_unique<Event>(Event::Exit);
			_queue.push(std::move(ev));
		}

		void postGamepadEvent(WindowHandle handle, GamepadHandle gamepad, bool connected)
		{
			auto ev = std::make_unique<GamepadEvent>(handle);
			ev->gamepad   = gamepad;
			ev->connected = connected;
			_queue.push(std::move(ev));
		}

		void postKeyEvent(WindowHandle handle, Key key, uint8_t modifiers, bool down)
		{
			auto ev = std::make_unique<KeyEvent>(handle);
			ev->key       = key;
			ev->modifiers = modifiers;
			ev->down      = down;
			_queue.push(std::move(ev));
		}

		void postMouseEvent(WindowHandle handle, int32_t x, int32_t y, int32_t z)
		{
			auto ev = std::make_unique<MouseEvent>(handle);
			ev->x     = x;
			ev->y     = y;
			ev->z     = z;
			ev->button = MouseButton::None;
			ev->down   = false;
			ev->move   = true;
			_queue.push(std::move(ev));
		}

		void postMouseEvent(WindowHandle handle, int32_t x, int32_t y, int32_t z, MouseButton button, bool down)
		{
			auto ev = std::make_unique<MouseEvent>(handle);
			ev->x     = x;
			ev->y     = y;
			ev->z     = z;
			ev->button = button;
			ev->down   = down;
			ev->move   = false;
			_queue.push(std::move(ev));
		}

		void postSizeEvent(WindowHandle handle, uint32_t width, uint32_t height)
		{
			auto ev = std::make_unique<SizeEvent>(handle);
			ev->width  = width;
			ev->height = height;
			_queue.push(std::move(ev));
		}

		void postWindowEvent(WindowHandle handle, void* nwh = NULL)
		{
			auto ev = std::make_unique<WindowEvent>(handle);
			ev->nwh = nwh;
			_queue.push(std::move(ev));
		}

		void postSuspendEvent(WindowHandle handle, Suspend state)
		{
			auto ev = std::make_unique<SuspendEvent>(handle);
			ev->state = state;
			_queue.push(std::move(ev));
		}

		void postDropFileEvent(WindowHandle handle, const bx::FilePath& filePath)
		{
			auto ev = std::make_unique<DropFileEvent>(handle);
			ev->filePath = filePath;
			_queue.push(std::move(ev));
		}

		std::unique_ptr<Event>&& poll()
		{
			auto ev = std::move(_queue.front());
			_queue.pop();
			return std::move(ev);
		}

		std::unique_ptr<Event>&& poll(WindowHandle handle)
		{
			if (isValid(handle) )
			{
				auto& ev = _queue.front();
				if (ev == nullptr || ev->handle.idx != handle.idx)
				{
					return nullptr;
				}
			}
			return poll();
		}

	private:
		std::queue<std::unique_ptr<Event>> _queue;
	};

}

