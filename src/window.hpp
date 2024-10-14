#pragma once

#include <unordered_set>
#include <darmok/window.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/collection.hpp>

namespace darmok
{
	class Platform;

	class WindowImpl final
	{
	public:
		WindowImpl(Platform& plat) noexcept;
		WindowImpl(const WindowImpl& other) = delete;
		WindowImpl(WindowImpl&& other) = delete;

		void shutdown() noexcept;

		bool setSize(const glm::uvec2& size);
		bool setPixelSize(const glm::uvec2& size);
		bool setPhase(WindowPhase phase);
		bool setVideoMode(const VideoMode& mode);
		bool setCursorMode(WindowCursorMode mode);
		void setVideoModeInfo(const VideoModeInfo& info);
		void onError(const std::string& error);

		const glm::uvec2& getSize() const noexcept;
		const glm::uvec2& getPixelSize() const noexcept;
		const VideoMode& getVideoMode() const noexcept;
		const VideoModeInfo& getVideoModeInfo() const noexcept;
		WindowCursorMode getCursorMode() const noexcept;
		WindowPhase getPhase() const noexcept;

		void requestVideoModeInfo() noexcept;
		void requestVideoMode(const VideoMode& mode) noexcept;
		void requestCursorMode(WindowCursorMode mode) noexcept;
		void requestDestruction() noexcept;

		glm::vec2 screenToWindowPoint(const glm::vec2& point) const noexcept;
		glm::vec2 windowToScreenPoint(const glm::vec2& point) const noexcept;
		glm::vec2 windowToScreenDelta(const glm::vec2& delta) const noexcept;
		glm::vec2 screenToWindowDelta(const glm::vec2& delta) const noexcept;
		glm::vec2 getFramebufferScale() const noexcept;

		void addListener(std::unique_ptr<IWindowListener>&& listener) noexcept;
		void addListener(IWindowListener& listener) noexcept;
		bool removeListener(const IWindowListener& listener) noexcept;
		size_t removeListeners(const IWindowListenerFilter& filter) noexcept;

	private:
		glm::uvec2 _size;
		glm::uvec2 _pixelSize;
		WindowPhase _phase;
		VideoMode _videoMode;
		VideoModeInfo _videoModeInfo;
		WindowCursorMode _cursorMode;
		OwnRefCollection<IWindowListener> _listeners;
		Platform& _plat;
	};

} // namespace darmok