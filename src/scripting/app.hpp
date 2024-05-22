#pragma once

#include <darmok/optional_ref.hpp>
#include <vector>
#include <optional>
#include <unordered_map>
#include <sol/sol.hpp>
#include "glm.hpp"

namespace darmok
{
	class App;
	class LuaAssets;
	class LuaWindow;
	class LuaInput;
	class LuaScene;
	class LuaTexture;
	class SceneAppComponent;
	class RmluiAppComponent;
	class LuaRmluiAppComponent;

	class LuaApp final
	{
	public:
		LuaApp(App& app) noexcept;
		App& getReal() noexcept;
		const App& getReal() const noexcept;

		LuaScene getScene() noexcept;
		void setScene(const LuaScene& scene) noexcept;
		std::vector<LuaScene> getScenes() noexcept;
		bool addScene1(const LuaScene& scene) noexcept;
		LuaScene addScene2() noexcept;
		bool removeScene(const LuaScene& scene) noexcept;

		std::reference_wrapper<LuaRmluiAppComponent> getMainGui() noexcept;
		OptionalRef<LuaRmluiAppComponent>::std_t getGui(const std::string& name) noexcept;
		std::reference_wrapper<LuaRmluiAppComponent> getOrAddGui(const std::string& name) noexcept;
		std::reference_wrapper<LuaRmluiAppComponent> addGui(const std::string& name);
		bool removeGui(const std::string& name) noexcept;
		
		LuaAssets getAssets() noexcept;
		LuaWindow getWindow() noexcept;
		LuaInput getInput() noexcept;
		void registerUpdate(const sol::protected_function& func) noexcept;
		bool unregisterUpdate(const sol::protected_function& func) noexcept;
		void update(float deltaTime) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::vector<sol::protected_function> _updates;
		OptionalRef<App> _app;

		using SceneComponents = std::vector<OptionalRef<SceneAppComponent>>;
		SceneComponents _sceneComponents;
		SceneComponents::iterator findSceneComponent(const LuaScene& scene) noexcept;

		using RmluiComponents = std::unordered_map<std::string, LuaRmluiAppComponent>;
		RmluiComponents _guiComponents;
	};

	class ScriptingAppImpl final
    {
    public:
        void init(App& app, const std::vector<std::string>& args);
        void updateLogic(float deltaTime);
		void beforeShutdown() noexcept;
        void afterShutdown() noexcept;

    private:
        std::unique_ptr<sol::state> _lua;
		std::optional<LuaApp> _luaApp;
		static std::string findMainLua(const std::vector<std::string>& args) noexcept;

		void addPackagePath(const std::string& path) noexcept;
    };
}