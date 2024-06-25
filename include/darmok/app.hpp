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

#ifndef DARMOK_IMPLEMENT_MAIN
#	define DARMOK_IMPLEMENT_MAIN 0
#endif // DARMOK_IMPLEMENT_MAIN

#define DARMOK_CREATE_APP_DECL(app, ...)						\
	std::unique_ptr<App> darmokCreateApp()						\
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

namespace darmok
{
	class AppComponent;
	class App;
	class AppImpl;
	class Input;
	class Window;
	class AssetContext;
	class Platform;

	DARMOK_EXPORT int32_t main(int32_t argc, const char* const* argv, std::unique_ptr<App>&& app);

	struct DARMOK_EXPORT AppConfig
	{
		static const AppConfig& getDefaultConfig() noexcept;

		float targetUpdateDeltaTime = getDefaultConfig().targetUpdateDeltaTime;
		int maxInstantLogicUpdates = getDefaultConfig().maxInstantLogicUpdates;
		Color clearColor = getDefaultConfig().clearColor;
	};

	typedef std::shared_ptr<AppComponent>(*SharedAppComponentCreationCallback)();

	class DARMOK_EXPORT App
	{
	public:
		App() noexcept;
		virtual ~App() noexcept;
		// return unix exit code for early exit
		virtual std::optional<int> setup(Platform& plat, const std::vector<std::string>& args);
		virtual void init();
		virtual void shutdown();
		bool update();

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

		void toggleDebugFlag(uint32_t flag) noexcept;
		void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;

		void addComponent(std::unique_ptr<AppComponent>&& component) noexcept;
		bool removeComponent(AppComponent& component) noexcept;

		template<typename T, typename... A>
		T& addComponent(A&&... args) noexcept
		{
			auto ptr = new T(std::forward<A>(args)...);
			addComponent(std::unique_ptr<AppComponent>(ptr));
			return *ptr;
		}

		template<typename T>
		std::shared_ptr<T> getSharedComponent()
		{
			auto comp = getSharedComponent(typeid(T).hash_code(), createComponent<T>);
			return std::static_pointer_cast<T>(comp);
		}

	protected:


		void setConfig(const AppConfig& config) noexcept;

		template<typename T>
		static std::shared_ptr<AppComponent> createComponent()
		{
			return std::make_shared<T>();
		}

		std::shared_ptr<AppComponent> getSharedComponent(size_t typeHash, SharedAppComponentCreationCallback callback);

		virtual void updateLogic(float deltaTime);
		virtual bgfx::ViewId render(bgfx::ViewId viewId) const;

	private:
		std::unique_ptr<AppImpl> _impl;
	};

	class DARMOK_EXPORT AppComponent
	{
	public:
		AppComponent() = default;
		virtual ~AppComponent() = default;

		virtual void init(App& app) {};
		virtual void shutdown() {};
		virtual void updateLogic(float deltaTime) {};
		virtual bgfx::ViewId render(bgfx::ViewId viewId) const { return viewId;  };
	};

	template<typename T, typename... A>
	std::unique_ptr<App> createApp(A&&... constructArgs)
	{
		return std::unique_ptr<App>(new T(std::forward<A>(constructArgs)...));
	};
} // namespace darmok