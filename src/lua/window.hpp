#pragma once

#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <sol/sol.hpp>
#include "glm.hpp"

namespace darmok
{
	enum class LuaWindowListenerType
	{
		Size,
		PixelSize,
		Phase,
		VideoMode,
		CursorMode,
	};

	class LuaWindow final : public IWindowListener
	{
	public:
		LuaWindow(Window& win) noexcept;
		~LuaWindow() noexcept;

		void onWindowSize(const glm::uvec2& size) override;
		void onWindowPixelSize(const glm::uvec2& size) override;
		void onWindowPhase(WindowPhase phase) override;
		void onWindowVideoMode(const VideoMode& mode) override;
		void onWindowCursorMode(WindowCursorMode mode) override;

		static void bind(sol::state_view& lua) noexcept;
	private:
		using ListenerType = LuaWindowListenerType;

		std::reference_wrapper<Window> _win;
		std::unordered_map<ListenerType, std::vector<sol::protected_function>> _listeners;

		void addListener(ListenerType type, const sol::protected_function& func) noexcept;
		bool removeListener(ListenerType type, const sol::protected_function& func) noexcept;

		const glm::uvec2& getSize() const noexcept;
		const glm::uvec2& getPixelSize() const noexcept;
		glm::vec2 getFramebufferScale() const noexcept;
		const VideoMode& getVideoMode() const noexcept;
		void setVideoMode(const VideoMode& mode) noexcept;
		const VideoModeInfo& getVideoModeInfo() noexcept;
		WindowCursorMode getCursorMode() const noexcept;
		void setCursorMode(WindowCursorMode mode) const noexcept;

		glm::vec2 screenToWindowPoint(const VarLuaTable<glm::vec2>& point) const noexcept;
		glm::vec2 windowToScreenPoint(const VarLuaTable<glm::vec2>& point) const noexcept;
	};
}