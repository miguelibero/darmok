#pragma once

#include <darmok/export.h>
#include <memory>
#include <string_view>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <darmok/window_fwd.hpp>

namespace darmok
{
	class WindowImpl;
	class Platform;

	class DARMOK_EXPORT BX_NO_VTABLE IWindowListener
	{
	public:
		virtual ~IWindowListener() = default;
		virtual void onWindowSize(const glm::uvec2& size) {};
		virtual void onWindowPixelSize(const glm::uvec2& size) {};
		virtual void onWindowPhase(WindowPhase phase) {};
		virtual void onWindowMode(WindowMode phase) {};
	};

	class DARMOK_EXPORT Window final
	{
	public:
		Window(Platform& plat) noexcept;
		~Window() noexcept;
		Window(const Window& other) = delete;
		Window(Window&& other) = delete;

		void requestMode(WindowMode mode) noexcept;
		void requestCursorMode(WindowCursorMode mode) noexcept;
		void requestDestruction() noexcept;

		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] const glm::uvec2& getPixelSize() const noexcept;
		[[nodiscard]] WindowPhase getPhase() const noexcept;
		[[nodiscard]] WindowMode getMode() const noexcept;

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