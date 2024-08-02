#pragma once

#include <darmok/export.h>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <optional>
#include <bgfx/bgfx.h>
#include <darmok/app_fwd.hpp>
#include <darmok/color_fwd.hpp>
#include <bx/bx.h>

#ifndef DARMOK_IMPLEMENT_MAIN
#	define DARMOK_IMPLEMENT_MAIN 0
#endif // DARMOK_IMPLEMENT_MAIN

#define DARMOK_CREATE_APP_DECL(app, ...)						\
	std::unique_ptr<darmok::App> darmokCreateApp()				\
	{															\
		return darmok::createApp<app>(##__VA_ARGS__);			\
	}															\

#if DARMOK_IMPLEMENT_MAIN
#define DARMOK_RUN_APP(app, ...)                                \
    DARMOK_CREATE_APP_DECL(app, ##__VA_ARGS__)                  \
                                                                \
	int main(int argc, const char* const* argv)                 \
	{                                                           \
		return darmok::main(argc, argv, darmokCreateApp());		\
	}															\

#else
#define DARMOK_RUN_APP(app, ...)                                \
	DARMOK_RUN_APP_DECL(app, ...)
#endif // DARMOK_IMPLEMENT_MAIN

namespace tf
{
	class Executor;
}

namespace darmok
{
	class IAppComponent;
	class App;
	class AppImpl;
	class Input;
	class Window;
	class AudioSystem;
	class AssetContext;
	class RenderGraphDefinition;

	DARMOK_EXPORT int32_t main(int32_t argc, const char* const* argv, std::unique_ptr<App>&& app);

	struct DARMOK_EXPORT AppConfig
	{
		static const AppConfig& getDefaultConfig() noexcept;

		float targetUpdateDeltaTime = getDefaultConfig().targetUpdateDeltaTime;
		int maxInstantLogicUpdates = getDefaultConfig().maxInstantLogicUpdates;
		Color clearColor = getDefaultConfig().clearColor;
	};

	class DARMOK_EXPORT App
	{
	public:
		App() noexcept;
		virtual ~App() noexcept;
		// return unix exit code for early exit
		virtual std::optional<int32_t> setup(const std::vector<std::string>& args);
		virtual void init();
		virtual void shutdown();
		AppUpdateResult update();

		enum class Phase
		{
			Setup,
			Init,
			Update,
			Shutdown
		};

		virtual void onException(Phase phase, const std::exception& ex) noexcept;

		[[nodiscard]] Input& getInput() noexcept;
		[[nodiscard]] const Input& getInput() const noexcept;

		[[nodiscard]] Window& getWindow() noexcept;
		[[nodiscard]] const Window& getWindow() const noexcept;

		[[nodiscard]] AssetContext& getAssets() noexcept;
		[[nodiscard]] const AssetContext& getAssets() const noexcept;

		[[nodiscard]] RenderGraphDefinition& getRenderGraph() noexcept;
		[[nodiscard]] const RenderGraphDefinition& getRenderGraph() const noexcept;

		[[nodiscard]] tf::Executor& getTaskExecutor() noexcept;
		[[nodiscard]] const tf::Executor& getTaskExecutor() const noexcept;

#ifdef DARMOK_MINIAUDIO
		[[nodiscard]] AudioSystem& getAudio() noexcept;
		[[nodiscard]] const AudioSystem& getAudio() const noexcept;
#endif

		void toggleDebugFlag(uint32_t flag) noexcept;
		void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;

		void addComponent(std::unique_ptr<IAppComponent>&& component) noexcept;
		bool removeComponent(const IAppComponent& component) noexcept;
		bool hasComponent(const IAppComponent& component) const noexcept;

		template<typename T, typename... A>
		T& addComponent(A&&... args)
		{
			auto ptr = std::make_unique<T>(std::forward<A>(args)...);
			auto& ref = *ptr;
			addComponent(std::move(ptr));
			return ref;
		}



	protected:
		void setConfig(const AppConfig& config) noexcept;

		virtual void updateLogic(float deltaTime);
		virtual void render() const;

	private:
		std::unique_ptr<AppImpl> _impl;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IAppComponent
	{
	public:
		virtual ~IAppComponent() = default;

		virtual void init(App& app) {};
		virtual void shutdown() {};
		virtual void renderReset() {};
		virtual void updateLogic(float deltaTime) {};
	};

	template<typename T, typename... A>
	std::unique_ptr<App> createApp(A&&... constructArgs)
	{
		return std::unique_ptr<App>(new T(std::forward<A>(constructArgs)...));
	};
} // namespace darmok