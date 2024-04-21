#include "window.hpp"
#include "platform.hpp"

namespace darmok
{
	WindowImpl::WindowImpl() noexcept
		: _phase(WindowPhase::Unknown)
		, _mode(WindowMode::Normal)
		, _size(0)
		, _viewport(0)
	{
	}

	void WindowImpl::setSize(const glm::uvec2& size) noexcept
	{
		_size = size;
	}

	void WindowImpl::setPixelSize(const glm::uvec2& size) noexcept
	{
		_viewport[2] = size.x;
		_viewport[3] = size.y;
	}

	void WindowImpl::setPhase(WindowPhase phase) noexcept
	{
		_phase = phase;
	}

	void WindowImpl::setMode(WindowMode mode) noexcept
	{
		_mode = mode;
	}

	const glm::uvec2& WindowImpl::getSize() const noexcept
	{
		return _size;
	}

	glm::uvec2 WindowImpl::getPixelSize() const noexcept
	{
		return glm::uvec2(_viewport[2], _viewport[3]);
	}

	const glm::ivec4& WindowImpl::getViewport() const noexcept
	{
		return _viewport;
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
		auto f = glm::vec2(_size) / glm::vec2(_viewport[2], _viewport[3]);
		auto p =  (point - glm::vec2(0.5F)) * f;
		p.y = _size.y - p.y;
		return p;
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

	glm::uvec2 Window::getPixelSize() const noexcept
	{
		return _impl->getPixelSize();
	}

	const glm::ivec4& Window::getViewport() const noexcept
	{
		return _impl->getViewport();
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

	void Window::bgfxConfig(bgfx::ViewId viewId) const noexcept
	{
		// set view default viewport
		auto& vp = getViewport();
		bgfx::setViewRect(viewId, vp[0], vp[1], vp[2], vp[3]);

		// this dummy draw call is here to make sure that view is cleared
		// if no other draw calls are submitted to view.
		bgfx::touch(viewId);

		// use debug font to print information about this example.
		bgfx::dbgTextClear();

		// clear the view
		if (viewId == 0)
		{
			bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH | BGFX_CLEAR_COLOR, 1.F, 0U, 1);
		}
		else
		{
			bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH);
		}
	}
}