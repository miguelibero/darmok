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
	class RenderChain;

	DARMOK_EXPORT int32_t main(int32_t argc, const char* const* argv, std::unique_ptr<App>&& app);

	struct DARMOK_EXPORT AppUpdateConfig
	{
		static const AppUpdateConfig& getDefaultConfig() noexcept;
		float deltaTime = getDefaultConfig().deltaTime;
		int maxInstant = getDefaultConfig().maxInstant;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IAppListener
	{
	public:
		virtual ~IAppListener() = default;
		virtual void onAppComponentAdded(entt::id_type type, IAppComponent& component) {};
		virtual void onAppComponentRemoved(entt::id_type type, IAppComponent& component) {};
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
		AppRunResult run();

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

	protected:
		virtual void update(float deltaTime);
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
		virtual void update(float deltaTime) {};
	};

	template<typename T, typename... A>
	std::unique_ptr<App> createApp(A&&... constructArgs)
	{
		return std::unique_ptr<App>(new T(std::forward<A>(constructArgs)...));
	};
} // namespace darmok