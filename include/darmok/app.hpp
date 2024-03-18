/*
 * based on https://github.com/bkaradzic/bgfx/blob/master/examples/common/entry/entry.h
 */
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <bgfx/bgfx.h>

extern "C" int _main_(int argc, char** argv);

#ifndef DARMOK_IMPLEMENT_MAIN
#	define DARMOK_IMPLEMENT_MAIN 0
#endif // DARMOK_IMPLEMENT_MAIN

#if DARMOK_IMPLEMENT_MAIN
#define DARMOK_MAIN(app, ...)                      \
	int32_t _main_(int32_t argc, char** argv)                \
	{                                                        \
		return darmok::runApp<app>(argc, argv, ##__VA_ARGS__); \
	}
#else
#define DARMOK_MAIN(app, ...) \
	static app s_app(__VA_ARGS__)
#endif // DARMOK_IMPLEMENT_MAIN

namespace darmok
{
	class AppComponent;
	class AppImpl;
	class Input;
	class Window;
	class AssetContext;

	struct AppConfig
	{
		static const AppConfig defaultConfig;
		double targetUpdateDeltaTime;
		int maxInstantLogicUpdates;
	};

	class App
	{
	public:
		App() noexcept;
		~App() noexcept;
		virtual void init(const std::vector<std::string>& args);
		virtual int shutdown();
		bool update();

		[[nodiscard]] Input& getInput() noexcept;
		[[nodiscard]] const Input& getInput() const noexcept;

		[[nodiscard]] Window& getWindow() noexcept;
		[[nodiscard]] const Window& getWindow() const noexcept;

		[[nodiscard]] AssetContext& getAssets() noexcept;
		[[nodiscard]] const AssetContext& getAssets() const noexcept;

	protected:
		void configure(const AppConfig& config) noexcept;

		virtual void updateLogic(float deltaTime);
		[[nodiscard]] virtual bgfx::ViewId render(bgfx::ViewId viewId);

		void toggleDebugFlag(uint32_t flag) noexcept;
		void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;

		void addComponent(std::unique_ptr<AppComponent>&& component) noexcept;

		template<typename T, typename... A>
		T& addComponent(A&&... args) noexcept
		{
			auto ptr = new T(std::forward<A>(args)...);
			addComponent(std::unique_ptr<AppComponent>(ptr));
			return *ptr;
		}
	private:
		std::unique_ptr<AppImpl> _impl;
	};

	class AppComponent
	{
	public:
		AppComponent() = default;
		virtual ~AppComponent() = default;

		virtual void init(App& app);
		virtual void shutdown();
		virtual void updateLogic(float deltaTime);
		virtual bgfx::ViewId render(bgfx::ViewId viewId);
	};

	int runApp(App& app, const std::vector<std::string>& args);

	template<typename T, typename... A>
	int runApp(int argc, const char* const* argv, A&&... constructArgs)
	{
		T app(std::forward<A>(constructArgs)...);
		std::vector<std::string> args(argc);
		for (int i = 0; i < argc; ++i)
		{
			args[i] = argv[i];
		}
		return runApp(app, args);
	};
} // namespace darmok