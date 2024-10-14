#pragma once

#include "lua.hpp"
#include "glm.hpp"
#include "coroutine.hpp"
#include "utils.hpp"

#include <darmok/optional_ref.hpp>
#include <darmok/app.hpp>

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
	class LuaAppUpdater final : public ITypeAppUpdater<LuaAppUpdater>
	{
	public:
		LuaAppUpdater(const sol::object& obj) noexcept;		
		const LuaDelegate& getDelegate() const noexcept;

		void update(float deltaTime) override;
	private:
		LuaDelegate _delegate;
	};

	class LuaAppUpdaterFilter final : public IAppUpdaterFilter
	{
	public:
		LuaAppUpdaterFilter(const sol::object& obj) noexcept;
		bool operator()(const IAppUpdater& updater) const noexcept override;
	private:
		sol::object _object;
		entt::id_type _type;
	};

	class LuaApp final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
		static App& addUpdater(App& app, const sol::object& updater) noexcept;

	private:

		static bool removeUpdater(App& app, const sol::object& updater) noexcept;

		static bool removeComponent(App& app, const sol::object& type);
		static bool hasComponent(const App& app, const sol::object& type);
		static void addLuaComponent(App& app, const sol::table& table);
		static sol::object getLuaComponent(App& app, const sol::object& type) noexcept;

		static LuaCoroutine startCoroutine(App& app, const sol::function& func) noexcept;
		static bool stopCoroutine(App& app, const LuaCoroutine& coroutine) noexcept;

		static bool getDebug() noexcept;

		static void setDebugFlag(App& app, uint32_t flag) noexcept;
		static void setResetFlag(App& app, uint32_t flag) noexcept;
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
		void earlyShutdown();
		void shutdown() noexcept;
		void render() noexcept;

    private:
		static std::string _defaultAssetInputPath;
		static std::string _defaultAssetOutputPath;
		static std::string _defaultAssetCachePath;
		App& _app;
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

	class LuaAppComponent final : public ITypeAppComponent<LuaAppComponent>
	{
	public:
		LuaAppComponent(const sol::table& table) noexcept;
		sol::object getReal() const noexcept;

		void init(App& app) override;
		void shutdown() override;
		void renderReset() override;
		void update(float deltaTime) override;

	private:
		sol::main_table _table;

		static const LuaTableDelegateDefinition _initDef;
		static const LuaTableDelegateDefinition _shutdownDef;
		static const LuaTableDelegateDefinition _renderResetDef;
		static const LuaTableDelegateDefinition _updateDef;
	};
}