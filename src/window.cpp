#include <darmok/window.hpp>
#include <array>

namespace darmok
{
	std::array<Window, Window::MaxWindows> Window::s_windows;

	Window::Window(const WindowHandle& handle)
		: _handle(handle)
		, _mouseLock(false)
		, _flags(WindowFlags::None)
		, _suspendPhase(WindowSuspendPhase::None)
	{
	}

	Window& Window::get(const WindowHandle& handle)
	{
		auto& win = s_windows[handle.idx];
		if (handle == DefaultHandle)
		{
			win._handle = handle;
		}
		return win;
	}

	const WindowPosition& Window::getPosition() const
	{
		return _pos;
	}

	const WindowSize& Window::getSize() const
	{
		return _size;
	}

	const std::string& Window::getTitle() const
	{
		return _title;
	}

	const WindowHandle& Window::getHandle() const
	{
		return _handle;
	}

	uint32_t Window::getFlags() const
	{
		return _flags;
	}

	const std::string& Window::getDropFilePath() const
	{
		return _dropFilePath;
	}

	WindowSuspendPhase Window::getSuspendPhase() const
	{
		return _suspendPhase;
	}

}
