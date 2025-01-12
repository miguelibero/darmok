#include "window.hpp"
#include "platform.hpp"
#include <sstream>

namespace darmok
{
	bool VideoMode::operator==(const VideoMode& other) const noexcept
	{
		return screenMode == other.screenMode
			&& size == other.size
			&& depth == other.depth
			&& refreshRate == other.refreshRate
			&& monitor == other.monitor;
	}

	bool VideoMode::operator!=(const VideoMode& other) const noexcept
	{
		return !operator==(other);
	}

	std::string VideoMode::toShortString() const noexcept
	{
		std::ostringstream ss;
		ss << size.x << "x" << size.y;
		if (refreshRate > 0)
		{
			ss << " " << refreshRate << "Hz";
		}
		if (depth.r > 0 || depth.g > 0 || depth.b > 0)
		{
			ss << " ";
			if (depth.r == depth.g && depth.r == depth.b)
			{
				ss << (unsigned int)depth.r;
			}
			else
			{
				ss << (unsigned int)depth.r << "-" << (unsigned int)depth.g << "-" << (unsigned int)depth.b;
			}
			ss << "bpp";
		}
		return ss.str();
	}

	std::string VideoMode::toString() const noexcept
	{
		std::ostringstream ss;
		switch (screenMode)
		{
		case WindowScreenMode::Normal:
			ss << "Windowed ";
			break;
		case WindowScreenMode::Fullscreen:
			ss << "Fullscreen ";
			break;
		case WindowScreenMode::WindowedFullscreen:
			ss << "Borderless Windowed ";
			break;
		default:
			break;
		}
		ss << toShortString();
		if (monitor >= 0)
		{
			ss << " monitor " << monitor;
		}
		return ss.str();
	}

	bool VideoMode::complete() const noexcept
	{
		return screenMode != WindowScreenMode::Count
			&& size.x > 0 && size.y > 0
			&& depth.r > 0 && depth.g > 0 && depth.b > 0
			&& refreshRate > 0;
	}

	WindowImpl::WindowImpl(Platform& plat) noexcept
		: _plat(plat)
		, _phase(WindowPhase::Unknown)
		, _cursorMode(WindowCursorMode::Normal)
		, _size(0)
		, _pixelSize(0)
		, _title("darmok")
	{
	}

	void WindowImpl::shutdown() noexcept
	{
		_listeners.clear();
	}

	bool WindowImpl::setSize(const glm::uvec2& size)
	{
		if (_size == size)
		{
			return false;
		}
		_size = size;
		for (auto& listener : _listeners)
		{
			listener.onWindowSize(size);
		}
		return true;
	}

	bool WindowImpl::setPixelSize(const glm::uvec2& size)
	{
		if (_pixelSize == size)
		{
			return false;
		}
		_pixelSize = size;
		for (auto& listener : _listeners)
		{
			listener.onWindowPixelSize(size);
		}
		return true;
	}

	bool WindowImpl::setPhase(WindowPhase phase)
	{
		if (_phase == phase)
		{
			return false;
		}
		for (auto& listener : _listeners)
		{
			listener.onWindowPhase(phase);
		}
		_phase = phase;
		return true;
	}

	bool WindowImpl::setVideoMode(const VideoMode& mode)
	{
		if (_videoMode == mode)
		{
			return false;
		}
		_videoMode = mode;
		for (auto& listener : _listeners)
		{
			listener.onWindowVideoMode(mode);
		}
		return true;
	}

	void WindowImpl::setVideoModeInfo(const VideoModeInfo& info)
	{
		_videoModeInfo = info;
		for (auto& listener : _listeners)
		{
			listener.onWindowVideoModeInfo(info);
		}
	}

	void WindowImpl::setTitle(const std::string& title)
	{
		_title = title;
	}

	void WindowImpl::onError(const std::string& error)
	{
		for (auto& listener : _listeners)
		{
			listener.onWindowError(error);
		}
	}

	bool WindowImpl::setCursorMode(WindowCursorMode mode)
	{
		if (_cursorMode == mode)
		{
			return false;
		}
		for (auto& listener : _listeners)
		{
			listener.onWindowCursorMode(mode);
		}
		_cursorMode = mode;
		return true;
	}

	const glm::uvec2& WindowImpl::getSize() const noexcept
	{
		return _size;
	}

	const glm::uvec2& WindowImpl::getPixelSize() const noexcept
	{
		return _pixelSize;
	}

	WindowPhase WindowImpl::getPhase() const noexcept
	{
		return _phase;
	}

	const std::string& WindowImpl::getTitle() const noexcept
	{
		return _title;
	}

	void WindowImpl::requestTitle(const std::string& title)
	{
		_plat.requestWindowTitle(title);
	}

	void WindowImpl::requestVideoModeInfo() noexcept
	{
		_plat.requestVideoModeInfo();
	}

	void WindowImpl::requestVideoMode(const VideoMode& mode) noexcept
	{
		_plat.requestWindowVideoModeChange(mode);
	}

	void WindowImpl::requestCursorMode(WindowCursorMode mode) noexcept
	{
		_plat.requestWindowCursorModeChange(mode);
	}

