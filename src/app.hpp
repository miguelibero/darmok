#pragma once

#include <darmok/app.hpp>
#include <string>
#include <cstdint>
#include <map>

namespace darmok
{
	typedef std::map<bgfx::ViewId, WindowHandle> WindowViews;

	class AppImpl final
	{
	public:
		static AppImpl& get();

		virtual void init(App& app, const std::vector<std::string>& args);
		virtual void shutdown();
		virtual void update(const InputState& input, bgfx::ViewId viewId, const WindowHandle& window);
		bool processEvents();

		uint32_t getResetFlags();
		bool toggleDebugFlag(uint32_t flag);
		void setDebugFlag(uint32_t flag, bool enabled = true);
		bool toggleResetFlag(uint32_t flag);
		void setResetFlag(uint32_t flag, bool enabled);
		
		void addComponent(std::unique_ptr<AppComponent>&& component);
		void addViewComponent(bgfx::ViewId viewId, std::unique_ptr<ViewComponent>&& component);
		void setViewWindow(bgfx::ViewId viewId, const WindowHandle& window);
		const WindowViews& getWindowViews() const;

	private:

		AppImpl();

		void setCurrentDir(const std::string& dir);
		void addBindings();
		void removeBindings();

		static void exitAppBinding();
		static void fullscreenToggleBinding();
		static void toggleDebugStatsBinding();
		static void toggleDebugTextBinding();
		static void toggleDebugIfhBinding();
		static void toggleDebugWireFrameBinding();
		static void toggleDebugProfilerBinding();
		static void disableDebugFlagsBinding();
		static void toggleResetVsyncBinding();
		static void toggleResetMsaaBinding();
		static void toggleResetFlushAfterRenderBinding();
		static void toggleResetFlipAfterRenderBinding();
		static void toggleResetHidpiBinding();
		static void resetDepthClampBinding();
		static void screenshotBinding();


		bool getResetFlag(uint32_t flag);
		bool getDebugFlag(uint32_t flag);

		std::string _currentDir;
		static const std::string _bindingsName;
	
		bool _init;
		bool _exit;
		uint32_t _debug;
		uint32_t _reset;
		bool _needsReset;

		std::map<bgfx::ViewId, WindowHandle> _winViews;
		std::vector<std::unique_ptr<AppComponent>> _appComponents;
		std::map<bgfx::ViewId, std::vector<std::unique_ptr<ViewComponent>>> _viewComponents;
	};
}
