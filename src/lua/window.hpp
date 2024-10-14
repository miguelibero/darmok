#pragma once

#include "lua.hpp"
#include "glm.hpp"
#include "utils.hpp"

#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>

namespace darmok
{
	class LuaWindowListener : public IWindowListener
	{
	public:
		LuaWindowListener(const sol::table& table) noexcept;
		void onWindowSize(const glm::uvec2& size) override;
		void onWindowPixelSize(const glm::uvec2& size) override;
		void onWindowPhase(WindowPhase phase) override;
		void onWindowVideoMode(const VideoMode& mode) override;
		void onWindowCursorMode(WindowCursorMode mode) override;
		sol::object getReal() const noexcept;
	private:
		sol::main_table _table;
		static const LuaTableDelegateDefinition _sizeDelegate;
		static const LuaTableDelegateDefinition _pixelSizeDelegate;
		static const LuaTableDelegateDefinition _phaseDelegate;
		static const LuaTableDelegateDefinition _videoModeDelegate;
		static const LuaTableDelegateDefinition _cursorModeDelegate;
	};

	class LuaWindowListenerFilter final : public IWindowListenerFilter
	{
	public:
		LuaWindowListenerFilter(const sol::table& table) noexcept;
		bool operator()(const IWindowListener& listener, entt::id_type type) const noexcept override;
	private:
		sol::main_table _table;
		entt::id_type _type;
	};

	class LuaWindow final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static void addListener(Window& win, const sol::table& table) noexcept;
		static bool removeListener(Window& win, const sol::table& table) noexcept;

		static glm::vec2 screenToWindowPoint(const Window& win, const VarLuaTable<glm::vec2>& point) noexcept;
		static glm::vec2 windowToScreenPoint(const Window& win, const VarLuaTable<glm::vec2>& point) noexcept;
	};
}