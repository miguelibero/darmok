#pragma once

#include <darmok/app.hpp>

namespace darmok
{
	class AppImpl final
	{
	public:
		AppImpl();

		bool processEvents();
		void setCurrentDir(const std::string& dir);
		uint32_t getResetFlags();

		void addBindings();
		void removeBindings();

		void toggleDebugFlag(uint32_t flag);
		void setDebugFlag(uint32_t flag, bool enabled = true);

	private:

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

		void toggleResetFlag(uint32_t flag);
		void setResetFlag(uint32_t flag, bool enabled);
		bool getResetFlag(uint32_t flag);

		bool getDebugFlag(uint32_t flag);

		std::string _currentDir;
		static const std::string _bindingsName;
	
		bool _exit;
		uint32_t _debug;
		uint32_t _reset;
		bool _needsReset;
	};
}
