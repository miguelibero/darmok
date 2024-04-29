#pragma once

#include <memory>
#include <string_view>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>
#include <darmok/window_fwd.hpp>

namespace darmok
{
	class WindowImpl;
	class Platform;

	class Window final
	{
	public:
		Window(Platform& plat) noexcept;
		Window(const Window& other) = delete;
		Window(Window&& other) = delete;

		void requestMode(WindowMode mode) noexcept;
		void requestCursorMode(WindowCursorMode mode) noexcept;
		void requestDestruction() noexcept;

		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] const glm::uvec2& getPixelSize() const noexcept;
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