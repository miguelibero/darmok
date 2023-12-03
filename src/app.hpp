#pragma once

#include <darmok/app.hpp>
#include <string>
#include <cstdint>
#include <entt/entt.hpp>

namespace darmok
{
	class AppImpl final
	{
	public:
		static AppImpl& get();

		virtual void init(App& app, const std::vector<std::string>& args);
		virtual void shutdown();
		virtual void beforeUpdate(const WindowHandle& window, const InputState& input);
		virtual void afterUpdate(const WindowHandle& window, const InputState& input);
		bool processEvents();

		uint32_t getResetFlags();
		void toggleDebugFlag(uint32_t flag);
		void setDebugFlag(uint32_t flag, bool enabled = true);

		void addComponent(std::unique_ptr<AppComponent>&& component);

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

		void toggleResetFlag(uint32_t flag);
		void setResetFlag(uint32_t flag, bool enabled);
		bool getResetFlag(uint32_t flag);
		bool getDebugFlag(uint32_t flag);

		App& _app;
		std::vector<std::string> _args;
		std::string _currentDir;
		static const std::string _bindingsName;
	
		bool _exit;
		uint32_t _debug;
		uint32_t _reset;
		bool _needsReset;

		std::vector<std::unique_ptr<AppComponent>> _components;
		entt::registry _registry;
	};
}
