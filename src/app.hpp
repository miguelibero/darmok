#pragma once

#include <darmok/app.hpp>
#include <string>
#include <cstdint>
#include <map>

namespace darmok
{
	using ViewWindows = std::map<bgfx::ViewId, WindowHandle>;

	class AppImpl final
	{
	public:
		static const float defaultTargetUpdateDeltaTime;
		static const int maxInstantLogicUpdates;

		AppImpl() noexcept;
		void init(App& app, const std::vector<std::string>& args, double targetUpdateDeltaTime = defaultTargetUpdateDeltaTime);
		void shutdown();

		template <typename F>
		void update(const F& logicCallback)
		{
			auto timePassed = updateTimePassed();

			auto& win = WindowContext::get().getWindow();
			if (win.isSuspended())
			{
				_mainWindowSuspended = true;
				return;
			}

			const auto dt = _targetUpdateDeltaTime;
			auto i = 0;
			while (timePassed > dt && i < maxInstantLogicUpdates)
			{
				logicCallback(dt);
				timePassed -= dt;
				i++;
			}
			logicCallback(timePassed);
		}

		void updateLogic(float deltaTime);

		void beforeWindowRender(const WindowHandle& window, bgfx::ViewId firstViewId);
		void beforeRender(bgfx::ViewId viewId);
		void render(bgfx::ViewId viewId);
		void afterRender(bgfx::ViewId viewId);
		void afterWindowRender(const WindowHandle& window, bgfx::ViewId viewId);
		void triggerExit() noexcept;

		bool processEvents();

		[[nodiscard]] uint32_t getResetFlags() const noexcept;
		bool toggleDebugFlag(uint32_t flag) noexcept;
		void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;
		bool toggleResetFlag(uint32_t flag) noexcept;
		void setResetFlag(uint32_t flag, bool enabled) noexcept;
		
		void addComponent(std::unique_ptr<AppComponent>&& component) noexcept;
		void addViewComponent(bgfx::ViewId viewId, std::unique_ptr<ViewComponent>&& component) noexcept;
		
		void setWindowView(bgfx::ViewId viewId, const WindowHandle& window = Window::DefaultHandle) noexcept;
		[[nodiscard]] WindowHandle getViewWindow(bgfx::ViewId viewId) const noexcept;
		[[nodiscard]] const ViewWindows& getViewWindows() const noexcept;
		[[nodiscard]] std::vector<bgfx::ViewId> getWindowViews(const WindowHandle& window = Window::DefaultHandle) const noexcept;
		[[nodiscard]] const std::string& getCurrentDir() const noexcept;

	private:
		float updateTimePassed() noexcept;

		void setCurrentDir(const std::string& dir) noexcept;
		void addBindings() noexcept;
		static void removeBindings() noexcept;

		[[nodiscard]] bool getResetFlag(uint32_t flag) const noexcept;
		[[nodiscard]] bool getDebugFlag(uint32_t flag) const noexcept;

		std::string _currentDir;
		static const std::string _bindingsName;
	
		bool _init;
		bool _exit;
		uint32_t _debug;
		uint32_t _reset;
		bool _mainWindowSuspended;
		bool _needsReset;
		uint64_t _lastUpdate;
		float _targetUpdateDeltaTime;

		ViewWindows _viewWindows;
		std::vector<std::unique_ptr<AppComponent>> _appComponents;
		std::map<bgfx::ViewId, std::vector<std::unique_ptr<ViewComponent>>> _viewComponents;
	};
}
