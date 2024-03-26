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

	class Window final
	{
	public:
		Window(Platform& plat) noexcept;
		Window(const Window& other) = delete;
		Window(Window&& other) = delete;

		void requestMode(WindowMode mode) noexcept;
		void requestMouseLock(bool enabled) noexcept;
		void requestDestruction() noexcept;

		void bgfxConfig(bgfx::ViewId viewId) noexcept;

		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] WindowPhase getPhase() const noexcept;
		[[nodiscard]] WindowMode getMode() const noexcept;

		[[nodiscard]] const WindowImpl& getImpl() const noexcept;
		[[nodiscard]] WindowImpl& getImpl() noexcept;

	private:
		std::unique_ptr<WindowImpl> _impl;
		Platform& _plat;
	};

} // namespace darmok