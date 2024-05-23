#pragma once

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

	class BX_NO_VTABLE IWindowListener
	{
	public:
		DLLEXPORT virtual ~IWindowListener() = default;
		DLLEXPORT virtual void onWindowSize(const glm::uvec2& size) {};
		DLLEXPORT virtual void onWindowPixelSize(const glm::uvec2& size) {};
		DLLEXPORT virtual void onWindowPhase(WindowPhase phase) {};
		DLLEXPORT virtual void onWindowMode(WindowMode phase) {};
	};

	class Window final
	{
	public:
		DLLEXPORT Window(Platform& plat) noexcept;
		DLLEXPORT Window(const Window& other) = delete;
		DLLEXPORT Window(Window&& other) = delete;

		DLLEXPORT void requestMode(WindowMode mode) noexcept;
		DLLEXPORT void requestCursorMode(WindowCursorMode mode) noexcept;
		DLLEXPORT void requestDestruction() noexcept;

		[[nodiscard]] DLLEXPORT const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] DLLEXPORT const glm::uvec2& getPixelSize() const noexcept;
		[[nodiscard]] DLLEXPORT WindowPhase getPhase() const noexcept;
		[[nodiscard]] DLLEXPORT WindowMode getMode() const noexcept;

		[[nodiscard]] DLLEXPORT glm::vec2 windowToScreenPoint(const glm::vec2& point) const noexcept;
		[[nodiscard]] DLLEXPORT glm::vec2 screenToWindowPoint(const glm::vec2& point) const noexcept;

		[[nodiscard]] const WindowImpl& getImpl() const noexcept;
		[[nodiscard]] WindowImpl& getImpl() noexcept;

		DLLEXPORT void addListener(IWindowListener& listener) noexcept;
		DLLEXPORT bool removeListener(IWindowListener& listener) noexcept;

	private:
		std::unique_ptr<WindowImpl> _impl;
		Platform& _plat;
	};

} // namespace darmok