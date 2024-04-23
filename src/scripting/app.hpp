#pragma once

#include <darmok/optional_ref.hpp>
#include "scene.hpp"
#include <vector>
#include <optional>
#include "sol.hpp"

namespace darmok
{
	class App;
	class LuaAssets;
	class LuaWindow;
	class LuaInput;

	class LuaApp final
	{
	public:
		LuaApp(App& app) noexcept;
		LuaScene getScene() noexcept;
		LuaAssets getAssets() noexcept;
		LuaWindow getWindow() noexcept;
		LuaInput getInput() noexcept;
		void registerUpdate(const sol::protected_function& func) noexcept;
		bool unregisterUpdate(const sol::protected_function& func) noexcept;
		void update(float deltaTime) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::vector<sol::protected_function> _updates;
		OptionalRef<App> _app;
		OptionalRef<Scene> _scene;
	};

	class ScriptingAppImpl final
    {
    public:
        void init(App& app, const std::vector<std::string>& args);
        void updateLogic(float deltaTime);
        void shutdown() noexcept;

    private:
        std::unique_ptr<sol::state> _lua;
		std::optional<LuaApp> _luaApp;
    };
}