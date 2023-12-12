#pragma once

#include <darmok/app.hpp>
#include <string>
#include <cstdint>
#include <map>
#include <bx/timer.h>

namespace darmok
{
	typedef std::map<bgfx::ViewId, WindowHandle> ViewWindows;

	class AppImpl final
	{
	public:
		static const float defaultTargetUpdateDeltaTime;
		static const int maxInstantLogicUpdates;

		static AppImpl& get();

		void init(App& app, const std::vector<std::string>& args, double targetUpdateDeltaTime = defaultTargetUpdateDeltaTime);
		void shutdown();

		template <typename F>
		void update(const F& logicCallback)
		{

			int64_t now = bx::getHPCounter();
			float timePassed = (now - _lastUpdate) / 1000000.0;
 			_lastUpdate = bx::getHPCounter();

			auto& win = WindowContext::get().getWindow();
			if (win.isSuspended())
			{
				_mainWindowSuspended = true;
				return;
			}

			auto dt = _targetUpdateDeltaTime;
			auto i = 0;
			while (timePassed > dt && i < maxInstantLogicUpdates)
			{
				logicCallback(dt);
				timePassed -= dt;
				i++;
			}
			logicCallback(timePassed);
		}

		void updateLogic(float dt);

		void beforeRender(bgfx::ViewId viewId);
		void render(bgfx::ViewId viewId);
		void afterRender(bgfx::ViewId viewId);

		bool processEvents();

		uint32_t getResetFlags();
		bool toggleDebugFlag(uint32_t flag);
		void setDebugFlag(uint32_t flag, bool enabled = true);
		bool toggleResetFlag(uint32_t flag);
		void setResetFlag(uint32_t flag, bool enabled);
		
		void addComponent(std::unique_ptr<AppComponent>&& component);
		void addViewComponent(bgfx::ViewId viewId, std::unique_ptr<ViewComponent>&& component);
		
		void setViewWindow(bgfx::ViewId viewId, const WindowHandle& window);
		WindowHandle getViewWindow(bgfx::ViewId viewId) const;
		const ViewWindows& getViewWindows() const;

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
		bool _mainWindowSuspended;
		uint32_t _debug;
		uint32_t _reset;
		bool _needsReset;
		uint64_t _lastUpdate;
		float _targetUpdateDeltaTime;

		ViewWindows _viewWindows;
		std::vector<std::unique_ptr<AppComponent>> _appComponents;
		std::map<bgfx::ViewId, std::vector<std::unique_ptr<ViewComponent>>> _viewComponents;
	};
}
