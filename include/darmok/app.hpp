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

#ifndef DARMOK_CONFIG_IMPLEMENT_MAIN
#	define DARMOK_CONFIG_IMPLEMENT_MAIN 0
#endif // ENTRY_CONFIG_IMPLEMENT_MAIN

#if DARMOK_CONFIG_IMPLEMENT_MAIN
#define DARMOK_IMPLEMENT_MAIN(app, ...)                      \
	int32_t _main_(int32_t argc, char** argv)                \
	{                                                        \
		return darmok::runApp<app>(argc, argv, ##__VA_ARGS__); \
	}
#else
#define DARMOK_IMPLEMENT_MAIN(app, ...) \
	static app s_app(__VA_ARGS__)
#endif // ENTRY_CONFIG_IMPLEMENT_MAIN

namespace darmok
{
	struct InputState;
	struct WindowHandle;
	class AppComponent;
	class ViewComponent;
	
	class BX_NO_VTABLE App
	{
	public:
		virtual void init(const std::vector<std::string>& args);
		virtual int  shutdown();
		bool update();

	protected:
		virtual void beforeUpdate(const InputState& input, bgfx::ViewId viewId, const WindowHandle& window);
		virtual void update(const InputState& input, bgfx::ViewId viewId, const WindowHandle& window);
		virtual void afterUpdate(const InputState& input, bgfx::ViewId viewId, const WindowHandle& window);

		void setViewWindow(bgfx::ViewId viewId, const WindowHandle& window);

		void toggleDebugFlag(uint32_t flag);
		void setDebugFlag(uint32_t flag, bool enabled = true);

		void toggleResetFlag(uint32_t flag);
		void setResetFlag(uint32_t flag, bool enabled = true);

		void addViewComponent(bgfx::ViewId viewId, std::unique_ptr<ViewComponent>&& component);

		template<typename T, typename... A>
		T& addViewComponent(bgfx::ViewId viewId, A&&... args)
		{
			auto ptr = new T(std::forward<A>(args)...);
			addViewComponent(viewId, std::unique_ptr<ViewComponent>(ptr));
			return *ptr;
		}

		void addComponent(std::unique_ptr<AppComponent>&& component);

		template<typename T, typename... A>
		T& addComponent(A&&... args)
		{
			auto ptr = new T(std::forward<A>(args)...);
			addComponent(std::unique_ptr<AppComponent>(ptr));
			return *ptr;
		}
	};

	class AppComponent
	{
	public:
		AppComponent() = default;
		virtual ~AppComponent() = default;

		virtual void init();
		virtual void shutdown();
		virtual void update(const InputState& input, bgfx::ViewId viewId, const WindowHandle& window);
	};

	class ViewComponent
	{
	public:
		ViewComponent() = default;
		virtual ~ViewComponent() = default;

		virtual void init(bgfx::ViewId viewId);
		virtual void shutdown();
		virtual void update(const InputState& input, const WindowHandle& window);
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