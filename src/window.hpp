#pragma once

#include <unordered_set>
#include <darmok/window.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
	class Platform;

	class WindowImpl final
	{
	public:
		WindowImpl() noexcept;
		WindowImpl(const WindowImpl& other) = delete;
		WindowImpl(WindowImpl&& other) = delete;

		bool setSize(const glm::uvec2& size);
		bool setPixelSize(const glm::uvec2& size);
		bool setPhase(WindowPhase phase);
		bool setVideoMode(const VideoMode& mode);
		bool setCursorMode(WindowCursorMode mode);
		void onSupportedVideoModes(const VideoModeInfo& info);
		void onError(const std::string& error);

		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] const glm::uvec2& getPixelSize() const noexcept;
		[[nodiscard]] const VideoMode& getVideoMode() const noexcept;
		[[nodiscard]] WindowCursorMode getCursorMode() const noexcept;
		[[nodiscard]] WindowPhase getPhase() const noexcept;

		[[nodiscard]] glm::vec2 screenToWindowPoint(const glm::vec2& point) const noexcept;
		[[nodiscard]] glm::vec2 windowToScreenPoint(const glm::vec2& point) const noexcept;

		void addListener(IWindowListener& listener) noexcept;
		bool removeListener(IWindowListener& listener) noexcept;

	private:
		glm::uvec2 _size;
		glm::uvec2 _pixelSize;
		WindowPhase _phase;
		VideoMode _videoMode;
		WindowCursorMode _cursorMode;
		std::unordered_set<OptionalRef<IWindowListener>> _listeners;

		glm::vec2 getScreenToWindowFactor() const noexcept;
	};

} // namespace darmok