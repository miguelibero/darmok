#pragma once

#include <darmok/optional_ref.hpp>
#include <vector>
#include <optional>
#include <unordered_map>
#include <filesystem>
#include <sol/sol.hpp>
#include "glm.hpp"
#include "asset.hpp"
#include "window.hpp"
#include "input.hpp"

namespace bx
{
	class CommandLine;
}

namespace darmok
{
	class App;
	class LuaAssets;
	class LuaWindow;
	class LuaInput;
	class LuaScene;
	class LuaTexture;

	class LuaApp final
	{
	public:
		LuaApp(App& app) noexcept;
		App& getReal() noexcept;
		const App& getReal() const noexcept;

		LuaAssets& getAssets() noexcept;
		LuaWindow& getWindow() noexcept;
		LuaInput& getInput() noexcept;

		void registerUpdate(const sol::protected_function& func) noexcept;
		bool unregisterUpdate(const sol::protected_function& func) noexcept;
		void update(float deltaTime) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::vector<sol::protected_function> _updates;
		OptionalRef<App> _app;
		LuaAssets _assets;
		LuaWindow _window;
		LuaInput _input;
	};

	class LuaError final : std::runtime_error
	{
	public:
		LuaError(const std::string& msg, const sol::error& error);
	private:
		sol::error error;
	};

	class LuaRunnerAppImpl final
    {
    public:
		LuaRunnerAppImpl(App& app) noexcept;
        std::optional<int32_t> setup(const std::vector<std::string>& args);
		void init();
        void updateLogic(float deltaTime);
		void beforeShutdown() noexcept;
        void afterShutdown() noexcept;

    private:
		static std::string _defaultAssetInputPath;
		static std::string _defaultAssetOutputPath;
		static std::string _defaultAssetCachePath;
		App& _app;
		std::optional<LuaApp> _luaApp;
        std::unique_ptr<sol::state> _lua;

		std::optional<std::filesystem::path> findMainLua(const std::string& cmdName, const bx::CommandLine& cmdLine) noexcept;
		bool importAssets(const std::string& cmdName, const bx::CommandLine& cmdLine);
		std::optional<int32_t> loadLua(const std::filesystem::path& mainPath);

		void addPackagePath(const std::string& path, bool binary = false) noexcept;

		void version(const std::string& name) noexcept;
		void help(const std::string& name, const char* error = nullptr) noexcept;
    };
}