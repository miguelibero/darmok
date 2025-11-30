#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/app_fwd.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/optional_ref.hpp>

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <optional>
#include <filesystem>

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <entt/entt.hpp>

#ifndef DARMOK_IMPLEMENT_MAIN
#	define DARMOK_IMPLEMENT_MAIN 0
#endif // DARMOK_IMPLEMENT_MAIN

#define DARMOK_CREATE_APP_DELEGATE_FACTORY(appDlgClass, ...)						\
    class appDlgClass##Factory final : public darmok::IAppDelegateFactory			\
	{																				\
	public:																			\
        std::unique_ptr<darmok::IAppDelegate> operator()(darmok::App& app) override \
        {																			\
            return std::make_unique<appDlgClass>(app, ##__VA_ARGS__);				\
        }																			\
	};																				\


#if DARMOK_IMPLEMENT_MAIN
#define DARMOK_RUN_APP(appDlgClass, ...)								    \
    DARMOK_CREATE_APP_DELEGATE_FACTORY(appDlgClass, ##__VA_ARGS__)	        \
																		    \
	int main(int argc, const char* argv[])							    \
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
	class IAppDelegateFactory;
	class App;
	class AppImpl;
	class Input;
	class Window;
	class AudioSystem;
	class AssetContext;
	class RenderChain;

	DARMOK_EXPORT int32_t main(int32_t argc, const char* argv[], std::unique_ptr<IAppDelegateFactory>&& factory);

	struct DARMOK_EXPORT AppUpdateConfig final
	{
		static const AppUpdateConfig& getDefaultConfig() noexcept;
		float deltaTime = getDefaultConfig().deltaTime;
		int maxInstant = getDefaultConfig().maxInstant;
	};

	using CmdArgs = std::span<const char*>;

	class DARMOK_EXPORT BX_NO_VTABLE IAppDelegate
	{
	public:
		virtual ~IAppDelegate() = default;

		// return unix exit code for early exit
		virtual expected<int32_t, std::string> setup(const CmdArgs& args) noexcept { return 0; }
		virtual expected<void, std::string> init() noexcept { return {}; }
		virtual expected<void, std::string> earlyShutdown() noexcept { return {}; }
		virtual expected<void, std::string> shutdown() noexcept { return {}; }
		virtual expected<void, std::string> render() const noexcept { return {}; }
		virtual expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept { return viewId; }
		virtual expected<void, std::string> update(float deltaTime) noexcept { return {}; }
	};

	class DARMOK_EXPORT BX_NO_VTABLE IAppDelegateFactory
	{
	public:
		virtual ~IAppDelegateFactory() = default;
		virtual std::unique_ptr<IAppDelegate> operator()(App& app) = 0;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IAppUpdater
	{
	public:
		virtual ~IAppUpdater() = default;
		virtual entt::id_type getAppUpdaterType() const noexcept { return 0; };
		virtual expected<void, std::string> update(float deltaTime) noexcept = 0;
	};

	template<typename T>
	class DARMOK_EXPORT BX_NO_VTABLE ITypeAppUpdater : public IAppUpdater
	{
	public:
		entt::id_type getAppUpdaterType() const noexcept override
		{
			return entt::type_hash<T>::value();
		}
	};

	class DARMOK_EXPORT BX_NO_VTABLE IAppUpdaterFilter
	{
	public:
		virtual ~IAppUpdaterFilter() = default;
		virtual bool operator()(const IAppUpdater& updater) const = 0;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IAppComponent
	{
	public:
		virtual ~IAppComponent() = default;
		virtual entt::id_type getAppComponentType() const noexcept { return 0; };
		virtual expected<void, std::string> init(App& app) noexcept { return {}; }
		virtual expected<void, std::string> shutdown() noexcept { return {}; }
		virtual expected<void, std::string> render() noexcept { return {}; }
		virtual expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept { return viewId; }
		virtual expected<void, std::string> update(float deltaTime) noexcept { return {}; }
	};

	template<typename T>
	class DARMOK_EXPORT BX_NO_VTABLE ITypeAppComponent : public IAppComponent
	{
	public:
		entt::id_type getAppComponentType() const noexcept override
		{
			return entt::type_hash<T>::value();
		}
	};

	class DARMOK_EXPORT App final
	{
	public:
		App(std::unique_ptr<IAppDelegateFactory>&& delegateFactory) noexcept;
		~App() noexcept;
		App(const App&) = delete;
		App(App&&) = delete;
		App& operator=(const App&) = delete;
		App& operator=(App&&) = delete;

		expected<int32_t, std::string> setup(const CmdArgs& args) noexcept;
		expected<void, std::string> init() noexcept;
		void requestRenderReset() noexcept;
		expected<void, std::string> shutdown() noexcept;
		expected<AppRunResult, std::string> run() noexcept;

		void onUnexpected(AppPhase phase, const std::string& error) noexcept;
		void quit() noexcept;

		[[nodiscard]] Input& getInput() noexcept;
		[[nodiscard]] const Input& getInput() const noexcept;

		[[nodiscard]] Window& getWindow() noexcept;
		[[nodiscard]] const Window& getWindow() const noexcept;

		[[nodiscard]] AssetContext& getAssets() noexcept;
		[[nodiscard]] const AssetContext& getAssets() const noexcept;

		[[nodiscard]] tf::Executor& getTaskExecutor() noexcept;
		[[nodiscard]] const tf::Executor& getTaskExecutor() const noexcept;

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

		expected<void, std::string> setRendererType(bgfx::RendererType::Enum renderer) noexcept;

		void setClearColor(const Color& color) noexcept;
		void setUpdateConfig(const AppUpdateConfig& config) noexcept;

		void setPaused(bool paused) noexcept;
		bool isPaused() const noexcept;

		bool setAssetsBasePath(const std::filesystem::path& path) noexcept;
		bool addAssetsRootPath(const std::filesystem::path& path) noexcept;
		bool removeAssetsRootPath(const std::filesystem::path& path) noexcept;

		void addUpdater(std::unique_ptr<IAppUpdater>&& updater) noexcept;
		void addUpdater(IAppUpdater& updater) noexcept;
		bool removeUpdater(const IAppUpdater& updater) noexcept;
		size_t removeUpdaters(const IAppUpdaterFilter& filter) noexcept;

		template<typename T, typename... A>
		T& addUpdater(A&&... args)
		{
			auto ptr = std::make_unique<T>(std::forward<A>(args)...);
			auto& ref = *ptr;
			addUpdater(std::move(ptr));
			return ref;
		}

		expected<void, std::string> addComponent(std::unique_ptr<IAppComponent>&& component) noexcept;
		bool removeComponent(entt::id_type type) noexcept;
		[[nodiscard]] bool hasComponent(entt::id_type type) const noexcept;
		[[nodiscard]] OptionalRef<IAppComponent> getComponent(entt::id_type type) noexcept;
		[[nodiscard]] OptionalRef<const IAppComponent> getComponent(entt::id_type type) const noexcept;

		template<typename T>
		bool removeComponent()
		{
			return removeComponent(entt::type_hash<T>::value());
		}

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
		expected<std::reference_wrapper<T>, std::string> addComponent(A&&... args) noexcept
		{
			auto ptr = std::make_unique<T>(std::forward<A>(args)...);
			auto& ref = *ptr;
			auto result = addComponent(std::move(ptr));
			if(!result)
			{
				return unexpected{ std::move(result).error() };
			}
			return ref;
		}

		template<typename T, typename... A>
		OptionalRef<T> tryAddComponent(A&&... args) noexcept
		{
			auto result = addComponent<T>(std::forward<A>(args)...);
			return result ? &result.value().get() : nullptr;
		}

		template<typename T, typename... A>
		expected<std::reference_wrapper<T>, std::string> getOrAddComponent(A&&... args) noexcept
		{
			if (auto comp = getComponent<T>())
			{
				return comp.value();
			}
			return addComponent<T>(std::forward<A>(args)...);
		}

		template<typename T, typename... A>
		OptionalRef<T> tryGetOrAddComponent(A&&... args) noexcept
		{
			auto result = getOrAddComponent<T>(std::forward<A>(args)...);
			return result ? &result.value().get() : nullptr;
		}

	protected:
		expected<void, std::string> render() const noexcept;

	private:
		std::unique_ptr<AppImpl> _impl;
	};

} // namespace darmok