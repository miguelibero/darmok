#pragma once

#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <sol/sol.hpp>
#include "glm.hpp"

namespace darmok
{
	class LuaWindow final
	{
	public:
		LuaWindow(Window& win) noexcept;
		static void bind(sol::state_view& lua) noexcept;
	private:
		std::reference_wrapper<Window> _win;

		const glm::uvec2& getSize() const noexcept;
		const glm::uvec2& getPixelSize() const noexcept;
		const VideoMode& getVideoMode() const noexcept;
		void setVideoMode(const VideoMode& mode) noexcept;
		const VideoModeInfo& getVideoModeInfo() noexcept;
		WindowCursorMode getCursorMode() const noexcept;
		void setCursorMode(WindowCursorMode mode) const noexcept;

		glm::vec2 screenToWindowPoint(const Window& win, const VarLuaTable<glm::vec2>& point) const noexcept;
		glm::vec2 windowToScreenPoint(const Window& win, const VarLuaTable<glm::vec2>& point) const noexcept;
	};
}