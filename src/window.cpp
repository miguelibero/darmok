#include "window.hpp"
#include "platform.hpp"

namespace darmok
{
	WindowImpl::WindowImpl() noexcept
		: _phase(WindowPhase::Unknown)
		, _mode(WindowMode::Normal)
		, _size(0, 0)
	{
	}

	void WindowImpl::setSize(const glm::uvec2& size) noexcept
	{
		_size = size;
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

	WindowPhase WindowImpl::getPhase() const noexcept
	{
		return _phase;
	}

	WindowMode WindowImpl::getMode() const noexcept
	{
		return _mode;
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

	void Window::requestMode(WindowMode mode) noexcept
	{
		_plat.requestWindowModeChange(mode);
	}

	void Window::requestMouseLock(bool enabled) noexcept
	{
		_plat.requestWindowMouseLock(enabled);
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

	void Window::bgfxConfig(bgfx::ViewId viewId) const noexcept
	{
		// set view default viewport
		const auto& size = getSize();
		bgfx::setViewRect(viewId, 0, 0, uint16_t(size.x), uint16_t(size.y));

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