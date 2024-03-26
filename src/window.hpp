#pragma once

#include <darmok/window.hpp>

namespace darmok
{
	class Platform;

	class WindowImpl final
	{
	public:
		WindowImpl() noexcept;
		WindowImpl(const WindowImpl& other) = delete;
		WindowImpl(WindowImpl&& other) = delete;

		void setSize(const glm::uvec2& size) noexcept;
		void setPhase(WindowPhase phase) noexcept;
		void setMode(WindowMode mode) noexcept;
		
		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] WindowMode getMode() const noexcept;
		[[nodiscard]] WindowPhase getPhase() const noexcept;

	private:
		glm::uvec2 _size;
		WindowPhase _phase;
		WindowMode _mode;
	};

} // namespace darmok