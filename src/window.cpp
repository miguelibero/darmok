#include "window.hpp"
#include "platform.hpp"

namespace darmok
{
	WindowImpl::WindowImpl() noexcept
		: _handle(Window::InvalidHandle)
		, _mouseLock(false)
		, _flags(WindowFlags::None)
		, _suspended(false)
		, _frameBuffer{ bgfx::kInvalidHandle }
	{
	}

	void WindowImpl::init(const WindowHandle& handle, const WindowCreationOptions& options)
	{
		_handle = handle;
		_size = options.size;
		_pos = options.pos.value_or(WindowPosition());
		_title = options.title;
		_flags = options.flags;

		createFrameBuffer();
	}

	void WindowImpl::createFrameBuffer()
	{
		resetFrameBuffer();
		if (_handle.idx != Window::DefaultHandle.idx)
		{
			_frameBuffer = bgfx::createFrameBuffer(getNativeHandle(), uint16_t(_size.x), uint16_t(_size.y));
		}
	}

	bool WindowImpl::resetFrameBuffer()
	{
		if (!bgfx::isValid(_frameBuffer))
		{
			return false;
		}
		bgfx::destroy(_frameBuffer);
		_frameBuffer.idx = bgfx::kInvalidHandle;
		return true;
	}

	void WindowImpl::reset()
	{
		resetFrameBuffer();
		_handle = Window::InvalidHandle;
	}

	void WindowImpl::requestScreenshot(std::string_view path) noexcept
	{
		bgfx::requestScreenShot(_frameBuffer, std::string(path).c_str());
	}

	void WindowImpl::setPosition(const WindowPosition& pos) noexcept
	{
		_pos = pos;
	}

	void WindowImpl::setSize(const WindowSize& size) noexcept
	{
		if (_size != size)
		{
			_size = size;
			createFrameBuffer();
		}
	}

	void WindowImpl::setTitle(const std::string& title) noexcept
	{
		_title = title;
	}

	void WindowImpl::setFlags(uint32_t flags) noexcept
	{
		_flags = flags;
	}

	void WindowImpl::setDropFilePath(const std::string& filePath) noexcept
	{
		_dropFilePath = filePath;
	}

	void WindowImpl::onSuspendPhase(WindowSuspendPhase phase) noexcept
	{
		switch (phase)
		{
		case WindowSuspendPhase::DidSuspend:
		case WindowSuspendPhase::WillResume:
			_suspended = true;
			break;
		default:
			_suspended = false;
			break;
		}
	}

	void WindowImpl::setHandle(const WindowHandle& handle) noexcept
	{
		_handle = handle;
	}

	const WindowPosition& WindowImpl::getPosition() const noexcept
	{
		return _pos;
	}

	const WindowSize& WindowImpl::getSize() const noexcept
	{
		return _size;
	}

	const std::string& WindowImpl::getTitle() const noexcept
	{
		return _title;
	}

	const WindowHandle& WindowImpl::getHandle() const noexcept
	{
		return _handle;
	}

	uint32_t WindowImpl::getFlags() const noexcept
	{
		return _flags;
	}

	const std::string& WindowImpl::getDropFilePath() const noexcept
	{
		return _dropFilePath;
	}

	bool  WindowImpl::isRunning() const noexcept
	{
		return isValid(_handle);
	}

	bool  WindowImpl::isSuspended() const noexcept
	{
		return _suspended;
	}

	const bgfx::FrameBufferHandle& WindowImpl::getFrameBuffer() const noexcept
	{
		return _frameBuffer;
	}

	void* WindowImpl::getNativeHandle() const noexcept
	{
		return PlatformContext::get().getNativeWindowHandle(getHandle());
	}

	void* WindowImpl::getNativeDisplayHandle() noexcept
	{
		return PlatformContext::get().getNativeDisplayHandle();
	}

	bgfx::NativeWindowHandleType::Enum WindowImpl::getNativeHandleType() const noexcept
	{
		return PlatformContext::get().getNativeWindowHandleType(getHandle());
	}

