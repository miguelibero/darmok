#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/color.hpp>
#include <darmok/viewport.hpp>
#include <darmok/window_fwd.hpp>

#include <entt/entt.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

#include <memory>
#include <string>


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
		std::string toString() const noexcept;
		bool complete() const noexcept;

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
		virtual entt::id_type getType() const { return 0; }
		virtual void onWindowSize(const glm::uvec2& size) {};
		virtual void onWindowPixelSize(const glm::uvec2& size) {};
		virtual void onWindowPhase(WindowPhase phase) {};
		virtual void onWindowVideoMode(const VideoMode& mode) {};
		virtual void onWindowCursorMode(WindowCursorMode mode) {};
		virtual void onWindowVideoModeInfo(const VideoModeInfo& info) {};
		virtual void onWindowError(const std::string& error) {};
	};

	class DARMOK_EXPORT BX_NO_VTABLE IWindowListenerFilter
	{
	public:
		virtual ~IWindowListenerFilter() = default;
		virtual bool operator()(const IWindowListener& listener) const = 0;
	};

	class WindowImpl;

	class DARMOK_EXPORT Window final
	{
	public:
		Window(Platform& plat) noexcept;
		~Window() noexcept;
		Window(const Window& other) = delete;
		Window(Window&& other) = delete;

		void requestVideoMode(const VideoMode& mode) noexcept;
		void requestCursorMode(WindowCursorMode mode) noexcept;
		void requestDestruction() noexcept;
		void requestVideoModeInfo() noexcept;

		[[nodiscard]] float getAspect() const noexcept;
		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] const glm::uvec2& getPixelSize() const noexcept;
		[[nodiscard]] WindowPhase getPhase() const noexcept;
		[[nodiscard]] WindowCursorMode getCursorMode() const noexcept;
		[[nodiscard]] const VideoMode& getVideoMode() const noexcept;
		[[nodiscard]] const VideoModeInfo& getVideoModeInfo() const noexcept;

		[[nodiscard]] glm::vec2 windowToScreenPoint(const glm::vec2& point) const noexcept;
		[[nodiscard]] glm::vec2 screenToWindowPoint(const glm::vec2& point) const noexcept;
		[[nodiscard]] glm::vec2 windowToScreenDelta(const glm::vec2& delta) const noexcept;
		[[nodiscard]] glm::vec2 screenToWindowDelta(const glm::vec2& delta) const noexcept;
		[[nodiscard]] glm::vec2 getFramebufferScale() const noexcept;

		[[nodiscard]] const WindowImpl& getImpl() const noexcept;
		[[nodiscard]] WindowImpl& getImpl() noexcept;

		void addListener(std::unique_ptr<IWindowListener>&& listener) noexcept;
		void addListener(IWindowListener& listener) noexcept;
		bool removeListener(const IWindowListener& listener) noexcept;
		size_t removeListeners(const IWindowListenerFilter& filter) noexcept;

	private:
		std::unique_ptr<WindowImpl> _impl;
	};

} // namespace darmok