#pragma once

#include <memory>
#include <string_view>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
	enum class WindowPhase
	{
		Unknown,
		Running,
		Suspended,
		Destroyed,
		Count,
	};

	enum class WindowMode
	{
		Normal,
		Fullscreen,
		WindowedFullscreen,
		Count,
	};

	class WindowImpl;
	class Platform;

	enum class WindowCursorMode
	{
		Normal,
		Hidden,
		Disabled,
	};

	class Window final
	{
	public:
		Window(Platform& plat) noexcept;
		Window(const Window& other) = delete;
		Window(Window&& other) = delete;

		void requestMode(WindowMode mode) noexcept;
		void requestCursorMode(WindowCursorMode mode) noexcept;
		void requestDestruction() noexcept;

		void bgfxConfig(bgfx::ViewId viewId) const noexcept;

		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] glm::uvec2 getPixelSize() const noexcept;
		[[nodiscard]] const glm::ivec4& getViewport() const noexcept;
		[[nodiscard]] WindowPhase getPhase() const noexcept;
		[[nodiscard]] WindowMode getMode() const noexcept;

		[[nodiscard]] glm::uvec2 screenPointToWindow(const glm::vec2& point) const noexcept;

		[[nodiscard]] const WindowImpl& getImpl() const noexcept;
		[[nodiscard]] WindowImpl& getImpl() noexcept;

	private:
		std::unique_ptr<WindowImpl> _impl;
		Platform& _plat;
	};

} // namespace darmok