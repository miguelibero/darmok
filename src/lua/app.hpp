#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/app.hpp>
#include <vector>
#include <optional>
#include <unordered_map>
#include <filesystem>
#include <sol/sol.hpp>
#include "glm.hpp"
#include "asset.hpp"
#include "window.hpp"
#include "input.hpp"
#include "audio.hpp"
#include "coroutine.hpp"

namespace bx
{
	class CommandLine;
}

namespace darmok
{
	class AssetContext;
	class LuaAppComponentContainer;	

	class LuaApp final
	{
	public:
		LuaApp(App& app) noexcept;
		App& getReal() noexcept;
		const App& getReal() const noexcept;

		bool toggleDebugFlag(uint32_t flag) noexcept;
		bool getDebugFlag(uint32_t flag) const noexcept;
		void setDebugFlag1(uint32_t flag) noexcept;
		void setDebugFlag2(uint32_t flag, bool enabled) noexcept;

		bool toggleResetFlag(uint32_t flag) noexcept;
		bool getResetFlag(uint32_t flag) const noexcept;
		void setResetFlag1(uint32_t flag) noexcept;
		void setResetFlag2(uint32_t flag, bool enabled) noexcept;

		void update(float deltaTime, sol::state_view& lua) noexcept;

		template<typename T, typename R, typename... A>
		T& addComponent(A&&... args)
		{
			auto& real = getReal().addComponent<R>(std::forward<A>(args)...);
			return getReal().addComponent<T>(real);
		}

		LuaApp& addUpdater1(const sol::protected_function& func) noexcept;
		LuaApp& addUpdater2(const sol::table& table) noexcept;
		bool removeUpdater1(const sol::protected_function& func) noexcept;
		bool removeUpdater2(const sol::table& table) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::vector<sol::protected_function> _updaterFunctions;
		std::vector<sol::table> _updaterTables;
		LuaCoroutineRunner _coroutineRunner;
		std::reference_wrapper<App> _app;
		LuaInput _input;
		LuaWindow _win;
		LuaAudioSystem _audio;
		OptionalRef<LuaAppComponentContainer> _luaComponents;

		AssetContext& getAssets() noexcept;
		LuaWindow& getWindow() noexcept;
		LuaInput& getInput() noexcept;
		LuaAudioSystem& getAudio() noexcept;

		bool removeComponent(const sol::object& type);
		bool hasComponent(const sol::object& type) const;
		bool hasLuaComponent(const sol::object& type) const noexcept;
		void addLuaComponent(const sol::table& comp);
		bool removeLuaComponent(const sol::object& type) noexcept;
		sol::object getLuaComponent(const sol::object& type) noexcept;

		LuaCoroutineThread startCoroutine(const sol::function& func, sol::this_state ts) noexcept;
		bool stopCoroutine(const LuaCoroutineThread& thread) noexcept;

		void updateUpdaters(float deltaTime) noexcept;

		static bool getDebug() noexcept;
	};

	class LuaError final : std::exception
	{
	public:
		LuaError(const std::string& msg, const sol::error& error);
		const char* what() const noexcept override;
	private:
		sol::error error;
		std::string _msg;
	};

	class LuaAppDelegateImpl final
    {
    public:
		LuaAppDelegateImpl(App& app) noexcept;
        std::optional<int32_t> setup(const std::vector<std::string>& args);
		void init();
        void update(float deltaTime);
		void shutdown() noexcept;
		void render() noexcept;

    private:
		static std::string _defaultAssetInputPath;
		static std::string _defaultAssetOutputPath;
		static std::string _defaultAssetCachePath;
		App& _app;
		std::optional<LuaApp> _luaApp;
        std::unique_ptr<sol::state> _lua;
		std::filesystem::path _mainLuaPath;

		struct DbgText final
		{
			glm::uvec2 pos;
			std::string message;
		};

		std::vector<DbgText> _dbgTexts;

		std::optional<std::filesystem::path> findMainLua(const std::string& cmdName, const bx::CommandLine& cmdLine) noexcept;
		bool importAssets(const std::string& cmdName, const bx::CommandLine& cmdLine);
		std::optional<int32_t> loadLua(const std::filesystem::path& mainPath);
		void unloadLua() noexcept;

		void addPackagePath(const std::string& path, bool binary = false) noexcept;
		void luaDebugScreenText(const glm::uvec2& pos, const std::string& msg) noexcept;

		void version(const std::string& name) noexcept;
		void help(const std::string& name, const char* error = nullptr) noexcept;
    };
}