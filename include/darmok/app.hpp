/*
 * based on https://github.com/bkaradzic/bgfx/blob/master/examples/common/entry/entry.h
 */
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <bx/bx.h>
#include <darmok/window.hpp>

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
	struct InputState;
	struct WindowHandle;
	class AppComponent;
	class ViewComponent;
	class AppImpl;

	class BX_NO_VTABLE App
	{
	public:
		App() noexcept;
		~App() noexcept;
		virtual void init(const std::vector<std::string>& args);
		virtual int  shutdown();
		bool update();

		[[nodiscard]] const AppImpl& getImpl() const noexcept;
		[[nodiscard]] AppImpl& getImpl() noexcept;

	protected:
		virtual void updateLogic(float deltaTime);
		virtual void beforeWindowRender(const WindowHandle& window, bgfx::ViewId firstViewId);
		virtual void beforeRender(bgfx::ViewId viewId);
		virtual void render(bgfx::ViewId viewId);
		virtual void afterRender(bgfx::ViewId viewId);
		virtual void afterWindowRender(const WindowHandle& window, bgfx::ViewId lastViewId);

		void setWindowView(bgfx::ViewId viewId, const WindowHandle& window = Window::DefaultHandle) noexcept;
		[[nodiscard]] WindowHandle getViewWindow(bgfx::ViewId viewId) const noexcept;

		void toggleDebugFlag(uint32_t flag) noexcept;
		void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;

		void toggleResetFlag(uint32_t flag) noexcept;
		void setResetFlag(uint32_t flag, bool enabled = true) noexcept;

		void addViewComponent(bgfx::ViewId viewId, std::unique_ptr<ViewComponent>&& component) noexcept;

		template<typename T, typename... A>
		T& addViewComponent(bgfx::ViewId viewId, A&&... args) noexcept
		{
			auto ptr = new T(std::forward<A>(args)...);
			addViewComponent(viewId, std::unique_ptr<ViewComponent>(ptr));
			return *ptr;
		}

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

		virtual void init();
		virtual void shutdown();
		virtual void updateLogic(float deltaTime);
		virtual void beforeRender(bgfx::ViewId viewId);
		virtual void render(bgfx::ViewId viewId);
		virtual void afterRender(bgfx::ViewId viewId);
	};

	class ViewComponent
	{
	public:
		ViewComponent() = default;
		virtual ~ViewComponent() = default;

		virtual void init(bgfx::ViewId viewId);
		virtual void shutdown();
		virtual void updateLogic(float deltaTime);
		virtual void beforeRender();
		virtual void render();
		virtual void afterRender();
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