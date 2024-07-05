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

	std::string VideoMode::to_string() const noexcept
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

	WindowImpl::WindowImpl() noexcept
		: _phase(WindowPhase::Unknown)
		, _cursorMode(WindowCursorMode::Normal)
		, _size(0)
		, _pixelSize(0)
	{
	}

	bool WindowImpl::setSize(const glm::uvec2& size)
	{
		if (_size == size)
		{
			return false;
		}
		for (auto& listener : _listeners)
		{
			listener->onWindowSize(size);
		}
		_size = size;
		return true;
	}

	bool WindowImpl::setPixelSize(const glm::uvec2& size)
	{
		if (_pixelSize == size)
		{
			return false;
		}
		for (auto& listener : _listeners)
		{
			listener->onWindowPixelSize(size);
		}
		_pixelSize = size;
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
		for (auto& listener : _listeners)
		{
			listener->onWindowVideoMode(mode);
		}
		_videoMode = mode;
		return true;
	}

	void WindowImpl::onSupportedVideoModes(const VideoModeInfo& info)
	{
		for (auto& listener : _listeners)
		{
			listener->onWindowSupportedVideoModes(info);
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
		return _pixelSize;
	}

	WindowPhase WindowImpl::getPhase() const noexcept
	{
		return _phase;
	}

	const VideoMode& WindowImpl::getVideoMode() const noexcept
	{
		return _videoMode;
	}

	WindowCursorMode WindowImpl::getCursorMode() const noexcept
	{
		return _cursorMode;
	}

	glm::vec2 WindowImpl::getScreenToWindowFactor() const noexcept
	{
		return glm::vec2(_size) / glm::vec2(_pixelSize);
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
		: _impl(std::make_unique<WindowImpl>())
		, _plat(plat)
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

	const glm::uvec2& Window::getSize() const noexcept
	{
		return _impl->getSize();
	}

	const glm::uvec2& Window::getPixelSize() const noexcept
	{
		return _impl->getPixelSize();
	}

	void Window::requestSupportedVideoModes() noexcept
	{
		_plat.requestSupportedWindowVideoModes();
	}

	void Window::requestVideoMode(const VideoMode& mode) noexcept
	{
		_plat.requestWindowVideoModeChange(mode);
	}

	void Window::requestCursorMode(WindowCursorMode mode) noexcept
	{
		_plat.requestWindowCursorModeChange(mode);
	}

	void Window::requestDestruction() noexcept
	{
		_plat.requestWindowDestruction();
	}

	WindowPhase Window::getPhase() const noexcept
	{
		return _impl->getPhase();
	}

	const VideoMode& Window::getVideoMode() const noexcept
	{
		return _impl->getVideoMode();
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

	void Window::addListener(IWindowListener& listener) noexcept
	{
		_impl->addListener(listener);
	}

	bool Window::removeListener(IWindowListener& listener) noexcept
	{
		return _impl->removeListener(listener);
	}
}