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

		bool setSize(const glm::uvec2& size) noexcept;
		bool setPixelSize(const glm::uvec2& size) noexcept;
		bool setPhase(WindowPhase phase) noexcept;
		bool setMode(WindowMode mode) noexcept;
		
		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] const glm::uvec2& getPixelSize() const noexcept;
		[[nodiscard]] WindowMode getMode() const noexcept;
		[[nodiscard]] WindowPhase getPhase() const noexcept;

		[[nodiscard]] glm::vec2 screenToWindowPoint(const glm::vec2& point) const noexcept;
		[[nodiscard]] glm::vec2 windowToScreenPoint(const glm::vec2& point) const noexcept;

		void addListener(IWindowListener& listener) noexcept;
		bool removeListener(IWindowListener& listener) noexcept;

	private:
		glm::uvec2 _size;
		glm::uvec2 _pixelSize;
		WindowPhase _phase;
		WindowMode _mode;
		std::unordered_set<OptionalRef<IWindowListener>> _listeners;

		glm::vec2 getScreenToWindowFactor() const noexcept;
	};

} // namespace darmok