	WindowContextImpl::WindowContextImpl() noexcept
		: _windows{}
	{
	}

	Windows& WindowContextImpl::getWindows() noexcept
	{
		return _windows;
	}

	Window& WindowContextImpl::getWindow(const WindowHandle& handle) noexcept
	{
		return _windows[handle.idx];
	}

	WindowContext::WindowContext() noexcept
		: _impl(std::make_unique<WindowContextImpl>())
	{
	}

	Windows& WindowContext::getWindows() noexcept
	{
		return _impl->getWindows();
	}

	Window& WindowContext::getWindow(const WindowHandle& handle) noexcept
	{
		return _impl->getWindow(handle);
	}

	const Window& WindowContext::getWindow(const WindowHandle& handle) const noexcept
	{
		return _impl->getWindow(handle);
	}

	Window& WindowContext::createWindow(const WindowCreationOptions& options)
	{
		auto handle = PlatformContext::get().pushCreateWindowCmd(options);
		auto& win = getWindow(handle);
		win.getImpl().setHandle(handle);
		return win;
	}

	WindowContext& WindowContext::get() noexcept
	{
		static WindowContext instance;
		return instance;
	}

	Window::Window() noexcept
		: _impl(std::make_unique<WindowImpl>())
	{
	}

	WindowImpl& Window::getImpl() noexcept
	{
		return *_impl;
	}

	const WindowImpl& Window::getImpl() const noexcept
	{
		return *_impl;
	}

	void Window::destroy() noexcept
	{
		PlatformContext::get().pushDestroyWindowCmd(getHandle());
	}

	void Window::setPosition(const WindowPosition& pos) noexcept
	{
		PlatformContext::get().pushSetWindowPositionCmd(getHandle(), pos);
	}

	void Window::setSize(const WindowSize& size) noexcept
	{
		PlatformContext::get().pushSetWindowSizeCmd(getHandle(), size);
	}

	void Window::setTitle(const std::string& title) noexcept
	{
		PlatformContext::get().pushSetWindowTitleCmd(getHandle(), title);
	}

	void Window::setFlags(uint32_t flags, bool enabled) noexcept
	{
		PlatformContext::get().pushSetWindowFlagsCmd(getHandle(), flags, enabled);
	}

	void Window::toggleFullscreen() noexcept
	{
		PlatformContext::get().pushToggleWindowFullscreenCmd(getHandle());
	}

	void Window::setMouseLock(bool lock) noexcept
	{
		PlatformContext::get().pushSetMouseLockToWindowCmd(getHandle(), lock);
	}

	void Window::requestScreenshot(std::string_view path) noexcept
	{
		_impl->requestScreenshot(path);
	}

	const WindowPosition& Window::getPosition() const noexcept
	{
		return _impl->getPosition();
	}

	const WindowSize& Window::getSize() const noexcept
	{
		return _impl->getSize();
	}

	const std::string& Window::getTitle() const noexcept
	{
		return _impl->getTitle();
	}

	const WindowHandle& Window::getHandle() const noexcept
	{
		return _impl->getHandle();
	}

	uint32_t Window::getFlags() const noexcept
	{
		return _impl->getFlags();
	}

	const std::string& Window::getDropFilePath() const noexcept
	{
		return _impl->getDropFilePath();
	}

	bool Window::isRunning() const noexcept
	{
		return _impl->isRunning();
	}

	bool Window::isSuspended() const noexcept
	{
		return _impl->isSuspended();
	}

	void* Window::getNativeHandle() const noexcept
	{
		return _impl->getNativeHandle();
	}

	void* Window::getNativeDisplayHandle() noexcept
	{
		return WindowImpl::getNativeDisplayHandle();
	}

	bgfx::NativeWindowHandleType::Enum Window::getNativeHandleType() const noexcept
	{
		return _impl->getNativeHandleType();
	}
}