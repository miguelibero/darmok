#pragma once

#include <darmok/export.h>
#include <memory>
#include <string_view>
#include <darmok/glm.hpp>
#include <darmok/color.hpp>
#include <darmok/viewport.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <darmok/window_fwd.hpp>

namespace darmok
{
	class Platform;

	struct DARMOK_EXPORT VideoMode final
	{
		WindowScreenMode screenMode = WindowScreenMode::Normal;
		glm::uvec2 size = glm::uvec2(0);
		Color3 depth = Color3(0);
		uint16_t refreshRate = 0;
		int monitor = -1;

		std::string toShortString() const noexcept;
		std::string to_string() const noexcept;

		bool operator==(const VideoMode& other) const noexcept;
		bool operator!=(const VideoMode& other) const noexcept;
	};

	struct DARMOK_EXPORT MonitorInfo final
	{
		std::string name;
		Viewport workarea;
	};

	struct DARMOK_EXPORT VideoModeInfo final
	{
		std::vector<VideoMode> modes;
		std::vector<MonitorInfo> monitors;
	};


	class DARMOK_EXPORT BX_NO_VTABLE IWindowListener
	{
	public:
		virtual ~IWindowListener() = default;
		virtual void onWindowSize(const glm::uvec2& size) {};
		virtual void onWindowPixelSize(const glm::uvec2& size) {};
		virtual void onWindowPhase(WindowPhase phase) {};
		virtual void onWindowVideoMode(const VideoMode& phase) {};
		virtual void onWindowCursorMode(WindowCursorMode phase) {};
		virtual void onWindowSupportedVideoModes(const VideoModeInfo& info) {};
		virtual void onWindowError(const std::string& error) {};
	};

	class WindowImpl;

	class DARMOK_EXPORT Window final
	{
	public:
		Window(Platform& plat) noexcept;
		~Window() noexcept;
		Window(const Window& other) = delete;
		Window(Window&& other) = delete;

		void requestSupportedVideoModes() noexcept;
		void requestVideoMode(const VideoMode& mode) noexcept;
		void requestCursorMode(WindowCursorMode mode) noexcept;
		void requestDestruction() noexcept;

		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] const glm::uvec2& getPixelSize() const noexcept;
		[[nodiscard]] WindowPhase getPhase() const noexcept;
		[[nodiscard]] const VideoMode& getVideoMode() const noexcept;
		[[nodiscard]] WindowCursorMode getCursorMode() const noexcept;

		[[nodiscard]] glm::vec2 windowToScreenPoint(const glm::vec2& point) const noexcept;
		[[nodiscard]] glm::vec2 screenToWindowPoint(const glm::vec2& point) const noexcept;

		[[nodiscard]] const WindowImpl& getImpl() const noexcept;
		[[nodiscard]] WindowImpl& getImpl() noexcept;

		void addListener(IWindowListener& listener) noexcept;
		bool removeListener(IWindowListener& listener) noexcept;

	private:
		std::unique_ptr<WindowImpl> _impl;
		Platform& _plat;
	};

} // namespace darmok