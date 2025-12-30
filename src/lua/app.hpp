#pragma once

#include "lua/lua.hpp"
#include "lua/glm.hpp"
#include "lua/coroutine.hpp"
#include "lua/utils.hpp"

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
	sol::state createLuaState(App& app) noexcept;

	class LuaAppUpdater final : public ITypeAppUpdater<LuaAppUpdater>
	{
	public:
		LuaAppUpdater(const sol::object& obj) noexcept;		
		const LuaDelegate& getDelegate() const noexcept;

		expected<void, std::string> update(float deltaTime) noexcept override;
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

		static LuaCoroutine startCoroutine(App& app, const sol::function& func);
		static bool stopCoroutine(App& app, const LuaCoroutine& coroutine) noexcept;

		static bool getDebug() noexcept;

		static void setDebugFlag(App& app, uint32_t flag) noexcept;
		static void setResetFlag(App& app, uint32_t flag) noexcept;
	};

	class LuaError final : std::exception
	{
	public:
		LuaError(std::string_view msg, const sol::error& error);
		const char* what() const noexcept override;
	private:
		sol::error error;
		std::string _msg;
	};

	class LuaAppDelegateImpl final
    {
    public:
		LuaAppDelegateImpl(App& app) noexcept;
		expected<int32_t, std::string> setup(const CmdArgs& args) noexcept;
		expected<void, std::string> init() noexcept;
		expected<void, std::string> earlyShutdown()noexcept;
		expected<void, std::string> shutdown() noexcept;
		expected<void, std::string> render() noexcept;

    private:

		struct CliConfig final
		{
			std::filesystem::path mainPath;
			CommandLineFileImporterConfig assetImport;
		};

		App& _app;
        std::unique_ptr<sol::state> _lua;
		std::filesystem::path _mainLuaPath;

		struct DbgText final
		{
			glm::uvec2 pos;
			std::string message;
		};

		std::vector<DbgText> _dbgTexts;

		std::optional<std::filesystem::path> findMainLua(const std::filesystem::path& path);
		bool importAssets(const CommandLineFileImporterConfig& cfg) noexcept;
		expected<int32_t, std::string> loadLua(const std::filesystem::path& mainPath) noexcept;
		void unloadLua() noexcept;

		void luaDebugScreenText(const glm::uvec2& pos, const std::string& msg) noexcept;
    };

	class LuaAppComponent final : public ITypeAppComponent<LuaAppComponent>
	{
	public:
		LuaAppComponent(const sol::table& table) noexcept;
		sol::object getReal() const noexcept;

		expected<void, std::string> init(App& app) noexcept override;
		expected<void, std::string> shutdown() noexcept override;
		expected < bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
		expected<void, std::string> update(float deltaTime) noexcept override;

	private:
		sol::main_table _table;

		static const LuaTableDelegateDefinition _initDef;
		static const LuaTableDelegateDefinition _shutdownDef;
		static const LuaTableDelegateDefinition _renderResetDef;
		static const LuaTableDelegateDefinition _updateDef;
	};
}