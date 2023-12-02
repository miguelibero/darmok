#include "window.hpp"
#include "platform.hpp"

namespace darmok
{
	bool WindowHandle::operator==(const WindowHandle& other) const
	{
		return idx == other.idx;
	}

	bool WindowHandle::operator<(const WindowHandle& other) const
	{
		return idx < other.idx;
	}

	bool WindowHandle::isValid() const
	{
		return idx < Window::MaxAmount;
	}

	WindowPosition::WindowPosition(int32_t vx, int32_t vy)
		: x(vx)
		, y(vy)
	{
	}

	bool WindowPosition::operator==(const WindowPosition& other) const
	{
		return x == other.x && y == other.y;
	}

	WindowSize::WindowSize(int32_t vwidth , int32_t vheight)
		: width(vwidth)
		, height(vheight)
	{
	}

	bool WindowSize::operator==(const WindowSize& other) const
	{
		return width == other.width && height == other.height;
	}

	WindowImpl::WindowImpl()
		: _handle(Window::InvalidHandle)
		, _mouseLock(false)
		, _flags(WindowFlags::None)
		, _suspendPhase(WindowSuspendPhase::None)
	{
	}

	void WindowImpl::init(const WindowHandle& handle, const WindowCreationOptions& options)
	{
		_handle = handle;
		_size = options.size;
		_pos = options.pos.value_or(WindowPosition());
		_title = options.title;
		_flags = options.flags;
	}

	void WindowImpl::reset()
	{
		_handle = Window::InvalidHandle;
	}

	void WindowImpl::setPosition(const WindowPosition& pos)
	{
		_pos = pos;
	}

	void WindowImpl::setSize(const WindowSize& size)
	{
		_size = size;
	}

	void WindowImpl::setTitle(const std::string& title)
	{
		_title = title;
	}

	void WindowImpl::setFlags(uint32_t flags)
	{
		_flags = flags;
	}

	void WindowImpl::setDropFilePath(const std::string& filePath)
	{
		_dropFilePath = filePath;
	}

	void WindowImpl::setSuspendPhase(WindowSuspendPhase phase)
	{
		_suspendPhase = phase;
	}

	const WindowPosition& WindowImpl::getPosition() const
	{
		return _pos;
	}

	const WindowSize& WindowImpl::getSize() const
	{
		return _size;
	}

	const std::string& WindowImpl::getTitle() const
	{
		return _title;
	}

	const WindowHandle& WindowImpl::getHandle() const
	{
		return _handle;
	}

	uint32_t WindowImpl::getFlags() const
	{
		return _flags;
	}

	const std::string& WindowImpl::getDropFilePath() const
	{
		return _dropFilePath;
	}

	WindowSuspendPhase WindowImpl::getSuspendPhase() const
	{
		return _suspendPhase;
	}

	bool  WindowImpl::isRunning() const
	{
		return _handle.isValid();
	}

	ContextImpl::ContextImpl()
		: _windows{}
	{
	}

	Windows& ContextImpl::getWindows()
	{
		return _windows;
	}

	Window& ContextImpl::getWindow(const WindowHandle& handle)
	{
		return _windows[handle.idx];
	}

	Context::Context()
		: _impl(std::make_unique<ContextImpl>())
	{
	}

	Windows& Context::getWindows()
	{
		return _impl->getWindows();
	}

	Window& Context::getWindow(const WindowHandle& handle)
	{
		return _impl->getWindow(handle);
	}

	const Window& Context::getWindow(const WindowHandle& handle) const
	{
		return _impl->getWindow(handle);
	}

	Window& Context::createWindow(const WindowCreationOptions& options)
	{
		auto handle = PlatformContext::get().pushCreateWindowCmd(options);
		return getWindow(handle);
	}

	Context& Context::get()
	{
		static Context instance;
		return instance;
	}

	Window::Window()
		: _impl(std::make_unique<WindowImpl>())
	{
	}

	WindowImpl& Window::getImpl()
	{
		return *_impl;
	}

	const WindowImpl& Window::getImpl() const
	{
		return *_impl;
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
		return _impl->getPosition();
	}

	const WindowSize& Window::getSize() const
	{
		return _impl->getSize();
	}

	const std::string& Window::getTitle() const
	{
		return _impl->getTitle();
	}

	const WindowHandle& Window::getHandle() const
	{
		return _impl->getHandle();
	}

	uint32_t Window::getFlags() const
	{
		return _impl->getFlags();
	}

	const std::string& Window::getDropFilePath() const
	{
		return _impl->getDropFilePath();
	}

	WindowSuspendPhase Window::getSuspendPhase() const
	{
		return _impl->getSuspendPhase();
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