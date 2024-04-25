#pragma once

#include <darmok/app.hpp>
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/optional_ref.hpp>
#include <string>
#include <cstdint>
#include <map>

namespace darmok
{
	class Platform;

	class AppImpl final
	{
	public:
		AppImpl() noexcept;
		void configure(const AppConfig& config) noexcept;
		void init(App& app, const std::vector<std::string>& args);
		void shutdown();

		void updateLogic(float deltaTime);
		bgfx::ViewId render(bgfx::ViewId viewId) const;
		void triggerExit() noexcept;

		bool processEvents();

		bool toggleDebugFlag(uint32_t flag) noexcept;
		void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;
		
		void addComponent(std::unique_ptr<AppComponent>&& component) noexcept;
		
		[[nodiscard]] Input& getInput() noexcept;
		[[nodiscard]] const Input& getInput() const noexcept;

		[[nodiscard]] Window& getWindow() noexcept;
		[[nodiscard]] const Window& getWindow() const noexcept;

		[[nodiscard]] AssetContext& getAssets() noexcept;
		[[nodiscard]] const AssetContext& getAssets() const noexcept;

		[[nodiscard]] Platform& getPlatform() noexcept;
		[[nodiscard]] const Platform& getPlatform() const noexcept;

		template <typename F>
		void update(const F& logicCallback)
		{
			if (getWindow().getPhase() != WindowPhase::Running)
			{
				return;
			}
			auto timePassed = updateTimePassed();
			const auto dt = _config.targetUpdateDeltaTime;
			auto i = 0;
			while (timePassed > dt && i < _config.maxInstantLogicUpdates)
			{
				logicCallback(dt);
				timePassed -= dt;
				i++;
			}
			logicCallback(timePassed);
		}

	private:
		void addBindings() noexcept;
		void removeBindings() noexcept;

		[[nodiscard]] float updateTimePassed() noexcept;
		[[nodiscard]] bool getDebugFlag(uint32_t flag) const noexcept;
		
		static const std::string _bindingsName;
	
		bool _exit;
		uint32_t _debug;
		uint64_t _lastUpdate;
		AppConfig _config;
		Platform& _plat;
		OptionalRef<App> _app;
		Input _input;
		Window _window;
		AssetContext _assets;

		std::vector<std::unique_ptr<AppComponent>> _components;
	};
}
