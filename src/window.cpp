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
		, _fbSize(0)
	{
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
			listener->onWindowSize(size);
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
		glm::uvec2 fsize = size;
		if (fsize.x == 0 && fsize.y == 0)
		{
			fsize = _fbSize;
		}
		bgfxReset(fsize);
		return true;
	}

	bool WindowImpl::setFramebufferSize(const glm::uvec2& size)
	{
		if (_fbSize == size)
		{
			return false;
		}
		_fbSize = size;
		if (_pixelSize.x == 0 && _pixelSize.y == 0)
		{
			bgfxReset(_fbSize);
		}
		return true;
	}

	void WindowImpl::bgfxReset(const glm::uvec2& size) const noexcept
	{
		for (auto& listener : _listeners)
		{
			listener->onWindowPixelSize(size);
		}
	}

	bool WindowImpl::setPhase(WindowPhase phase)
	{
		if (_phase == phase)
		{
			return false;
		}
		for (auto& listener : _listeners)
		{
			listener->onWindowPhase(phase);
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
			listener->onWindowVideoMode(mode);
		}
		return true;
	}

	void WindowImpl::setVideoModeInfo(const VideoModeInfo& info)
	{
		_videoModeInfo = info;
		for (auto& listener : _listeners)
		{
			listener->onWindowVideoModeInfo(info);
		}
	}

	void WindowImpl::onError(const std::string& error)
	{
		for (auto& listener : _listeners)
		{
			listener->onWindowError(error);
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
			listener->onWindowCursorMode(mode);
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
		if (_pixelSize.x == 0 && _pixelSize.y == 0)
		{
			return _fbSize;
		}
		return _pixelSize;
	}

	const glm::uvec2& WindowImpl::getFramebufferSize() const noexcept
	{
		return _fbSize;
	}

	WindowPhase WindowImpl::getPhase() const noexcept
	{
		return _phase;
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

	glm::vec2 WindowImpl::getScreenToWindowFactor() const noexcept
	{
		if (_fbSize.x == 0 || _fbSize.y == 0)
		{
			return glm::vec2(0);
		}
		return glm::vec2(_size) / glm::vec2(_fbSize);
	}

	glm::vec2 WindowImpl::screenToWindowPoint(const glm::vec2& point) const noexcept
	{
		auto f = getScreenToWindowFactor();
		auto p = (point - glm::vec2(0.5F)) * f;
		p.y = _size.y - p.y;
		return p;
	}

	glm::vec2 WindowImpl::windowToScreenPoint(const glm::vec2& point) const noexcept
	{
		glm::vec2 p(point);
		p.y = _size.y - p.y;
		auto f = getScreenToWindowFactor();
		p = p / f + glm::vec2(0.5F);
		return p;
	}

	void WindowImpl::addListener(IWindowListener& listener) noexcept
	{
		_listeners.insert(listener);
	}

	bool WindowImpl::removeListener(IWindowListener& listener) noexcept
	{
		return _listeners.erase(listener) > 0;
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

	const glm::uvec2& Window::getFramebufferSize() const noexcept
	{
		return _impl->getFramebufferSize();
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

	glm::vec2 Window::getScreenToWindowFactor() const noexcept
	{
		return _impl->getScreenToWindowFactor();
	}

	void Window::addListener(IWindowListener& listener) noexcept
	{
		_impl->addListener(listener);
	}

	bool Window::removeListener(IWindowListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}
}