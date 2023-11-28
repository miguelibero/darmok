#include <darmok/window.hpp>
#include "platform.hpp"
#include <array>

namespace darmok
{
	std::array<Window, Window::MaxWindows> s_windows;

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

	Window& Window::create(const WindowCreationOptions& options)
	{
		auto handle = PlatformContext::get().pushCreateWindowCmd(options);
		auto& win = s_windows[handle.idx];
		win._handle = handle;
		return win;
	}

	void Window::destroy()
	{
		PlatformContext::get().pushDestroyWindowCmd(getHandle());
	}

	void Window::setPosition(const WindowPosition& pos)
	{
		PlatformContext::get().pushSetWindowPositionCmd(getHandle(), pos);
	}

	void Window::setSize(const WindowSize& size)
	{
		PlatformContext::get().pushSetWindowSizeCmd(getHandle(), size);
	}

	void Window::setTitle(const std::string& title)
	{
		PlatformContext::get().pushSetWindowTitleCmd(getHandle(), title);
	}

	void Window::setFlags(uint32_t flags, bool enabled)
	{
		PlatformContext::get().pushSetWindowFlagsCmd(getHandle(), flags, enabled);
	}

	void Window::toggleFullscreen()
	{
		PlatformContext::get().pushToggleWindowFullscreenCmd(getHandle());
	}

	void Window::setMouseLock(bool lock)
	{
		PlatformContext::get().pushSetMouseLockToWindowCmd(getHandle(), lock);
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

	void* Window::getNativeHandle() const
	{
		return PlatformContext::get().getNativeWindowHandle(getHandle());
	}

	void* Window::getNativeDisplayHandle()
	{
		return PlatformContext::get().getNativeDisplayHandle();
	}

	bgfx::NativeWindowHandleType::Enum Window::getNativeHandleType() const
	{
		return PlatformContext::get().getNativeWindowHandleType(getHandle());
	}
}