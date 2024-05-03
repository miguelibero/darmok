#include "window.hpp"
#include "platform.hpp"

namespace darmok
{
	WindowImpl::WindowImpl() noexcept
		: _phase(WindowPhase::Unknown)
		, _mode(WindowMode::Normal)
		, _size(0)
		, _pixelSize(0)
	{
	}

	bool WindowImpl::setSize(const glm::uvec2& size) noexcept
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

	bool WindowImpl::setPixelSize(const glm::uvec2& size) noexcept
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

	bool WindowImpl::setPhase(WindowPhase phase) noexcept
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

	bool WindowImpl::setMode(WindowMode mode) noexcept
	{
		if (_mode == mode)
		{
			return false;
		}
		for (auto& listener : _listeners)
		{
			listener->onWindowMode(mode);
		}
		_mode = mode;
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

	WindowMode WindowImpl::getMode() const noexcept
	{
		return _mode;
	}

	glm::uvec2 WindowImpl::screenPointToWindow(const glm::vec2& point) const noexcept
	{
		auto f = glm::vec2(_size) / glm::vec2(_pixelSize);
		auto p =  (point - glm::vec2(0.5F)) * f;
		p.y = _size.y - p.y;
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

	void Window::requestMode(WindowMode mode) noexcept
	{
		_plat.requestWindowModeChange(mode);
	}

	void Window::requestCursorMode(WindowCursorMode mode) noexcept
	{
		_plat.requestCursorModeChange(mode);
	}

	void Window::requestDestruction() noexcept
	{
		_plat.requestWindowDestruction();
	}

	WindowPhase Window::getPhase() const noexcept
	{
		return _impl->getPhase();
	}

	WindowMode Window::getMode() const noexcept
	{
		return _impl->getMode();
	}

	glm::uvec2 Window::screenPointToWindow(const glm::vec2& pos) const noexcept
	{
		return _impl->screenPointToWindow(pos);
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