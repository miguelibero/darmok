#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <bgfx/bgfx.h>
#include <darmok/app_fwd.hpp>
#include <darmok/color_fwd.hpp>

#ifndef DARMOK_IMPLEMENT_MAIN
#	define DARMOK_IMPLEMENT_MAIN 0
#endif // DARMOK_IMPLEMENT_MAIN

#define DARMOK_RUN_APP_DECL(app, ...)                           \
	int32_t darmokRunApp(const std::vector<std::string>& args)  \
	{                                                           \
		return darmok::runApp<app>(args, ##__VA_ARGS__);        \
	}                                                           \

#if DARMOK_IMPLEMENT_MAIN
#define DARMOK_RUN_APP(app, ...)                                \
    DARMOK_RUN_APP_DECL(app, ##__VA_ARGS__)                     \
                                                                \
	int main(int argc, const char* const* argv)                 \
	{                                                           \
		return darmok::main(argc, argv, darmokRunApp);          \
	}                                                           \

#else
#define DARMOK_RUN_APP(app, ...)                                \
	DARMOK_RUN_APP_DECL(app, ...)
#endif // DARMOK_IMPLEMENT_MAIN

namespace darmok
{
	class AppComponent;
	class AppImpl;
	class Input;
	class Window;
	class AssetContext;

	DLLEXPORT int32_t main(int32_t argc, const char* const* argv, RunAppCallback callback);

	struct AppConfig
	{
		static const AppConfig defaultConfig;
		double targetUpdateDeltaTime = defaultConfig.targetUpdateDeltaTime;
		int maxInstantLogicUpdates = defaultConfig.maxInstantLogicUpdates;
		Color clearColor = defaultConfig.clearColor;
	};

	typedef std::shared_ptr<AppComponent> (*SharedAppComponentCreationCallback)();

	class App
	{
	public:
		DLLEXPORT App() noexcept;
		DLLEXPORT virtual ~App() noexcept;
		DLLEXPORT virtual void init(const std::vector<std::string>& args);
		DLLEXPORT virtual int shutdown();
		bool update();

		[[nodiscard]] DLLEXPORT Input& getInput() noexcept;
		[[nodiscard]] DLLEXPORT const Input& getInput() const noexcept;

		[[nodiscard]] DLLEXPORT Window& getWindow() noexcept;
		[[nodiscard]] DLLEXPORT const Window& getWindow() const noexcept;

		[[nodiscard]] DLLEXPORT AssetContext& getAssets() noexcept;
		[[nodiscard]] DLLEXPORT const AssetContext& getAssets() const noexcept;

		DLLEXPORT void toggleDebugFlag(uint32_t flag) noexcept;
		DLLEXPORT void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;

		DLLEXPORT void addComponent(std::unique_ptr<AppComponent>&& component) noexcept;
		DLLEXPORT bool removeComponent(AppComponent& component) noexcept;

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
		void configure(const AppConfig& config) noexcept;

		template<typename T>
		static std::shared_ptr<AppComponent> createComponent()
		{
			return std::make_shared<T>();
		}

		std::shared_ptr<AppComponent> getSharedComponent(size_t typeHash, SharedAppComponentCreationCallback callback);

		DLLEXPORT virtual void updateLogic(float deltaTime);
		DLLEXPORT [[nodiscard]] virtual bgfx::ViewId render(bgfx::ViewId viewId) const;

	private:
		std::unique_ptr<AppImpl> _impl;
	};

	class AppComponent
	{
	public:
		AppComponent() = default;
		virtual ~AppComponent() = default;

		virtual void init(App& app) {};
		virtual void shutdown() {};
		virtual void updateLogic(float deltaTime) {};
		virtual bgfx::ViewId render(bgfx::ViewId viewId) const { return viewId;  };
	};

	DLLEXPORT int runApp(std::unique_ptr<App>&& app, const std::vector<std::string>& args);

	template<typename T, typename... A>
	int runApp(const std::vector<std::string>& args, A&&... constructArgs)
	{
		auto app = std::unique_ptr<App>(new T(std::forward<A>(constructArgs)...));		
		return runApp(std::move(app), args);
	};
} // namespace darmok