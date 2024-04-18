#pragma once

#include <darmok/optional_ref.hpp>
#include "scene.hpp"
#include <vector>
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

		static void configure(sol::state_view& lua) noexcept;
	private:
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
        sol::protected_function _luaUpdate;
    };
}