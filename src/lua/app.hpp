#pragma once

#include "lua.hpp"
#include "glm.hpp"
#include "coroutine.hpp"
#include "utils.hpp"

#include <darmok/optional_ref.hpp>

#include <vector>
#include <optional>
#include <unordered_map>
#include <filesystem>
#include <stdexcept>

namespace bx
{
	class CommandLine;
}

namespace darmok
{
	class App;
	class Window;
	class AudioSystem;
	class AssetContext;

	// TODO: make this class static, bind darmok::App directly
	class LuaApp final
	{
	public:
		LuaApp(App& app) noexcept;
		App& getReal() noexcept;
		const App& getReal() const noexcept;

		void quit() noexcept;

		bool toggleDebugFlag(uint32_t flag) noexcept;
		bool getDebugFlag(uint32_t flag) const noexcept;
		void setDebugFlag1(uint32_t flag) noexcept;
		void setDebugFlag2(uint32_t flag, bool enabled) noexcept;

		bool toggleResetFlag(uint32_t flag) noexcept;
		bool getResetFlag(uint32_t flag) const noexcept;
		void setResetFlag1(uint32_t flag) noexcept;
		void setResetFlag2(uint32_t flag, bool enabled) noexcept;

		void setRendererType(bgfx::RendererType::Enum renderer);

		void update(float deltaTime, sol::state_view& lua) noexcept;

		LuaApp& addUpdater(const sol::object& updater) noexcept;
		bool removeUpdater(const sol::object& updater) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::vector<LuaDelegate> _updaters;
		LuaCoroutineRunner _coroutineRunner;
		std::reference_wrapper<App> _app;

		AssetContext& getAssets() noexcept;
		Window& getWindow() noexcept;
		Input& getInput() noexcept;
		AudioSystem& getAudio() noexcept;

		bool removeComponent(const sol::object& type);
		bool hasComponent(const sol::object& type) const;
		void addLuaComponent(const sol::table& table);
		sol::object getLuaComponent(const sol::object& type) noexcept;

		LuaCoroutine startCoroutine(const sol::function& func) noexcept;
		bool stopCoroutine(const LuaCoroutine& coroutine) noexcept;

		void updateUpdaters(float deltaTime) noexcept;
		bool removeUpdater3(const LuaDelegate& dlg) noexcept;

		static bool getDebug() noexcept;

		bool getPaused() const noexcept;
		void setPaused(bool paused) noexcept;
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
		void earlyShutdown();
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

	class LuaAppComponent final : public IAppComponent
	{
	public:
		LuaAppComponent(const sol::table& table, LuaApp& app) noexcept;
		entt::id_type getType() const noexcept;
		sol::object getReal() const noexcept;

		void init(App& app) override;
		void shutdown() override;
		void renderReset() override;
		void update(float deltaTime) override;

	private:
		sol::main_table _table;
		LuaApp& _app;

		static const LuaTableDelegateDefinition _initDef;
		static const LuaTableDelegateDefinition _shutdownDef;
		static const LuaTableDelegateDefinition _renderResetDef;
		static const LuaTableDelegateDefinition _updateDef;
	};
}