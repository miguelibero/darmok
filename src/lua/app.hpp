#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/app.hpp>
#include <vector>
#include <optional>
#include <unordered_map>
#include <filesystem>
#include <sol/sol.hpp>
#include <bx/bx.h>
#include "glm.hpp"
#include "asset.hpp"
#include "window.hpp"
#include "input.hpp"
#include "audio.hpp"

namespace bx
{
	class CommandLine;
}

namespace darmok
{
	class AssetContext;
	class LuaAppComponentContainer;

	class LuaApp;

	class BX_NO_VTABLE ILuaYieldInstruction
	{
	public:
		virtual ~ILuaYieldInstruction() = default;
		virtual void update(float deltaTime) {};
		virtual bool finished() const = 0;
	};


	class LuaCombinedYieldInstruction final : public ILuaYieldInstruction
	{
	public:
		LuaCombinedYieldInstruction(const std::vector<std::shared_ptr<ILuaYieldInstruction>>& instructions) noexcept;
		void update(float deltaTime) override;
		bool finished() const noexcept override;
	private:
		std::vector<std::shared_ptr<ILuaYieldInstruction>> _instructions;
	};

	class LuaWaitForSeconds final : public ILuaYieldInstruction
	{
	public:
		LuaWaitForSeconds(float secs) noexcept;
		void update(float deltaTime) noexcept override;
		bool finished() const noexcept override;

		static void bind(sol::state_view& lua) noexcept;
	private:
		float _secs;
	};

	class LuaAppCoroutine final : public ILuaYieldInstruction
	{
	public:
		LuaAppCoroutine(const LuaApp& app, const sol::coroutine& coroutine) noexcept;
		bool finished() const noexcept override;
	private:
		std::reference_wrapper<const LuaApp> _app;
		sol::coroutine _coroutine;
	};

	class LuaApp final
	{
	public:
		LuaApp(App& app) noexcept;
		App& getReal() noexcept;
		const App& getReal() const noexcept;

		void update(float deltaTime) noexcept;

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

		bool isCoroutineRunning(const sol::coroutine& coroutine) const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::vector<sol::protected_function> _updaterFunctions;
		std::vector<sol::table> _updaterTables;
		std::vector<sol::coroutine> _coroutines;
		std::unordered_map<const void*, std::shared_ptr<ILuaYieldInstruction>> _coroutineAwaits;
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

		LuaAppCoroutine startCoroutine(const sol::coroutine& coroutine) noexcept;
		bool stopCoroutine(const sol::coroutine& coroutine) noexcept;

		void updateUpdaters(float deltaTime) noexcept;
		void updateCoroutines(float deltaTime) noexcept;
		static std::shared_ptr<ILuaYieldInstruction> readYieldInstruction(const sol::object& obj) noexcept;

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

	class LuaRunnerAppImpl final
    {
    public:
		LuaRunnerAppImpl(App& app) noexcept;
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