#include "detail/window.hpp"
#include "detail/platform.hpp"
#include <darmok/string.hpp>
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
		std::ostringstream out;
		out << size.x << "x" << size.y;
		if (refreshRate > 0)
		{
			out << " " << refreshRate << "Hz";
		}
		if (depth.r > 0 || depth.g > 0 || depth.b > 0)
		{
			out << " ";
			if (depth.r == depth.g && depth.r == depth.b)
			{
				out << (unsigned int)depth.r;
			}
			else
			{
				out << (unsigned int)depth.r << "-" << (unsigned int)depth.g << "-" << (unsigned int)depth.b;
			}
			out << "bpp";
		}
		return out.str();
	}

	std::string VideoMode::toString() const noexcept
	{
		std::ostringstream out;
		switch (screenMode)
		{
		case WindowScreenMode::Normal:
			out << "Windowed ";
			break;
		case WindowScreenMode::Fullscreen:
			out << "Fullscreen ";
			break;
		case WindowScreenMode::WindowedFullscreen:
			out << "Borderless Windowed ";
			break;
		default:
			break;
		}
		out << toShortString();
		if (monitor >= 0)
		{
			out << " monitor " << monitor;
		}
		return out.str();
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

	expected<void, std::string> WindowImpl::setSize(const glm::uvec2& size) noexcept
	{
		if (_size == size)
		{
			return {};
		}
		_size = size;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onWindowSize(size);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> WindowImpl::setPixelSize(const glm::uvec2& size) noexcept
	{
		if (_pixelSize == size)
		{
			return {};
		}
		_pixelSize = size;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onWindowPixelSize(size);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> WindowImpl::setPhase(WindowPhase phase) noexcept
	{
		if (_phase == phase)
		{
			return {};
		}
		_phase = phase;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onWindowPhase(phase);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> WindowImpl::setVideoMode(const VideoMode& mode) noexcept
	{
		if (_videoMode == mode)
		{
			return {};
		}
		_videoMode = mode;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onWindowVideoMode(mode);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> WindowImpl::setVideoModeInfo(const VideoModeInfo& info) noexcept
	{
		_videoModeInfo = info;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onWindowVideoModeInfo(info);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> WindowImpl::setTitle(const std::string& title) noexcept
	{
		_title = title;
		return {};
	}

	expected<void, std::string> WindowImpl::onError(const std::string& error) noexcept
	{
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onWindowError(error);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> WindowImpl::setCursorMode(WindowCursorMode mode) noexcept
	{
		if (_cursorMode == mode)
		{
			return {};
		}
		_cursorMode = mode;
		std::vector<std::string> errors;
		for (auto& listener : _listeners)
		{
			auto result = listener.onWindowCursorMode(mode);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
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

	void WindowImpl::openFileDialog(const FileDialogOptions& options, const FileDialogCallback& callback) noexcept
	{
		_plat.openFileDialog(options, callback);
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
		auto scale = getFramebufferScale();
		auto winPoint = (point - glm::vec2(0.5F)) / scale;
		return winPoint;
	}

	glm::vec2 WindowImpl::windowToScreenPoint(const glm::vec2& point) const noexcept
	{
		glm::vec2 newPoint(point);
		auto scale = getFramebufferScale();
		newPoint = (newPoint * scale) + glm::vec2(0.5F);
		return newPoint;
	}

	void WindowImpl::addListener(std::unique_ptr<IWindowListener> listener) noexcept
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
		const auto& size = getPixelSize();
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

	void Window::requestTitle(std::string title) noexcept
	{
		_impl->requestTitle(std::move(title));
	}

	void Window::openFileDialog(FileDialogOptions options, FileDialogCallback callback) noexcept
	{
		_impl->openFileDialog(std::move(options), std::move(callback));
	}

	void Window::requestVideoModeInfo() noexcept
	{
		_impl->requestVideoModeInfo();
	}

	void Window::requestVideoMode(VideoMode mode) noexcept
	{
		_impl->requestVideoMode(std::move(mode));
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

	glm::vec2 Window::windowToScreenPoint(const glm::vec2& point) const noexcept
	{
		return _impl->windowToScreenPoint(point);
	}

	glm::vec2 Window::screenToWindowPoint(const glm::vec2& point) const noexcept
	{
		return _impl->screenToWindowPoint(point);
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

	void Window::addListener(std::unique_ptr<IWindowListener> listener) noexcept
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