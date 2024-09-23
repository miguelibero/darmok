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
#include <darmok/optional_ref.hpp>
#include <bx/bx.h>
#include <entt/entt.hpp>

#ifndef DARMOK_IMPLEMENT_MAIN
#	define DARMOK_IMPLEMENT_MAIN 0
#endif // DARMOK_IMPLEMENT_MAIN

#define DARMOK_CREATE_APP_DELEGATE_FACTORY(appDlgClass, ...)				\
    class appDlgClass##Factory final : public darmok::IAppDelegateFactory   \
	{                                                                       \
	public:                                                                 \
        std::unique_ptr<IAppDelegate> operator()(darmok::App& app) override \
        {                                                                   \
            return std::make_unique<appDlgClass>(app, ##__VA_ARGS__);       \
        }                                                                   \
	};                                                                      \

#if DARMOK_IMPLEMENT_MAIN
#define DARMOK_RUN_APP(appDlgClass, ...)								    \
    DARMOK_CREATE_APP_DELEGATE_FACTORY(appDlgClass, ##__VA_ARGS__)	        \
																		    \
	int main(int argc, const char* const* argv)							    \
	{																	    \
		return darmok::main(argc, argv,									    \
			std::make_unique<appDlgClass##Factory>());				        \
	}																	    \

#else
#define DARMOK_RUN_APP(app, ...)                                            \
	DARMOK_RUN_APP_DECL(app, ...)
#endif // DARMOK_IMPLEMENT_MAIN

namespace tf
{
	class Executor;
}

namespace darmok
{
	class IAppComponent;
	class IAppDelegateFactory;
	class App;
	class AppImpl;
	class Input;
	class Window;
	class AudioSystem;
	class AssetContext;
	class RenderGraphDefinition;
	class RenderChain;

	DARMOK_EXPORT int32_t main(int32_t argc, const char* const* argv, std::unique_ptr<IAppDelegateFactory>&& factory);

	struct DARMOK_EXPORT AppUpdateConfig
	{
		static const AppUpdateConfig& getDefaultConfig() noexcept;
		float deltaTime = getDefaultConfig().deltaTime;
		int maxInstant = getDefaultConfig().maxInstant;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IAppDelegate
	{
	public:
		virtual ~IAppDelegate() = default;

		// return unix exit code for early exit
		virtual std::optional<int32_t> setup(const std::vector<std::string>& args) { return std::nullopt; }
		virtual void init() {}
		virtual void earlyShutdown() {}
		virtual void shutdown() {}
		virtual void render() const {}
		virtual void renderReset() {}
		virtual void update(float deltaTime) {}
	};

	class DARMOK_EXPORT BX_NO_VTABLE IAppDelegateFactory
	{
	public:
		virtual ~IAppDelegateFactory() = default;
		virtual std::unique_ptr<IAppDelegate> operator()(App& app);
	};

	class DARMOK_EXPORT App final
	{
	public:
		App(std::unique_ptr<IAppDelegateFactory>&& delegateFactory) noexcept;
		~App() noexcept;
		std::optional<int32_t> setup(const std::vector<std::string>& args);
		void init();
		void shutdown();
		AppRunResult run();

		void onException(AppPhase phase, const std::exception& ex) noexcept;
		void quit() noexcept;

		[[nodiscard]] Input& getInput() noexcept;
		[[nodiscard]] const Input& getInput() const noexcept;

		[[nodiscard]] Window& getWindow() noexcept;
		[[nodiscard]] const Window& getWindow() const noexcept;

		[[nodiscard]] AssetContext& getAssets() noexcept;
		[[nodiscard]] const AssetContext& getAssets() const noexcept;

		[[nodiscard]] RenderGraphDefinition& getRenderGraph() noexcept;
		[[nodiscard]] const RenderGraphDefinition& getRenderGraph() const noexcept;

		[[nodiscard]] tf::Executor& getTaskExecutor();
		[[nodiscard]] const tf::Executor& getTaskExecutor() const;

#ifdef DARMOK_MINIAUDIO
		[[nodiscard]] AudioSystem& getAudio() noexcept;
		[[nodiscard]] const AudioSystem& getAudio() const noexcept;
#endif

		bool toggleDebugFlag(uint32_t flag) noexcept;
		void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;
		[[nodiscard]] bool getDebugFlag(uint32_t flag) const noexcept;

		bool toggleResetFlag(uint32_t flag) noexcept;
		void setResetFlag(uint32_t flag, bool enabled = true) noexcept;
		[[nodiscard]] bool getResetFlag(uint32_t flag) const noexcept;

		void setRendererType(bgfx::RendererType::Enum renderer);

		void setClearColor(const Color& color) noexcept;
		void setUpdateConfig(const AppUpdateConfig& config) noexcept;

		void addComponent(entt::id_type type, std::unique_ptr<IAppComponent>&& component) noexcept;
		bool removeComponent(entt::id_type type) noexcept;
		[[nodiscard]] bool hasComponent(entt::id_type type) const noexcept;
		[[nodiscard]] OptionalRef<IAppComponent> getComponent(entt::id_type type) noexcept;
		[[nodiscard]] OptionalRef<const IAppComponent> getComponent(entt::id_type type) const noexcept;

		template<typename T>
		[[nodiscard]] OptionalRef<T> getComponent() noexcept
		{
			auto ref = getComponent(entt::type_hash<T>::value());
			if (ref)
			{
				return (T*)ref.ptr();
			}
			return nullptr;
		}

		template<typename T>
		[[nodiscard]] OptionalRef<const T> getComponent() const noexcept
		{
			auto ref = getComponent(entt::type_hash<T>::value());
			if (ref)
			{
				return (const T*)ref.ptr();
			}
			return nullptr;
		}

		template<typename T, typename... A>
		T& addComponent(A&&... args)
		{
			auto ptr = std::make_unique<T>(std::forward<A>(args)...);
			auto& ref = *ptr;
			addComponent(entt::type_hash<T>::value(), std::move(ptr));
			return ref;
		}

		static void registerComponentDependency(entt::id_type typeId1, entt::id_type typeId2);

		template<typename T1, typename T2>
		static void registerComponentDependency()
		{
			registerComponentDependency(entt::type_hash<T1>::value(), entt::type_hash<T2>::value());
		}

	protected:
		void render() const;

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
		virtual void update(float deltaTime) {};
	};

} // namespace darmok