	void WindowImpl::requestDestruction() noexcept
	{
		_plat.requestWindowDestruction();
	}

	const VideoMode& WindowImpl::getVideoMode() const noexcept
	{
		return _videoMode;
	}

	const VideoModeInfo& WindowImpl::getVideoModeInfo() const noexcept
	{
		return _videoModeInfo;
	}

	WindowCursorMode WindowImpl::getCursorMode() const noexcept
	{
		return _cursorMode;
	}

	glm::vec2 WindowImpl::getFramebufferScale() const noexcept
	{
		if (_size.x == 0 || _size.y == 0)
		{
			return glm::vec2(1);
		}
		return glm::vec2(_pixelSize) / glm::vec2(_size);
	}

	glm::vec2 WindowImpl::windowToScreenDelta(const glm::vec2& delta) const noexcept
	{
		auto screenDelta = delta * getFramebufferScale();
		return screenDelta;
	}

	glm::vec2 WindowImpl::screenToWindowDelta(const glm::vec2& delta) const noexcept
	{
		auto screenDelta = delta / getFramebufferScale();
		return screenDelta;
	}

	glm::vec2 WindowImpl::screenToWindowPoint(const glm::vec2& point) const noexcept
	{
		auto f = getFramebufferScale();
		auto p = (point - glm::vec2(0.5F)) / f;
		return p;
	}

	glm::vec2 WindowImpl::windowToScreenPoint(const glm::vec2& point) const noexcept
	{
		glm::vec2 p(point);
		auto f = getFramebufferScale();
		p = (p * f) + glm::vec2(0.5F);
		return p;
	}

	void WindowImpl::addListener(std::unique_ptr<IWindowListener>&& listener) noexcept
	{
		_listeners.insert(std::move(listener));
	}

	void WindowImpl::addListener(IWindowListener& listener) noexcept
	{
		_listeners.insert(listener);
	}

	bool WindowImpl::removeListener(const IWindowListener& listener) noexcept
	{
		return _listeners.erase(listener);
	}

	size_t WindowImpl::removeListeners(const IWindowListenerFilter& filter) noexcept
	{
		return _listeners.eraseIf(filter);
	}

	Window::Window(Platform& plat) noexcept
		: _impl(std::make_unique<WindowImpl>(plat))
	{
	}

	Window::~Window() noexcept
	{
		// left empty to get the forward declaration of the impl working
	}

	WindowImpl& Window::getImpl() noexcept
	{
		return *_impl;
	}

	const WindowImpl& Window::getImpl() const noexcept
	{
		return *_impl;
	}

	float Window::getAspect() const noexcept
	{
		auto& size = getPixelSize();
		return (float)size.x / size.y;
	}

	const glm::uvec2& Window::getSize() const noexcept
	{
		return _impl->getSize();
	}

	const glm::uvec2& Window::getPixelSize() const noexcept
	{
		return _impl->getPixelSize();
	}

	const std::string& Window::getTitle() const noexcept
	{
		return _impl->getTitle();
	}

	void Window::requestTitle(const std::string& title)
	{
		_impl->requestTitle(title);
	}


	void Window::requestVideoModeInfo() noexcept
	{
		_impl->requestVideoModeInfo();
	}

	void Window::requestVideoMode(const VideoMode& mode) noexcept
	{
		_impl->requestVideoMode(mode);
	}

	void Window::requestCursorMode(WindowCursorMode mode) noexcept
	{
		_impl->requestCursorMode(mode);
	}

	void Window::requestDestruction() noexcept
	{
		_impl->requestDestruction();
	}

	WindowPhase Window::getPhase() const noexcept
	{
		return _impl->getPhase();
	}

	const VideoMode& Window::getVideoMode() const noexcept
	{
		return _impl->getVideoMode();
	}

	const VideoModeInfo& Window::getVideoModeInfo() const noexcept
	{
		return _impl->getVideoModeInfo();
	}

	WindowCursorMode Window::getCursorMode() const noexcept
	{
		return _impl->getCursorMode();
	}

	glm::vec2 Window::windowToScreenPoint(const glm::vec2& pos) const noexcept
	{
		return _impl->windowToScreenPoint(pos);
	}

	glm::vec2 Window::screenToWindowPoint(const glm::vec2& pos) const noexcept
	{
		return _impl->screenToWindowPoint(pos);
	}

	glm::vec2 Window::getFramebufferScale() const noexcept
	{
		return _impl->getFramebufferScale();
	}

	glm::vec2 Window::windowToScreenDelta(const glm::vec2& delta) const noexcept
	{
		return _impl->windowToScreenDelta(delta);
	}

	glm::vec2 Window::screenToWindowDelta(const glm::vec2& delta) const noexcept
	{
		return _impl->screenToWindowDelta(delta);
	}

	void Window::addListener(IWindowListener& listener) noexcept
	{
		_impl->addListener(listener);
	}

	void Window::addListener(std::unique_ptr<IWindowListener>&& listener) noexcept
	{
		_impl->addListener(std::move(listener));
	}

	bool Window::removeListener(const IWindowListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}

	size_t Window::removeListeners(const IWindowListenerFilter& filter) noexcept
	{
		return _impl->removeListeners(filter);
	}
}